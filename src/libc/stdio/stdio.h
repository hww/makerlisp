/* Interface to stdio that integrates ZSL and FATFS */
#ifndef _STDIO_H_
#define _STDIO_H_

#include <ff.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>

/* Original Zilog code */
int printf(char * format, ...);
int scanf(char * format, ...);
int sprintf(char * s,char * format, ...);
int sscanf(char * s,char * format, ...);
int vprintf(char * format,va_list arg);
int vsprintf(char * s,char * format,va_list arg);

/* New code */
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

typedef struct file_struct {
    char ungetbuf;
    int ungetcnt;
    FIL *filp;
} FILE;

extern FILE *stdin, *stdout, *stderr;

extern int fclose(FILE *fp);
extern int feof(FILE *fp);
extern int fflush(FILE *fp);
extern int fgetc(FILE *fp);
extern char *fgets(char *buf, int n, FILE *fp);
extern FILE *fopen(char *path, char *mode);
extern int fputc(char c, FILE *fp);
extern int fputs(char *s, FILE *fp);
extern int fseek(FILE *fp, long offset, int whence);
extern int remove(char *path);
extern int rewind(FILE *fp);
extern int setvbuf(FILE *fp, char *buf, int mode, size_t size);
extern int ungetc(int c, FILE *fp);

#define getc fgetc
#define getchar() getc(stdin)
#define gets(b) fgets(b, INT_MAX, stdin)
#define putc fputc
#define putchar(c) putc(c, stdout)
#define puts(s) (!(fputs(s, stdout) < 0) ? putchar('\n') : EOF)

#endif /* _STDIO_H_ */
