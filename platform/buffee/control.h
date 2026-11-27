/* control.h -- how a program on the Amiga talks to the PJIT payload (the `buffee` command, amiga/buffee/): the control page and the request block.
 *
 * The control page is the 4 KB at guest $F7F000, inside the MapROM megabyte that PJIT keeps in its own DRAM whatever the MapROM setting. The guest may read it (BFC_MAGIC and
 * the version at its start: a program checks them before anything else, so on a real 68000 or an older PJIT it writes nothing); it is read-only, and a long written to its
 * first word is the guest address of a request block. PJIT catches the write in its fault handler, serves the request with the guest stopped at that instruction, writes the
 * answer into the block and lets the guest go on: when the write instruction has finished, the answer is there. Nothing touches the Amiga's bus.
 *
 * The request block lives in the guest's memory (any RAM, chip or fast), big-endian like everything the 68000 sees. Text going either way is plain 8-bit characters with a
 * terminating zero. This header is included by the payload (ARM, little-endian), by the Amiga program (68000) and by the host test (tests/amiga/): it declares layout only.
 * Fields are only ever added at the end, and a program built for version 1 keeps working with a later PJIT.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#ifndef BUFFEE_CONTROL_H
#define BUFFEE_CONTROL_H
#include <stdint.h>

#define BFC_PAGE        0x00F7F000u     /* guest address of the control page */
#define BFC_CMD_TAG     0x00F01000u     /* guest address of the resident command's RomTag (amiga/buffee/resident.S; mk/amiga.mk links it there) */
#define BFC_MAGIC       0x42554643u     /* 'BUFC': the page's first long */
#define BFC_VERSION     1u              /* its second long */
#define BFC_REQ_MAGIC   0x42465251u     /* 'BFRQ': a request block's first long */
#define BFC_STAGE_MAX   0x00100000u     /* the largest image a program can send (BFC_UPLOAD): 1 MB */

enum {                                  /* bfc_req_t.cmd */
    BFC_STATUS = 1,                     /* the board, its flash and its settings as text, into reply. out[0], out[1]: the resident tag's init's report (BFC_RESIDENT) */
    BFC_SETTINGS,                       /* data: "item=value;..." as on the bootloader's settings page (docs/buffee/BUFFEE_CLI.md); saved in the EEPROM. Used at the next boot. */
    BFC_UPLOAD,                         /* data / len: a piece of an image, put at offset arg[0] of the board's staging buffer */
    BFC_STORE,                          /* the staged image into the flash. arg[0]: BFC_IMG_*; arg[1]: its length; arg[2]: its CRC-32 (zlib's) */
    BFC_VERIFY,                         /* the flash against an image: arg[0] BFC_IMG_*, arg[1] length, arg[2] CRC-32. out[0] = 1 if the flash holds that image and reads back whole */
    BFC_COPY_ROM,                       /* the Amiga's own ROM (as the MapROM settings place and size it) into the flash's ROM image */
    BFC_REBOOT,                         /* resets the board: the bootloader starts again, with the Amiga held in reset meanwhile */
    BFC_PACE,                           /* arg[0]: percent of an MC68000 for interpreted code, at once (0 = off). arg[1] bit 0: also save it */
    BFC_CLOCK,                          /* arg[0]: the ARM's clock in MHz, at once. arg[1] bit 0: also save it */
    BFC_CACHE,                          /* arg[0]: BFC_CACHE_* bits to set, arg[1]: bits to clear, at once. out[0]: the bits now */
    BFC_RESIDENT,                       /* from the resident tag's init (amiga/buffee/resident.S): arg[0] = BFC_RES_*, arg[1] = the use count (seg_UC) of the entry that
                                           FindSegment(name, 0, FALSE) returns just after AddSegment, as the Shell finds it ($80000000: none); kept for BFC_STATUS */
};
enum {                                  /* what the resident tag's init did */
    BFC_RES_NOT_RUN = 0,                /* (nothing reported since the board started) */
    BFC_RES_NO_DOS,                     /* dos.library 36 did not open */
    BFC_RES_NO_MEMORY,                  /* no memory for the copy */
    BFC_RES_NOT_ADDED,                  /* AddSegment refused it */
    BFC_RES_ADDED,                      /* in the Shell's resident list */
};
enum { BFC_IMG_PJIT = 1, BFC_IMG_ROM = 2 };
enum {                                  /* BFC_CACHE bits */
    BFC_CACHE_L1I    = 1,               /* the ARM's instruction cache */
    BFC_CACHE_L1D    = 2,               /* its data cache */
    BFC_CACHE_L2     = 4,               /* its level-2 cache */
    BFC_CACHE_SLOTS  = 8,               /* translated code: the slot cache (off: everything is interpreted) */
    BFC_CACHE_BLOCKS = 16,              /* translated code: hot code promoted to blocks */
};
enum {                                  /* bfc_req_t.result */
    BFC_OK         = 0,
    BFC_E_COMMAND  = -1,                /* unknown command, or not in this build */
    BFC_E_ARGS     = -2,                /* an argument out of range */
    BFC_E_RANGE    = -3,                /* the data does not fit the staging buffer */
    BFC_E_CRC      = -4,                /* the staged image does not have the CRC given */
    BFC_E_FORMAT   = -5,                /* not a PJIT bundle, or not a Kickstart of a size MapROM takes */
    BFC_E_FLASH    = -6,                /* the flash did not take it (written, then read back wrong) */
    BFC_E_EEPROM   = -7,                /* the EEPROM is missing or did not read back what was written */
    BFC_E_SETTING  = -8,                /* the settings text did not parse */
};

typedef struct {                        /* in guest memory, big-endian; 64 bytes */
    uint32_t magic;                     /* BFC_REQ_MAGIC (the program) */
    uint32_t cmd;                       /* BFC_* (the program) */
    uint32_t arg[4];                    /* (the program) */
    uint32_t data, len;                 /* the program: guest address and length of what it sends */
    uint32_t reply, reply_max;          /* the program: where the board's text goes (0: nowhere) and its size; the text always ends with a zero */
    int32_t  result;                    /* the board: BFC_OK or BFC_E_* */
    uint32_t out[5];                    /* the board: numbers, by command */
} bfc_req_t;

#endif
