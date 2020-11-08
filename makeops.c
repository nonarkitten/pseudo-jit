#include "makeops.h"

#define EA_BUFFER_SIZE 64
#define append(X,...) X += sprintf(X, __VA_ARGS__)

typedef void(*genopfn_t)(char*,int);

const char *fn_proto = "void opcode_%04X(void) { ";
const char *data_sizes[4] = { "uint8_t", "uint16_t", NULL, "uint32_t" };
const char *ext_vars[2] = { "ext_s", "ext_d" };

static int pc_correct = 0;
static jmp_buf error;

typedef enum { NEED_NOTHING, NEED_PC, NEED_EXT } needs_t;

#define emit_read_ea(A,B,C,D) emit_ea((A),((B)&7),C,((D)&7), false, false)
#define emit_write_ea(A,B,C,D) emit_ea((A),((B)&7),C,((D)&7), true, false)
#define emit_read_modify_ea(A,B,C,D) emit_ea((A),((B)&7),C,((D)&7), false, true)
#define emit_write_modify_ea(A,B,C,D) emit_ea((A),((B)&7),C,((D)&7), true, true)

// any addressing modes that modify the program counter cannot be inlined
#define ea_inline(EA) (((EA)& 7)<5)

#define emit_int(B, I) sprintf((B), "%d", (I))
#define emit_hex(B, X) sprintf((B), "0x%08x", I)
#define emit_reg(B) ("BW_L"[(((B) / 8) - 1) & 3])
#define emit_var(B) (data_sizes[((bits / 8) - 1) & 3])

// return true if this instruction modifies the program counter
needs_t emit_ea(char *buffer, int ea, int bits, int reg, bool is_dest, bool is_rmw) {
    static const char* base_ea[] = {
        "D(%d,%c)", "A(%d,%c)", "(*APTR(%d,%c))",
        "(*APTR(%d,%c)++)", "(*--APTR(%d,%c))",
        "(APTR(%d,%c)[*PC_WPTR++])",
        "(APTR(%d,%c)[%s])"
    };

    if(ea < 7) {
        const char regtype = emit_reg(bits);
        if(is_rmw) {
            // change read (Ax)++ to just (Ax); increment done in write
            if(!is_dest && (ea == 3)) ea = 2;
            // change write --(Ax) to just (Ax); decrement done in read
            if( is_dest && (ea == 4)) ea = 2;
        }
        sprintf(buffer, base_ea[ea], reg, regtype, ext_vars[is_dest]);
        return (ea < 5) ? NEED_NOTHING : (ea == 5) ? NEED_PC : NEED_EXT;

    } else {
        const char* x = emit_var(bits);
        if(reg ==  0) { // (abs.w)
            sprintf(buffer, "(*(uint32_t*)(*PC_WPTR++))");
            return NEED_PC;
        } else if(reg == 1) { // (abs.l)
            sprintf(buffer, "(*(uint32_t*)(*PC_LPTR++))");
            return NEED_PC;
        } else if(reg == 2) { // (d16, PC)
            if(is_dest) longjmp(error, Illegal_Instruction);
            sprintf(buffer, "(*(%s*)&PC_BPTR[*PC_WPTR++])", x);
            return NEED_PC;
        } else if(reg == 3) { // (d8, PC, Xn)
            if(is_dest) longjmp(error, Illegal_Instruction);
            sprintf(buffer, "(*(%s*)&PC_BPTR[%s])", x, ext_vars[is_dest]);
            return NEED_EXT;
        } else if(reg == 4) { // immediate
            if(is_dest) longjmp(error, Illegal_Instruction);
            if(bits == 8) sprintf(buffer, "(uint8_t)(*PC_WPTR++)");
            if(bits == 16) sprintf(buffer, "(*PC_WPTR++)");
            if(bits == 32) sprintf(buffer, "(*PC_LPTR++)");
            return NEED_PC;
        } // end if(reg)
    } // end if(ea)

    longjmp(error, Address_Error);
}

