/* lzt.h -- boot-time loader for LZTB bundles (tools/pack/lzt.py, tools/pack/lzt_elf.py); the decoders are in lzt.s.
 *
 * A bundle is 'LZTB', n, n x { offset from the load base, size (a multiple of 256), packed size, log2 of the plane stride (0 = plain) }, then the LZ4 blocks (4-aligned).
 * Every part is decoded straight to base + offset (plain) or through `scratch` (byte planes, then the NEON untranspose); a part's size is padded to 256 bytes, so its zero
 * tail overwrites what follows it: parts are stored in ascending address order and the last one needs 16 bytes of slack.  `scratch`: the largest part + 16 bytes, 16-aligned;
 * the image and every destination 4-aligned (16 for the plane stride 16 / 8 / 4 outputs); NEON enabled (lzt_neon_enable), SCTLR.A clear.  Returns 0, or -1 for a bad magic. */
#ifndef PJIT_LZT_H
#define PJIT_LZT_H
#include <stdint.h>
extern uint32_t lz4_block(const uint8_t *src, uint32_t srclen, uint8_t *dst);
extern void     lzt_untranspose(const uint8_t *planes, uint8_t *dst, uint32_t records);    /* 16-byte records */
extern void     lzt_untranspose8(const uint8_t *planes, uint8_t *dst, uint32_t records);
extern void     lzt_untranspose4(const uint8_t *planes, uint8_t *dst, uint32_t records);
extern uint32_t lzt_unpack(const uint8_t *image, uint8_t *dst, uint8_t *scratch);          /* the single-block LZT1 image of tools/pack/lzt.py pack */
extern void     lzt_neon_enable(void);

static inline int lztb_load(const uint8_t *img, uint8_t *base, uint8_t *scratch)
{
    const uint32_t *h = (const uint32_t *)img;
    if (h[0] != 0x42545A4Cu) return -1;                       /* 'L','Z','T','B' */
    const uint32_t *p = h + 2; const uint8_t *data = img + 8 + 16 * h[1];
    for (uint32_t i = 0; i < h[1]; i++, p += 4) {
        uint8_t *dst = base + p[0];
        if (p[3] == 0) lz4_block(data, p[2], dst);
        else {
            lz4_block(data, p[2], scratch);
            if (p[3] == 4) lzt_untranspose(scratch, dst, p[1] >> 4);
            else if (p[3] == 3) lzt_untranspose8(scratch, dst, p[1] >> 3);
            else lzt_untranspose4(scratch, dst, p[1] >> 2);
        }
        data += (p[2] + 3) & ~3u;
    }
    return 0;
}
#endif
