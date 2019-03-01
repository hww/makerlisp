/* Linux platform-specific definitions */
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

/* Evaluator options */
#define FORGET 1

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
extern int ttyopen();
extern void ttyclose();
extern int istty(FILE *input);

/* Default include file path */
extern int getdefinc(char *buf, int maxn);

/* Catch exceptions */
extern void (*hookbrk(void (*handler)()));
extern void (*hookfpe(void (*handler)()));

/* Demonstration primitives (temporary) */
extern void led(int on);
extern void ls();

/* Read and write I/O registers */
extern int readreg(int regaddr);
extern void writereg(int regaddr, int d);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  TERMS OF USE: MIT License
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