void generate_0_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_move(char* buffer, int op, int bits) {
    const char* x = emit_var(bits);
    char s[EA_BUFFER_SIZE], d[EA_BUFFER_SIZE];
    int need_s = emit_read_ea(s, op >> 3, bits, op);
    bool movea = (op & 0x01C0) == 0x0040;

    append(buffer, "\n");

    if(movea) {
        if(bits == 8) longjmp(error, Illegal_Instruction);
        else emit_write_ea(d, op >> 6, 32, op >> 9);

        if(need_s) append(buffer, "\tPC = m68_syncpc();\n");
        if(need_s & NEED_EXT) append(buffer, "\tint32_t %s = m68_addrext(*PC_WPTR++);\n", ext_vars[0]);

        if(bits == 32) append(buffer, "\t%s = %s;\n", d, s);
        else append(buffer, "\t%s = (int32_t)(int16_t)%s;\n", d, s);

    } else {
        int need_d = emit_write_ea(d, op >> 6, bits, op >> 9);

        if(need_s | need_d) append(buffer, "\tPC = m68_syncpc();\n");
        if(need_s & NEED_EXT) append(buffer, "\tint32_t %s = m68_addrext(*PC_WPTR++);\n", ext_vars[0]);
        if(need_d & NEED_EXT) append(buffer, "\tint32_t %s = m68_addrext(*PC_WPTR++);\n", ext_vars[1]);
        append(buffer, "\t%s temp_imm = %s;\n", x, s);
        append(buffer, "\t%s = temp_imm;\n", d);
        append(buffer, "\tSET_NZ_%c(temp_imm);\n", emit_reg(bits));    

    }
}

void generate_1_ops(char* buffer, int op) { generate_move(buffer, op, 8); }
void generate_2_ops(char* buffer, int op) { generate_move(buffer, op, 32); }
void generate_3_ops(char* buffer, int op) { generate_move(buffer, op, 16); }

void generate_4_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

const char* condition[16] = {
    "true",
    "false",
    "(FLAG_C == 0 && FLAG_Z == 0)", // hi
    "(FLAG_C == 1 || FLAG_Z == 1)", // ls
    "FLAG_C == 0", // cc
    "FLAG_C == 1", // cs
    "FLAG_Z == 0", // ne
    "FLAG_Z == 1", // eq
    "FLAG_V == 0", // vc
    "FLAG_V == 1", // vs
    "FLAG_N == 0", // pl
    "FLAG_N == 1", // mi
    "(FLAG_N ^ FLAG_V) == 0", // ge
    "(FLAG_N ^ FLAG_V) == 1", // lt
    "(FLAG_Z == 0) && ((FLAG_N ^ FLAG_V) == 0)", // gt
    "(FLAG_Z == 1) || ((FLAG_N ^ FLAG_V) == 1)", // le
};

// Mutiple
void generate_5_ops(char* buffer, int op) {
    char s[EA_BUFFER_SIZE], d[EA_BUFFER_SIZE];

    if((op & 0x0C00) == 0xC00) {
        append(buffer, "\n");

        uint8_t cond = (op >> 8) & 15;
        if((op & 0x0F80) == 0x0C80) {
            // DBcc
            append(buffer, "\tPC = m68_syncpc();\n");
            append(buffer, "\tconst int32_t off = *(int16_t*)PC_WPTR++;\n");
            append(buffer, "\tif(!(%s)) {\n", condition[cond]);
                emit_read_ea(s, 0, 16, op); // get register
                append(buffer, "\t\t%s -= 1;\n", s);
                append(buffer, "\t\tif(%s == 0xFFFF) m68_enter(PC + off);\n", s);
            append(buffer, "\t}\n");
            return;

        } else {
            // Scc
            int need = emit_write_modify_ea(d, op >> 3, 8, op);

            append(buffer, "\tSR = m68_get_ccr();\n");

            if(need) append(buffer, "\tPC = m68_syncpc();\n");
            if(need & NEED_EXT) append(buffer, "\tint32_t %s = m68_addrext(*PC_WPTR++);\n", ext_vars[1]);

            append(buffer, "\t%s = (%s) ? 0xFF : 0x00;\n", d, condition[cond]);
            return;

        }
    } else {
        // ADDQ/SUBQ
        uint32_t bits = (8 << ((op >> 6) & 3));
        const char* t = emit_var(bits);
        emit_read_modify_ea(s, op >> 3, bits, op);
        if(!emit_write_modify_ea(d, op >> 3, bits, op)) {
            uint8_t imm = 1 + ((op >> 9) & 7);
            if(op & 0x0100) append(buffer, "%s = add_%s(%s, %d); ", d, t, s, imm);
            else            append(buffer, "%s = sub_%s(%s, %d); ", d, t, s, imm);
            return;
        }
    }
    longjmp(error, Illegal_Instruction);
}

