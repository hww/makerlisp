/* Linux platform-specific code */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int fdtty;
static struct termios ttyattr;

void init_platform()
{
}

int ttyopen()
{
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

void led(on)
int on;
{
}
void ls()
{
}
int readreg(int regaddr)
{
    return 0;
}
void writereg(int regaddr, int d)
{
}

int istty(FILE *input)
{
    return isatty(fileno(input));
}

#define SDEXTRA ""
int getdefinc(char *buf, int maxn)
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
