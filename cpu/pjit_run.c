#if 0
// All memory above 24bit is cacheable; memory below this is split into
// 64KB chunks where we can define if it's cacheable
static struct pjit_page_ctrl_t {
    uint8_t cacheable : 1;
} pjit_ctrl[256] = { 0 };

register cpu_t cpu asm("r12");

/*
    PJIT can run in one of two basic operating modes; interpreter and JIT.
    In interpreter mode, we'll fetch and handle each instruction one-at-a-time.
    This is really slow, but gaurantees execution in DMA-able memory where it's
    possible to have code modified during execution. For example, on the Amiga,
    all chip RAM is DMA-able by the chipset and all 24-bit fast RAM is DMA-able
    from storage devices, so we have to stick with interpreted mode for these.
    However, the ROMs are always safe to cache. 
*/
void pjit_run(void) {
    
    cpu->a7 = *(uint32_t*)0;
    cpu->pc = *(uint32_t*)4;
    
}
#endif







