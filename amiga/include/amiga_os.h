/* amiga_os.h -- the few exec.library and dos.library calls the Amiga programs here make (amiga/lib/os.S: no Amiga SDK is needed). A call is a JSR through the library base
 * at the function's negative offset, with the arguments in the registers the library expects; the stubs load A6 themselves, so the C compiler stays free to use A6 as its
 * frame pointer. Every call here exists from Kickstart 1.2 on, except those marked dos 36 (Kickstart 2.0).
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#ifndef AMIGA_OS_H
#define AMIGA_OS_H
#include <stdint.h>

extern void *SysBase, *DOSBase;            /* set by start.S (SysBase) and by the program (DOSBase) */

#define MEMF_ANY        0u
#define MEMF_PUBLIC     1u
#define MEMF_CLEAR      0x10000u
#define MODE_OLDFILE    1005
#define OFFSET_BEGINNING (-1)
#define OFFSET_END      1

void   *OpenLibrary(const char *name, uint32_t version);
void    CloseLibrary(void *lib);
void   *AllocMem(uint32_t size, uint32_t flags);
void    FreeMem(void *mem, uint32_t size);
int32_t Open(const char *name, int32_t mode);           /* a file handle (BPTR), 0 on failure */
void    Close(int32_t fh);
int32_t Read(int32_t fh, void *buf, int32_t len);       /* bytes read, -1 on an error */
int32_t Write(int32_t fh, const void *buf, int32_t len);
int32_t Input(void);
int32_t Output(void);
int32_t Seek(int32_t fh, int32_t pos, int32_t mode);    /* the position before the seek, -1 on an error */
int32_t IoErr(void);
void    Delay(int32_t ticks);                           /* 50 ticks a second */
void    PrintFault(int32_t code, const char *header);   /* dos 36: "header: the error's text" on the output */
void   *ReadArgs(const char *tmpl, int32_t *array, void *rdargs);   /* dos 36: the Shell's argument line by a template; 0 on an error (IoErr) */
void    FreeArgs(void *rdargs);                         /* dos 36 */
void   *FindSegment(const char *name, void *start, int32_t system);   /* dos 36 and later: an entry of the Shell's resident list */

#endif
