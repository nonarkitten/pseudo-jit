/* Kernels for tools/data/kcmp.py / tests/qemu/smoke_kcmp.c: the same C is compiled for the 68000 (m68k-elf-gcc -m68000 -O2, run through PJIT) and for the ARM (arm-none-eabi-gcc -O2, run
 * natively as the "ideal" translation). No 32-bit multiply or divide (no libgcc). Every kernel returns a checksum so both runs can be compared. */
#pragma GCC optimize ("O2,no-tree-loop-distribute-patterns")
typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32; typedef short s16; typedef int s32;
#define K static __attribute__((noinline))

K u32 k_strlen(const u8 *s, const u8 *unused, u32 n)  { const u8 *p = s; (void)unused; (void)n; while (*p) p++; return p - s; }
K u32 k_strcpy(u8 *d, const u8 *s, u32 n)             { u8 *o = d; (void)n; while ((*d++ = *s++)) ; return (u32)(d - o) + o[0]; }
K u32 k_strncpy(u8 *d, const u8 *s, u32 n)            { u32 i = 0; for (; i < n && s[i]; i++) d[i] = s[i]; for (; i < n; i++) d[i] = 0; return d[0] + d[n - 1] + n; }
K u32 k_memcpy(u8 *d, const u8 *s, u32 n)             { u32 i; for (i = 0; i < n; i++) d[i] = s[i]; return d[0] + d[n - 1]; }
K u32 k_memset(u8 *d, const u8 *unused, u32 n)        { u32 i; (void)unused; for (i = 0; i < n; i++) d[i] = 0x5A; return d[0] + d[n - 1]; }
K u32 k_strcmp(const u8 *a, const u8 *b, u32 n)       { (void)n; while (*a && *a == *b) { a++; b++; } return (u32)*a - (u32)*b; }
K u32 k_crc32(const u8 *s, const u8 *unused, u32 n)   { u32 crc = 0xFFFFFFFFu, i; int k; (void)unused; for (i = 0; i < n; i++) { crc ^= s[i]; for (k = 0; k < 8; k++) crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1)); } return ~crc; }
K u32 k_sum32(const u32 *a, const u32 *unused, u32 n) { u32 s = 0, i; (void)unused; for (i = 0; i < n; i++) s += a[i]; return s; }
K u32 k_csum16(const u16 *a, const u16 *unused, u32 n){ u32 s = 0, i; (void)unused; for (i = 0; i < n; i++) s += a[i]; while (s >> 16) s = (s & 0xFFFF) + (s >> 16); return s; }
K u32 k_dot16(const s16 *a, const s16 *b, u32 n)      { s32 s = 0; u32 i; for (i = 0; i < n; i++) s += (s32)a[i] * b[i]; return (u32)s; }
K u32 k_isort(s32 *a, const s32 *unused, u32 n)       { u32 i, j; (void)unused; for (i = 1; i < n; i++) { s32 v = a[i]; for (j = i; j > 0 && a[j - 1] > v; j--) a[j] = a[j - 1]; a[j] = v; } return (u32)a[0] + (u32)a[n - 1]; }
K u32 k_popcount(const u32 *a, const u32 *unused, u32 n){ u32 c = 0, i; (void)unused; for (i = 0; i < n; i++) { u32 v = a[i]; while (v) { v &= v - 1; c++; } } return c; }
K u32 k_bswap(u32 *a, const u32 *unused, u32 n)       { u32 i, s = 0; (void)unused; for (i = 0; i < n; i++) { u32 v = a[i]; v = (v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24); a[i] = v; s += v; } return s; }

/* name, call, kind of buffers: B bytes (src a string of 255 characters + NUL), W 16-bit, L 32-bit */
#define KERNELS(X) \
    X(strlen,  k_strlen,  255, 'B') X(strcpy,  k_strcpy,  255, 'B') X(strncpy, k_strncpy, 200, 'B') X(memcpy, k_memcpy, 256, 'B') X(memset, k_memset, 256, 'B') \
    X(strcmp,  k_strcmp,  255, 'B') X(crc32,   k_crc32,    64, 'B') X(sum32,   k_sum32,   128, 'L') X(csum16, k_csum16, 128, 'W') X(dot16,  k_dot16,   64, 'W') \
    X(isort,   k_isort,    24, 'L') X(popcount,k_popcount, 32, 'L') X(bswap,   k_bswap,   128, 'L')
