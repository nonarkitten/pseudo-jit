#include "makeops.h"

#define append(X,...) X += sprintf(X, __VA_ARGS__)

typedef void(*genopfn_t)(char*,int);

const char *fn_proto = "void opcode_%04X(void) {";
//

static int pc_correct = 0;
static jmp_buf error;

enum {
    NEED_PC = 1,
    NEED_EXT = 2,
};

char* emit_ea(int ea, int bits, int reg, int *need) {
    static char buffer[64];
    const char regtype = (bits == 32) ? 'l' : (bits == 16) ? 'w' : 'b';
    char*x = 
        (bits == 32) ? "uint32_t" :
        (bits == 16) ? "uint16_t" : "uint8_t";

    reg &= 7; ea &= 7;

    switch(ea) {
        case 0: // Dx
            sprintf(buffer, "(d%d_%c[0])", reg, regtype);
            return strdup(buffer);
        case 1: // Ax
            if(bits == 8) longjmp(error, Address_Error); 
            sprintf(buffer, "(a%d_%c)", reg, regtype); 
            return strdup(buffer);
        case 2: // (Ax)
            sprintf(buffer, "(*a%d_%cp)", reg, regtype);
            return strdup(buffer);
        case 3: // (Ax)+
            sprintf(buffer, "(*a%d_%cp++)", reg, regtype);
            return strdup(buffer);
        case 4: // -(Ax)
            sprintf(buffer, "(*--a%d_%cp)", reg, regtype);
            return strdup(buffer);
        case 5: // (d16, Ax)
            sprintf(buffer, "(*(%s*)&a%d_bp[*pc_wp++])", x, reg);
            if(need) *need |= NEED_PC;
            return strdup(buffer);
        case 6: // (d8, An, Xn)
            sprintf(buffer, "(*(%s*)&a%d_bp[ext])", x, reg);
            if(need) *need |= NEED_EXT;
            return strdup(buffer);
        case 7: {
            switch(reg) {
                case 0: // (abs.w)
                    sprintf(buffer, "(*(uint32_t*)(*pc_wp++))");
                    if(need) *need |= NEED_PC;
                    return strdup(buffer);
                case 1: // (abs.l)
                    sprintf(buffer, "(*(uint32_t*)(*pc_lp++))");
                    if(need) *need |= NEED_PC;
                    return strdup(buffer);
                case 2: // (d16, PC)
                    sprintf(buffer, "(*(%s*)&pc_bp[*pc_wp++])", x);
                    if(need) *need |= NEED_PC;
                    return strdup(buffer);
                case 3: // (d8, PC, Xn)
                    sprintf(buffer, "(*(%s*)&pc_bp[ext])", x);
                    if(need) *need |= NEED_EXT;
                    return strdup(buffer);
                case 4: // immediate
                    sprintf(buffer, "(*pc_lp++)");
                    if(need) *need |= NEED_PC;
                    return strdup(buffer);
            } // end switch
        } // end case 7
    } // end switch

    longjmp(error, Address_Error);
}

void generate_0_ops(char* buffer, int op) {
}

void generate_move(char* buffer, int op, int bits) {
    int need_s = 0, need_d = 0;
    char*s = emit_ea(op >> 3, bits, op, &need_s);
    char*d = emit_ea(op >> 6, bits, op >> 9, &need_s);
    char*x = 
        (bits == 32) ? "uint32_t" :
        (bits == 16) ? "uint16_t" : "uint8_t";

    if((need_s | need_d)) {
        // long form
        append(buffer, "\n");
        // find original PC
        append(buffer, "\tm68_syncpc();\n");
        if(need_d & NEED_EXT) {
            append(buffer, "\tint32_t ext;\n");
            if(need_s & NEED_EXT) {
                append(buffer, "\text = m68_addrext();\n");
            }
            append(buffer, "\t%s x = %s\n", x, s);
            append(buffer, "ext = m68_addrext();\n");
            append(buffer, "\t%s = x;\n", d);
        } else {
            if(need_s & NEED_EXT) {
                append(buffer, "\tint32_t ext = m68_addrext();\n");
            }
            append(buffer, "\t%s = %s;\n", d, s);
        }
        append(buffer, "\tSET_NZ_B(%s);\n", d);
    } else {
        // short form
        if((op & 0x01C0) == 0x0040) {
            // movea
            free(d); d = emit_ea(op >> 6, 32, op >> 9, &need_s);
            if(bits == 16) {
                append(buffer, " %s = (int32_t)(int16_t)%s; ", d, s);
            } else {
                append(buffer, " %s = %s; ", d, s);
            }
        } else {
            append(buffer, " %s = %s; ", d, s);
            append(buffer, " SET_NZ_B(%s); ", d);
        }
    }
    free(s); free(d);
}

