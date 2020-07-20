/* Implementation of stdio that integrates ZSL and FATFS */
#include <cio.h>>
#include <ff.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uart.h>

/* FILE pointers for standard streams */
static FILE _stdin = {0, 0, NULL};
static char _stdout, _stderr;
FILE *stdin = &_stdin;
FILE *stdout = (FILE *)&_stdout;
FILE *stderr = (FILE *)&_stderr;

int fclose(FILE *fp)
{
    FRESULT r;

    r = f_close(fp->filp);

    free(fp->filp);
    free(fp);

    return r ? EOF : 0;
}

int feof(FILE *fp)
{
    return f_eof(fp->filp);
}

int fflush(FILE *fp)
{
    if (fp == stdin) {
        while (kbhit()) {
            getch();
        }
    }

    return 0;
}

int fgetc(FILE *fp)
{
    char c;
    UINT n;
    FRESULT r;

    if (!fp || (fp == stdout) || (fp == stderr)) {
        return EOF;
    }

    if (fp->ungetcnt) {
        fp->ungetcnt = 0;
        return (unsigned char)fp->ungetbuf;
    }

    if (fp == stdin) {
        return (unsigned char)getch();
    }

    r = f_read(fp->filp, &c, 1, &n);
    if (r || !n) {
        return EOF;
    }

    return (unsigned char)c;
}

char *fgets(char *buf, int n, FILE *fp)
{
    int i, r;

    i = 0;
    while (i < (n - 1)) {
        r = fgetc(fp);
        if (r < 0) {
            return i ? buf : NULL;
        }
        if (r == '\n') {
            break;
        }
        buf[i] = r;
        ++i;
    }
    buf[i] = '\0';

    return buf;
}

FILE *fopen(char *path, char *mode)
{
    BYTE ffmode;
    FIL *filp;
    FILE *fp;
    FRESULT r;

    /* Map C library modes to FAT fs modes */
    ffmode = FA_OPEN_EXISTING;
    if (!strcmp(mode, "r")) {
        ffmode = FA_READ;
    }
    if (!strcmp(mode, "r+")) {
        ffmode = FA_READ | FA_WRITE;
    }
    if (!strcmp(mode, "w")) {
        ffmode = FA_CREATE_ALWAYS | FA_WRITE;
    }
    if (!strcmp(mode, "w+")) {
        ffmode = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
    }
    if (!strcmp(mode, "a")) {
        ffmode = FA_OPEN_APPEND | FA_WRITE;
    }
    if (!strcmp(mode, "a+")) {
        ffmode = FA_OPEN_APPEND | FA_WRITE | FA_READ;
    }
    if (!strcmp(mode, "wx")) {
        ffmode = FA_CREATE_NEW | FA_WRITE;
    }
    if (!strcmp(mode, "w+x")) {
        ffmode = FA_CREATE_NEW | FA_WRITE | FA_READ;
    }

    filp = (FIL *)malloc(sizeof(FIL));
    if (!filp) {
        return NULL;
    }
    r = f_open(filp, path, ffmode);
    if (r) {
        free(filp);
        return NULL;
    }
    fp = (FILE *)malloc(sizeof(FILE));
    if (!fp) {
        free(filp);
        return NULL;
    }

    fp->filp = filp;
    fp->ungetcnt = 0;

    return fp;
}

int fputc(char c, FILE *fp)
{
    UINT n;
    FRESULT r;

    if (!fp || (fp == stdin)) {
        return EOF;
    }

    if ((fp == stdout) || (fp == stderr)) {
        putch(c);
        return (unsigned char)c;
    }

    r = f_write(fp->filp, &c, 1, &n);
    if (r || !n) {
        return EOF;
    }

    return (unsigned char)c;
}

int fputs(char *s, FILE *fp)
{
    char *cp;
    int r;

    cp = s;
    r = 0;
    while (*cp) {
        r = fputc(*cp, fp);
        if (r < 0) {
            break;
        }
        ++cp;
    }

    return r;
}

int fseek(FILE *fp, long offset, int whence)
{
    FSIZE_t foffs;

    if ((fp == stdin) || (fp == stdout) || (fp == stderr)) {
        return -1;
    }

    switch (whence) {
        case 0 :
            /* Absolute */
            foffs = offset;
            break;
        case 1 :
            /* Relative to current position */
            foffs = f_tell(fp->filp) + offset;
            break;
        case 2 :
            /* Relative to end of file */
            foffs = f_size(fp->filp) + offset;
            break;
        default :
            return -1;
            break;
    }

    fp->ungetcnt = 0;
    return f_lseek(fp->filp, foffs);
}

int remove(char *path)
{
    return f_unlink(path);
}

int rewind(FILE *fp)
{
    return (int)f_lseek(fp->filp, 0);
}

int setvbuf(FILE *fp, char *buf, int mode, size_t size)
{
    return 0;
}

int ungetc(int c, FILE *fp)
{
    if ((fp == stdout) || (fp == stderr)) {
        return EOF;
    }
    if (fp->ungetcnt) {
        return EOF;
    }

    fp->ungetbuf = (char)c;
    fp->ungetcnt = 1;

    return c;
}
