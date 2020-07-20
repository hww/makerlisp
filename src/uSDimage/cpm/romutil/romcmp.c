/* Read flash rom and compare with file */
#include <stdio.h>
#include "ez80.h"

#define SIZE 262144

int main(argc, argv)
int argc;
char *argv[];
{
    unsigned char c;
    long i;
    FILE *fp;

    /* Check arguments */
    if (!(argc == 2)) {
        printf("Usage: romcmp <filename>\n");
        return 1;
    }

    /* Open file for reading */
    fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Couldn't open file \"%s\"", argv[1]);
        return 1;
    }

    /* Set frequency divider for 50 MHz */
    while (!(readreg(FLASH_FDIV) == 255)) {
        writereg(FLASH_KEY, 0xB6);
        writereg(FLASH_KEY, 0x49);
        writereg(FLASH_FDIV, 255);
    }

    /* Reset flash I/O access counter */
    writereg(FLASH_PAGE, 0);
    writereg(FLASH_ROW, 0);
    writereg(FLASH_COL, 0);

    /* Read flash and file a byte at a time, and compare */
    printf("Reading flash and file ");
    i = 0;
    while (i < SIZE) {
        if (((c = fgetc(fp)) == -1) || !(c == readreg(FLASH_DATA))) {
            break;
        }
        ++i;
        if (!(i % 1024)) {
            printf(".");
        }
    }

    fclose(fp);
    printf(" %ld\n", i);
}