void generate_1_ops(char* buffer, int op) { generate_move(buffer, op, 8); }
void generate_2_ops(char* buffer, int op) { generate_move(buffer, op, 32); }
void generate_3_ops(char* buffer, int op) { generate_move(buffer, op, 16); }

void generate_4_ops(char* buffer, int op) {
    //return OPCODE_ILLEGAL;
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
    // if((op & 0x0C00) == 0xC00) {
    //     uint8_t cond = (op >> 8) & 15;
    //     if((op & 0x0F80) == 0x0C80) {
    //         // DBcc
    //         append(buffer, "\tconst int32_t off = m68_read_16(pc.u); pc.u += 2; ip.u += 4;\n"); 
    //         append(buffer, "\tif(!(%s)) {\n\t", condition[cond]);
    //         emit_read_ea(0, 16, op); // get register
    //         append(buffer, "\t\tx -= 1;\n\t");
    //         emit_write_ea(0, 16, op); // set register
    //         append(buffer, "\t\tif(x == 0xFFFF) m68_enter(pc.u + off);\n\t}\n");
    //         return OPCODE_OKAY;
    //     } else {
    //         // Scc
    //         //rmw = 1;
    //         if(!emit_read_ea(op >> 3, 8, op)) return OPCODE_BAD_ADDR_MODE;
    //         append(buffer, "\tif(%s) x |= 0xFF;\n", condition[cond]);
    //         append(buffer, "\telse x &= ~0xFF;\n");
    //         if(!emit_write_ea(op >> 3, 8, op)) return OPCODE_BAD_ADDR_MODE;
    //         return OPCODE_OKAY;
    //     }
    // } else {
    //     // ADDQ/SUBQ
    //     uint8_t imm = 1 + ((op >> 9) & 7);
    //     uint8_t bits = 8 << ((op >> 6) & 3);
    //     if(!emit_read_ea(op >> 3, bits, op)) return OPCODE_BAD_ADDR_MODE;
    //     if(op & 0x0100) append(buffer, "\tx -= %d;\n", imm);
    //     else append(buffer, "\tx += %d;\n", imm);
    //     if(!emit_write_ea(op >> 3, bits, op)) return OPCODE_BAD_ADDR_MODE;
    //     return OPCODE_OKAY;
    // }
    //return OPCODE_ILLEGAL;
}

// Bcc -- no flgs set
void generate_6_ops(char* buffer, int op) {
    // bra
    int32_t offset = (int8_t)op;
    uint8_t cond = (op >> 8) & 15;


    if(offset) {
        // 8-bit relative
        append(buffer, "\tconst int32_t off = %d;\n", offset); 
    } else {
        // 16-bit relative
        append(buffer, "\tconst int32_t off = *pc_wp++;\n"); 
    }
    
    if(cond == 0) {
        append(buffer, "\n\tm68_syncpc();\n");
        append(buffer, "\tm68_enter(pc_u + off);\n");

    } else if(cond == 1) {
        // branch subroutine, push to stack
        char* s = emit_ea(4, 32, 7, NULL);
        append(buffer, "\n\tm68_syncpc();\n");
        append(buffer, "\t%s = pc_u;\n", s); 
        append(buffer, "\tm68_enter(pc_u + off);\n");

    } else {
        //sr_ccr
        append(buffer, "\tm68_get_ccr();\n");
        append(buffer, "\tif(%s) { m68_syncpc(); m68_enter(pc.u + off); }\n", condition[cond]);
    }

    //return OPCODE_OKAY;
}

void generate_7_ops(char* buffer, int op) {
}

void generate_8_ops(char* buffer, int op) {
}

void generate_9_ops(char* buffer, int op) {
}

void generate_A_ops(char* buffer, int op) {
    append(buffer, "\tALINEEXP();\n}\n\n");
}

void generate_B_ops(char* buffer, int op) {
}

void generate_C_ops(char* buffer, int op) {
}

void generate_D_ops(char* buffer, int op) {
}

void generate_E_ops(char* buffer, int op) {
}

void generate_F_ops(char* buffer, int op) {
}


int main(void) {
    char function_buffer[2048];
    static const genopfn_t generator[16] = {
        generate_0_ops, generate_1_ops, generate_2_ops, generate_3_ops,
        generate_4_ops, generate_5_ops, generate_6_ops, generate_7_ops,
        generate_8_ops, generate_9_ops, generate_A_ops, generate_B_ops,
        generate_C_ops, generate_D_ops, generate_E_ops, generate_F_ops,
    };
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
                fprintf(file, fn_proto, (group << 12) | ops);
                if((errcode = setjmp(error)) == 0) {
                    generator[group](function_buffer, ops);
                    fprintf(file, "%s}\n", function_buffer);
                } else {
                    fprintf(file, " m68_exception(0x%04X); }\n", errcode);
                }
            }
            fclose(file);
        }

    }
    return 0;
}