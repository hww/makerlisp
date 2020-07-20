/* MakerLisp Machine platform-specific code */
#include <diskio.h>
#include <ez80.h>
#include <ff.h>
#include <stdio.h>
#include <string.h>
#include <uart.h>

#include "mlmhw.h"
#include "platform.h"

/* Timeout on VGA ready */
#define WAIT4VGA 2000000

/* Filesystem data objects */
static FATFS fs;
static DIR dir;
static int diropen;

void init_platform()
{
    int i;

    init_mlmhw();

    /* Expansion board attached ? */
    i = 0;
    if (mlmeb()) {

        /* Wait for VGA ready */
        while (i < WAIT4VGA) {
            if (PB_DR & (1 << 1)) {
                break;
            }
            ++i;
        }
        if (!(i < WAIT4VGA)) {
            i = 0;
        }
    }

    /* If not VGA cold boot */
    if (!i) {

        /* Set newline mode, graphic attributes */
        printf("%c[20h%c[0m", 27, 27);

        /* Clear, home cursor, beep */
        printf("%c[2J%c[H\a", 27, 27);
    }

    /* Initialize filesystem stuff */
    diropen = 0;
    f_mount(&fs, "", (BYTE)1);
}

PFV hookbrk(PFV handler)
{
    PFV h;

    h = setbrkh_UART0(handler);

    return h;
}
PFV hookfpe(PFV handler)
{
    return NULL;
}

int ttyopen()
{
    return 0;
}

int istty(FILE *stream)
{
    return ((stream == stdin) || (stream == stdout) || (stream == stderr));
}
void ttyclose()
{
}

int getfsroot(buf, maxn)
char *buf;
int maxn;
{
    strcpy(buf, "");

    return strlen(buf);
}

void eilongjmp(JMP_BUF env, int r)
{
    ei();
    longjmp(env, r);
}

int cd(char *path)
{
    return (int)f_chdir(path);
}

/* Open a directory */
int opendir(char *path)
{
    FRESULT res;

    diropen = 0;
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        diropen = 1;
    }

    return (int)res;
}

/* Close the open directory */
int closedir()
{
    if (diropen) {
        diropen = 0;
        return (int)f_closedir(&dir);
    }

    return 0;
}

/* Read a directory item */
void lsitem(char *item)
{
    FILINFO fno;
    FRESULT res;

    if (!diropen) {
        res = FR_NO_PATH;
    } else {
        res = f_readdir(&dir, &fno);
    }
    if (!(res == FR_OK)) {
        item[0] = '\0';
        return;
    }

    strcpy(item, fno.fname);
}

int readreg(int regaddr)
{
    return _readreg(regaddr);
}

void writereg(int regaddr, int d)
{
    _writereg(regaddr, d);
}

void sync()
{
    disk_change();
}

/* Find out if there is a character waiting at the console */
int ttykeyq()
{
    return kbhit();
}

/* SPI mode and rate */
int mode_spi(int d)
{
    return _mode_spi(d);
}

/* SPI data read/write */
int xchg_spi(int d)
{
    return (unsigned char)_xchg_spi(d);
}
