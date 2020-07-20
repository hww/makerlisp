/* Linux platform-specific definitions */
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <setjmp.h>
#include <stdio.h>

/* Integer scanning and printing formats */
#define AINTFMT "%td"
#define AHEXFMT "%tx"
#define ADDRFMT "%#tx"

/* Data manager options */
#define ALIGNED(x) __attribute__((aligned(x)))
#define CORRAL 1
#define HEAPHALF (1 << 19)
#define HEAPBMOD 16
#define SSTRINC 8

/* Reader options */
#define STARTUP "<makerlisp.l>"

/* Setjmp/longjmp */
#define SETJMP(e) sigsetjmp(e, 1)
#define LONGJMP(e, r) siglongjmp(e, r)
#define JMP_BUF sigjmp_buf

/* Start of bss data, past all code */
#define BSSDATA &__bss_start
extern char __bss_start;

/* Initialize HW/OS platform */
extern void init_platform();

/* TTY control, status */
#define NLMTTY 0
extern int ttyopen();
extern void ttyclose();
extern int istty(FILE *input);
extern int ttykeyq();

/* MakerLisp file system root */
extern int getfsroot(char *buf, int maxn);

/* Catch exceptions */
extern void (*hookbrk(void (*handler)()));
extern void (*hookfpe(void (*handler)()));

/* Demonstration primitives (temporary) */
extern int cd(char *path);
extern void ls();

/* Directory operations */
extern int cd(char *path);
extern int closedir();
extern void lsitem(char *item);
extern int opendir(char *path);

/* Read and write I/O registers */
extern int readreg(int regaddr);
extern void writereg(int regaddr, int d);

/* File system sync */
extern void sync();

/* SPI mode and rate */
extern int mode_spi(int d);

/* SPI data read/write */
extern int xchg_spi(int d);

#endif /* _PLATFORM_H_ */