// Bcc -- no flgs set
void generate_6_ops(char* buffer, int op) {
    int32_t offset = (int8_t)op;
    uint8_t cond = (op >> 8) & 15;
    char buffer2[EA_BUFFER_SIZE];

    if(offset == -1) emit_read_ea(buffer2, 7, 32, 4);
    else if(offset == 0) emit_read_ea(buffer2, 7, 16, 4);
    else if(offset & 1) longjmp(error, Bus_Error); 
    else emit_int(buffer2, offset);

    append(buffer, "\n\tPC = m68_syncpc();\n");
    append(buffer, "\tconst int32_t off = %s;\n", buffer2);     

    if(cond == 0) {
        // unconditional branch
        append(buffer, "\tm68_enter(PC + off);\n");

    } else if(cond == 1) {
        // branch subroutine, push to stack
        emit_read_ea(buffer2, 4, 32, 7);
        append(buffer, "\t%s = PC;\n", buffer2); 
        append(buffer, "\tm68_enter(PC + off);\n");

    } else {
        // conditional branch
        append(buffer, "\tSR = m68_get_ccr();\n");
        append(buffer, "\tif(%s) m68_enter(PC + off);\n", condition[cond]);
    }
}

// MOVEQ
void generate_7_ops(char* buffer, int op) {
    char s[EA_BUFFER_SIZE];
    int8_t value = op;
    emit_read_ea(s, 0, 32, op >> 9);
    append(buffer, "%s = %d; ", s, value);
}

void generate_8_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_9_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_A_ops(char* buffer, int op) {
    longjmp(error, Unimplemented_ALine);
}

void generate_B_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_C_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_D_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_E_ops(char* buffer, int op) {
    longjmp(error, Illegal_Instruction);
}

void generate_F_ops(char* buffer, int op) {
    longjmp(error, Unimplemented_FLine);
}

int main(void) {
    char function_buffer[2048];
    char name_buffer[256];
    static const genopfn_t generator[16] = {
        generate_0_ops, generate_1_ops, generate_2_ops, generate_3_ops,
        generate_4_ops, generate_5_ops, generate_6_ops, generate_7_ops,
        generate_8_ops, generate_9_ops, generate_A_ops, generate_B_ops,
        generate_C_ops, generate_D_ops, generate_E_ops, generate_F_ops,
    };
    static char* opcode_table[65536] = { 0 };
    static char filename[256];
    static FILE* file;
    uint16_t errcode;

    for(int group=0; group<16; group++) {
        sprintf(filename, "pjit_%x_ops.c", group);
        if((file = fopen(filename, "w"))) {
            fprintf(file, "// This file has been autogenerated, do not edit\n");
            fprintf(file, "// This file is part of PJIT, see pjit.h for copyright\n");
            fprintf(file, "\n#include \"pjit.h\"\n\n");
            for(int ops=0; ops<4096; ops++) {
                pc_correct = 0;
                //fprintf(file, fn_proto, (group << 12) | ops);
                if((errcode = setjmp(error)) == 0) {
                    generator[group](function_buffer, ops);
                    sprintf(name_buffer, "opcode_%04X", (group << 12) | ops);
                    fprintf(file, "void %s(void) { %s}\n", name_buffer, function_buffer);
                } else {
                    sprintf(name_buffer, "vector_%04X", errcode);
                }
                opcode_table[(group << 12) | ops] = strdup(name_buffer);
            }
            fclose(file);
        }
    }
    uint32_t used = 0;
    if((file = fopen("pjit_table.c", "w"))) {
        fprintf(file, "// This file has been autogenerated, do not edit\n");
        fprintf(file, "// This file is part of PJIT, see pjit.h for copyright\n");
        fprintf(file, "\n#include \"pjit.h\"\n\n");
        fprintf(file, "\nm68_opcodes[65536] = {\n");
        for(int i=0; i<65536; i++) {
            if(opcode_table[i][0] == 'o') used++;
            if((i % 6) == 0) fprintf(file, "\t");
            fprintf(file, "%s, ", opcode_table[i]);
            if((i % 6) == 5) fprintf(file, "\n");
        }
        fprintf(file, "}\n");
        fclose(file);
    }
    if((file = fopen("pjit_table.h", "w"))) {
        fprintf(file, "// This file has been autogenerated, do not edit\n");
        fprintf(file, "// This file is part of PJIT, see pjit.h for copyright\n");
        fprintf(file, "\n#include \"pjit.h\"\n\n");
        fprintf(file, "\n// %d defined opcodes\n\n", used);
        for(int i=0; i<65536; i++) {
            if(opcode_table[i][0] == 'v') continue;
            fprintf(file, "extern void %s(void);\n", opcode_table[i]);
        }
        fprintf(file, "}\n");
        fclose(file);
    }
    return 0;
}