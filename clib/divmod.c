#include <stdint.h>

// 64-bit unsigned division
uint64_t __udivmoddi4(uint64_t num, uint64_t den, uint64_t* rem_p) {
    uint64_t quot = 0, qbit = 1;

    /*  Intentional divide by zero, without triggering a compiler
        warning which would abort the build */
    if (den == 0) return 1 / ((unsigned)den);

    /* Left-justify denominator and count shift */
    while ((int64_t)den >= 0) den <<= 1, qbit <<= 1;

    while (qbit) {
        if (den <= num) num -= den, quot += qbit;
        den >>= 1;
        qbit >>= 1;
    }

    if (rem_p) *rem_p = num;
    return quot;
}

// 64-bit signed division
int64_t __divmoddi4(int64_t num, int64_t den, int64_t* rem_p) {
    int minus = 0;
    int64_t v, r;

    if (num < 0) num = -num, minus = 1;
    if (den < 0) den = -den, minus ^= 1;

    v = __udivmoddi4(num, den, &r);
    if (minus) v = -v, r = -r;
    if (rem_p) *rem_p = r;
    return v;
}

// 32-bit unsigned division
uint32_t __udivmodsi4(uint32_t num, uint32_t den, uint32_t* rem_p) {
    uint32_t quot = 0, qbit = 1;

    /*  Intentional divide by zero, without triggering a compiler
        warning which would abort the build */
    if (den == 0) return 1 / ((unsigned)den);

    /* Left-justify denominator and count shift */
    while ((int32_t)den >= 0) den <<= 1, qbit <<= 1;

    while (qbit) {
        if (den <= num) num -= den, quot += qbit;
        den >>= 1;
        qbit >>= 1;
    }

    if (rem_p) *rem_p = num;
    return quot;
}

// 32-bit signed division
int32_t __divmodsi4(int32_t num, int32_t den, int32_t* rem_p) {
    int minus = 0;
    int32_t v, r;

    if (num < 0) num = -num, minus = 1;
    if (den < 0) den = -den, minus ^= 1;

    v = __udivmodsi4(num, den, &r);
    if (minus) v = -v, r = -r;
    if (rem_p) *rem_p = r;
    return v;
}

// AEABI wrappers for the above
__attribute__((naked)) void __aeabi_uldivmod(void) {
    __asm __volatile(
        "push	{r11, lr}      \n\t"
        "sub	sp, sp, #16    \n\t"
        "add	r12, sp, #8    \n\t"
        "str	r12, [sp]      \n\t"
        "bl     __udivmoddi4   \n\t"
        "ldr	r2, [sp, #8]   \n\t"
        "ldr	r3, [sp, #12]  \n\t"
        "add	sp, sp, #16    \n\t"
        "pop	{r11, pc}      \n\t");
}

__attribute__((naked)) void __aeabi_ldivmod(void) {
    __asm __volatile(
        "push    {r11, lr}     \n\t"
        "sub     sp, sp, #16   \n\t"
        "add     r12, sp, #8   \n\t"
        "str     r12, [sp]     \n\t"
        "bl      __divmoddi4   \n\t"
        "ldr     r2, [sp, #8]  \n\t"
        "ldr     r3, [sp, #12] \n\t"
        "add     sp, sp, #16   \n\t"
        "pop     {r11, pc}     \n\t");
}

__attribute__((naked)) void __aeabi_uidivmod(void) {
    __asm __volatile(
        "push    { lr }        \n\t"
        "sub     sp, sp, #4    \n\t"
        "mov     r2, sp        \n\t"
        "bl      __udivmodsi4  \n\t"
        "ldr     r1, [sp]      \n\t"
        "add     sp, sp, #4    \n\t"
        "pop     { pc }        \n\t");
}

__attribute__((naked)) void __aeabi_idivmod(void) {
    __asm __volatile(
        "push    { lr }        \n\t"
        "sub     sp, sp, #4    \n\t"
        "mov     r2, sp        \n\t"
        "bl      __divmodsi4   \n\t"
        "ldr     r1, [sp]      \n\t"
        "add     sp, sp, #4    \n\t"
        "pop     { pc }        \n\t");
}

uint32_t __aeabi_uidiv(uint32_t num, uint32_t den) {
	return __udivmodsi4(num, den, (uint32_t*)0);
}

int32_t __aeabi_idiv(int32_t num, int32_t den) {
	return __divmodsi4(num, den, (int32_t*)0);
}
