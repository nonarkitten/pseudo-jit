/// @file asm_offsets.c
/// @brief Never linked. Compiled to assembly only so the build can read the
///        constants below out of the compiler's output and write them to
///        asm_offsets.inc for the .s files (`.include "asm_offsets.inc"`).
///        That way an assembler routine can never disagree with cpu_t.
///
///        Every offset is relative to r5, i.e. negative (see CPU_OFF()).
#include "pjit.h"
#include "bcache.h"
#include "blk.h"

#define DEFINE(name, value) __asm__ volatile ("\n->" #name " %0" :: "i" (value))

void pjit_asm_offsets(void)
{
    DEFINE(CPU_SIZE,     sizeof(cpu_t));
    DEFINE(CPU_D0,       CPU_OFF(d0));
    DEFINE(CPU_A0,       CPU_OFF(a0));
    DEFINE(CPU_SR,       CPU_OFF(sr));
    DEFINE(CPU_USP,      CPU_OFF(usp));
    DEFINE(CPU_SSP,      CPU_OFF(ssp));
    DEFINE(CPU_R0,       CPU_OFF(r0));
    DEFINE(CPU_R2,       CPU_OFF(r2));
    DEFINE(CPU_R12,      CPU_OFF(r12));
    DEFINE(CPU_JMP_CPSR, CPU_OFF(jmp_cpsr));
    DEFINE(CPU_DIV_CPSR, CPU_OFF(div_cpsr));
    DEFINE(CPU_DIV_DST,  CPU_OFF(div_dst));
    DEFINE(CPU_SCRATCH,  CPU_OFF(scratch));
    DEFINE(CPU_TAGS_MISS, CPU_OFF(tags_miss));
    DEFINE(CPU_IPC,      CPU_OFF(ipc));
    DEFINE(CPU_HOST_SP,  CPU_OFF(host_sp));
    DEFINE(CPU_TRAP_EXIT_MASK, CPU_OFF(trap_exit_mask));
    DEFINE(CPU_CACHE_BE, CPU_OFF(cache_be));
    DEFINE(CPU_TAGS_BE,  CPU_OFF(tags_be));
    DEFINE(CPU_SHADOW,   CPU_OFF(shadow));
    DEFINE(CPU_IRQ_LEVEL, CPU_OFF(irq_level));
    DEFINE(CPU_IRQ_SITE, CPU_OFF(irq_site));
    DEFINE(CPU_IRQ_FLAG, CPU_OFF(irq_flag));
    DEFINE(CPU_IRQ_UNMASK, CPU_OFF(irq_unmask));
    DEFINE(CPU_FLUSH_REQ, CPU_OFF(flush_req));
    DEFINE(CPU_CODEBITS, CPU_OFF(codebits));
    DEFINE(CPU_CODEXACT, CPU_OFF(codeexact));
    DEFINE(CPU_XLAT, CPU_OFF(xlat));
    DEFINE(CPU_IRQ_WINDOWS, CPU_OFF(irq_windows));
    DEFINE(CPU_TAGS_GOTO, CPU_OFF(tags_goto));
    DEFINE(CPU_PREFETCH_HOOK, CPU_OFF(prefetch_hook));
    DEFINE(CPU_VBR,      CPU_OFF(vbr));
    DEFINE(CPU_LR,       CPU_OFF(lr));
    DEFINE(CPU_PC,       CPU_OFF(pc));
    DEFINE(CPU_TAGS,     CPU_OFF(tags));
    DEFINE(CPU_DR_THRESHOLD, CPU_OFF(dr_threshold));
    DEFINE(CPU_SPILL,    CPU_OFF(spill));
    DEFINE(CPU_CACHE_DATA, CPU_OFF(cache_data));
    DEFINE(CPU_DRSR,     CPU_OFF(drsr));
    DEFINE(DR_CACHE_BYTES, DR_CACHE_BYTES);
    DEFINE(DR_CL_SHIFT,  DR_CL_SHIFT);
    DEFINE(DR_CL_COUNT,  DR_CL_COUNT);
    DEFINE(DR_HL_SHIFT_, DR_HL_SHIFT);
    DEFINE(DR_SLOT_MASK_, DR_SLOT_MASK);
    DEFINE(DR_LINE_BITS, DR_LINE_BITS);
    DEFINE(DR_TAG_SHIFT, DR_TAG_SHIFT);
    DEFINE(DRSR_SHIFT_,  DRSR_SHIFT);
    DEFINE(DRSR_PAD_,    DRSR_PAD);
    DEFINE(ARENA_SIZE,   sizeof(pjit_arena_t));
    DEFINE(ARENA_OT,     __builtin_offsetof(pjit_arena_t, ot) - sizeof(cpu_t));
    DEFINE(ARENA_MOVEM,  __builtin_offsetof(pjit_arena_t, movem) - sizeof(cpu_t));
    DEFINE(ARENA_INL,    __builtin_offsetof(pjit_arena_t, inl) - sizeof(cpu_t));
    DEFINE(ARENA_LINKS,  __builtin_offsetof(pjit_arena_t, links) - sizeof(cpu_t));
    DEFINE(ARENA_OTX,    __builtin_offsetof(pjit_arena_t, ot_x) - sizeof(cpu_t));
    DEFINE(ARENA_CINL,   __builtin_offsetof(pjit_arena_t, cinl) - sizeof(cpu_t));
    DEFINE(ARENA_CINLX,  __builtin_offsetof(pjit_arena_t, cinl_x) - sizeof(cpu_t));
    DEFINE(ARENA_COMP,   __builtin_offsetof(pjit_arena_t, comp) - sizeof(cpu_t));
    DEFINE(ARENA_INLX,   __builtin_offsetof(pjit_arena_t, inl_x) - sizeof(cpu_t));
    DEFINE(ARENA_XCONS,  __builtin_offsetof(pjit_arena_t, xcons) - sizeof(cpu_t));
    DEFINE(ARENA_FUSE_NEXT, __builtin_offsetof(pjit_arena_t, fuse_next) - sizeof(cpu_t));
    DEFINE(FUSE_LIMIT,   FUSE_WORDS - 16);
    DEFINE(ARENA_FORM,   __builtin_offsetof(pjit_arena_t, form) - sizeof(cpu_t));
    DEFINE(ARENA_XEXACT, __builtin_offsetof(pjit_arena_t, xexact) - sizeof(cpu_t));
    DEFINE(LK_COUNT,     __builtin_offsetof(pjit_arena_t, links.count) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(LK_EVICT,     __builtin_offsetof(pjit_arena_t, links.evictions) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(LK_INVAL,     __builtin_offsetof(pjit_arena_t, links.invalidated) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(LK_MADE,      __builtin_offsetof(pjit_arena_t, links.made) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(LK_FLAG,      __builtin_offsetof(pjit_arena_t, links.flag) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(LK_LIST,      __builtin_offsetof(pjit_arena_t, links.list) - __builtin_offsetof(pjit_arena_t, links));
    DEFINE(ARENA_EXT,    __builtin_offsetof(pjit_arena_t, ext) - sizeof(cpu_t));   // from r5 (the DT base)
    DEFINE(BC_CURSOR,    __builtin_offsetof(bcache_t, cursor));
    DEFINE(BC_HW,        __builtin_offsetof(bcache_t, hw));
    DEFINE(BC_RING,      __builtin_offsetof(bcache_t, ring));
    DEFINE(BC_MASK,      __builtin_offsetof(bcache_t, mask));
    DEFINE(BC_TAGS,      __builtin_offsetof(bcache_t, tags));
    DEFINE(BC_SET_MASK,  __builtin_offsetof(bcache_t, set_mask));
    DEFINE(BC_SEG_SHIFT, __builtin_offsetof(bcache_t, seg_shift));
    DEFINE(BC_LINKS,     __builtin_offsetof(bcache_t, links));
    DEFINE(BC_EPOCHS,    __builtin_offsetof(bcache_t, epochs));
    DEFINE(BC_H_EXIT,    __builtin_offsetof(bcache_t, h_exit));
    DEFINE(BC_H_CALL,    __builtin_offsetof(bcache_t, h_call));
    DEFINE(BC_HOOK,      __builtin_offsetof(bcache_t, hook));
    DEFINE(BC_LIST_SHIFT, __builtin_offsetof(bcache_t, list_shift));
    DEFINE(BC_LMAX_F,    __builtin_offsetof(bcache_t, lmax));
    DEFINE(BC_SAVE,      __builtin_offsetof(bcache_t, save));
    DEFINE(BLK_CAP,      __builtin_offsetof(blk_t, cap));
    DEFINE(BLK_SRC_CAP,  __builtin_offsetof(blk_t, src_cap));
    DEFINE(BLK_STACK,    __builtin_offsetof(blk_t, stack_top));
    DEFINE(BLK_PARK,     __builtin_offsetof(blk_t, park));
    DEFINE(BLK_STUB_IN,  __builtin_offsetof(blk_t, stub_in));
    DEFINE(BLK_STUB_EP,  __builtin_offsetof(blk_t, stub_ep));
    DEFINE(BLK_TARGET,   __builtin_offsetof(blk_t, target));
    DEFINE(BLK_NSTUB,    __builtin_offsetof(blk_t, nstub));
    DEFINE(BLK_NSITE,    __builtin_offsetof(blk_t, nsite));
    DEFINE(BLK_STUB_TGT, __builtin_offsetof(blk_t, stub_tgt));
    DEFINE(BLK_STUB_KIND, __builtin_offsetof(blk_t, stub_kind));
    DEFINE(BLK_STUB_SITE, __builtin_offsetof(blk_t, stub_site));
    DEFINE(BLK_SITE_AT,  __builtin_offsetof(blk_t, site_at));
    DEFINE(BLK_SITE_STUB, __builtin_offsetof(blk_t, site_stub));
    DEFINE(BLK_MIRROR,   __builtin_offsetof(blk_t, mirror));
    DEFINE(BLK_FOLDMASK, __builtin_offsetof(blk_t, foldmask));
    DEFINE(BLK_FWDST,    __builtin_offsetof(blk_t, fwdst));
    DEFINE(BLK_ITEM_END, __builtin_offsetof(blk_t, item_end));
    DEFINE(BLK_ENDKIND,  __builtin_offsetof(blk_t, endkind));
    DEFINE(BLK_ITEMS,    __builtin_offsetof(blk_t, items));
    DEFINE(BLK_PROF_LAST, __builtin_offsetof(blk_t, prof_last));
    DEFINE(BLK_PROF,     __builtin_offsetof(blk_t, prof));
    DEFINE(BLK_BK_N,     __builtin_offsetof(blk_t, bk_n));
    DEFINE(BLK_BK,       __builtin_offsetof(blk_t, bk));
    DEFINE(BLK_MAXBK,    BLK_MAXBK);
    DEFINE(BLK_SCHSTART, __builtin_offsetof(blk_t, schedstart));
    DEFINE(BLK_N_SCHT,   __builtin_offsetof(blk_t, n_sched_try));
    DEFINE(BLK_N_SCHO,   __builtin_offsetof(blk_t, n_sched_ok));
    DEFINE(BLK_SCHED,    __builtin_offsetof(blk_t, sched));
    DEFINE(BLK_MAXITEM,  BLK_MAXITEM);
    DEFINE(BLK_N_TRANS,  __builtin_offsetof(blk_t, n_translated));
    DEFINE(BLK_N_INSTR,  __builtin_offsetof(blk_t, n_instr));
    DEFINE(BLK_N_EXITS,  __builtin_offsetof(blk_t, n_exits));
    DEFINE(BLK_N_FMOV,   __builtin_offsetof(blk_t, n_fwd_mov));
    DEFINE(BLK_N_FEXT,   __builtin_offsetof(blk_t, n_fwd_ext));
    DEFINE(BLK_N_FDEL,   __builtin_offsetof(blk_t, n_fwd_del));
    DEFINE(BLK_N_SLD,    __builtin_offsetof(blk_t, n_slot_ld));
    DEFINE(BLK_N_SST,    __builtin_offsetof(blk_t, n_slot_st));
    DEFINE(BLK_N_ROUTE,  __builtin_offsetof(blk_t, n_route));
    DEFINE(BLK_N_ONE,    __builtin_offsetof(blk_t, n_one));
    DEFINE(BLK_N_FUSED, __builtin_offsetof(blk_t, n_fused));
    DEFINE(BLK_TIER_BUILDS, __builtin_offsetof(blk_t, tier_builds));
    DEFINE(BLK_TIER_REBUILDS, __builtin_offsetof(blk_t, tier_rebuilds));
    DEFINE(BLK_TIER_PAUSE, __builtin_offsetof(blk_t, tier_pause));
    DEFINE(BLK_MAXSTUB,  BLK_MAXSTUB);
    DEFINE(BLK_FAST_BITS, BLK_FAST_BITS);
    DEFINE(BLK_MAXSITE,  BLK_MAXSITE);
}
