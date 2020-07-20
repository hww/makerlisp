/* MakerLisp Machine platform-specific definitions */
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <setjmp.h>
#include <setjmp.h>
#include <stdio.h>

/* Integer scanning and printing formats */
#define AINTFMT "%d"
#define AHEXFMT "%x"
#define ADDRFMT "%#x"

/* Data manager options */
#define ALIGNED(x) _Align x
#define CORRAL 1
#ifdef NDEBUG
/* Release configuration, code in ROM, data in RAM */
#define HEAPHALF (420*1024)
#else
/* Debug configuration, everything in RAM */
#define HEAPHALF (342*1024)
#endif
#define HEAPBMOD 1
#define SSTRINC 1

/* Reader options */
#define STARTUP "<makerlisp.l>"

/* Start of bss data, past all code */
#define BSSDATA &_low_bss
extern char _low_bss;

/* Setjmp/longjmp */
#define SETJMP(e) setjmp(e)
#define LONGJMP(e, r) eilongjmp(e, r);
#define JMP_BUF jmp_buf

/* Catch exceptions */
extern void (*hookbrk(void (*handler)()));
extern void (*hookfpe(void (*handler)()));

/* Initialize HW/OS platform */
extern void init_platform();

/* TTY control, status */
#define NLMTTY 1
extern int ttyopen();
extern void ttyclose();
extern int istty(FILE *input);
extern int ttykeyq();

/* MakerLisp file system root */
extern int getfsroot(char *buf, int maxn);

/* Enable interrupts and longjmp */
extern void eilongjmp(jmp_buf b, int r);

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
