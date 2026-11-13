/* console.h -- what the menus need from the console: a choice, a line, a yes or no, and the numbers typed into them. */
#ifndef CONSOLE_H
#define CONSOLE_H
#include <stdint.h>

int      MenuChoice(void);                          /* the prompt "Ready / ] ", then a number: -1 for an empty or non-numeric line */
char*    Ask(const char* prompt, char* buf, int size);  /* prints the prompt, reads a line */
int      Confirm(const char* question);             /* "question [y/N]? ": 1 for y */
int      ParseHex(const char* s, uint32_t* v);      /* hex digits, an optional leading $ or 0x: 1 if there were any */
int      ParseFixed(const char* s, int scale, int32_t* v);  /* a decimal number with up to log10(scale) decimals, times scale ("7.09", 1000 -> 7090): 1 if there were digits */

#endif
