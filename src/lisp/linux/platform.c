/* Linux platform-specific code */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>

static int fdtty;
static struct termios ttyattr;

void init_platform()
{
}

int ttyopen()
{
    /* Open TTY */
    fdtty = open("/dev/tty", O_RDWR);
    if (fdtty < 0) {
        return 0;
    }

    /* Make it character by character, we will echo ourselves */
    tcgetattr(fdtty, &ttyattr);
    ttyattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(fdtty, TCSANOW, &ttyattr);

    return 1;
}
void ttyclose()
{
    if (!(fdtty < 0)) {

        /* Restore defaults */
        ttyattr.c_lflag |= (ICANON | ECHO);
        tcsetattr(fdtty, TCSANOW, &ttyattr);
        close(fdtty);
    }
}

void (*hookbrk(void (*handler)()))
{
    return signal(SIGINT, handler);
}
void (*hookfpe(void (*handler)()))
{
    return signal(SIGFPE, handler);
}

int cd(char *path)
{
    return 0;
}

/* Open a directory */
int opendir(char *path)
{
    return -1;
}

/* Close the open directory */
int closedir()
{
    return -1;
}

/* Read a directory item */
void lsitem(char *file)
{
    file[0] = '\0';
}

int readreg(int regaddr)
{
#define RTC_SEC 0xE0
#define RTC_MIN 0xE1
#define RTC_HRS 0xE2
#define RTC_DOW 0xE3
#define RTC_DOM 0xE4
#define RTC_MON 0xE5
#define RTC_YR  0xE6
#define RTC_CEN 0xE7

#define BIN2BCD(n) ((n) / 10)*16 + ((n) % 10)

    struct tm timeinfo;
    time_t rawtime;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    switch (regaddr) {
        case RTC_SEC :
            return BIN2BCD(timeinfo.tm_sec);
        case RTC_MIN :
            return BIN2BCD(timeinfo.tm_min);
        case RTC_HRS :
            return BIN2BCD(timeinfo.tm_hour);
        case RTC_DOW :
            return BIN2BCD(timeinfo.tm_wday);
        case RTC_DOM :
            return BIN2BCD(timeinfo.tm_mday);
        case RTC_MON :
            return BIN2BCD(timeinfo.tm_mon + 1);
        case RTC_YR :
            return BIN2BCD((timeinfo.tm_year + 1900) % 100);
        case RTC_CEN :
            return BIN2BCD((timeinfo.tm_year + 1900) / 100);
    }

    return 0;
}
void writereg(int regaddr, int d)
{
}

int istty(FILE *stream)
{
    return isatty(fileno(stream));
}

#define SDEXTRA "../../uSDimage/lisp"
int getfsroot(char *buf, int maxn)
{
    int i, j;

    i = readlink("/proc/self/exe", buf, maxn - strlen(SDEXTRA));
    if (!(i < 0)) {
        buf[i] = '\0';
        i = j = 0;
        while (i < strlen(buf)) {
            if (buf[i] == '/') {
                j = i;
            }
            ++i;
        }
        buf[j + 1] = '\0';
        strcat(buf, SDEXTRA);
    }

    return i;
}

void sync()
{
}

/* Find out if there is a character waiting at the console */
int ttykeyq()
{
    int count;

    ioctl(fdtty, TIOCINQ, &count);

    return count;
}

/* SPI mode and rate */
int mode_spi(int d)
{
    return 0;
}

/* SPI data read/write */
int xchg_spi(int d)
{
    return 0;
}
