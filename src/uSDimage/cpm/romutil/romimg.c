/* Read flash rom and save to file */
#include <stdio.h>
#include "ez80.h"

#define SIZE 262144L

int main(argc, argv)
int argc;
char *argv[];
{
    long i;
    FILE *fp;

    /* Check arguments */
    if (!(argc == 2)) {
        printf("Usage: romimg <filename>\n");
        return 1;
    }

    /* Open file for writing */
    fp = fopen(argv[1], "w");
    if (!fp) {
        printf("Couldn't open file %s\n", argv[1]);
        return 1;
    }

    /* Set frequency divider for 50 MHz */
    while (!(readreg(FLASH_FDIV)) == 255)) {
        writereg(FLASH_KEY, 0xB6);
        writereg(FLASH_KEY, 0x49);
        writereg(FLASH_FDIV, 255);
    }

    /* Reset flash I/O access counter */
    writereg(FLASH_PAGE, 0);
    writereg(FLASH_ROW, 0);
    writereg(FLASH_COL, 0);

    /* Read flash a byte at a time, and write it out */
    printf("Reading flash ");
    i = 0;
    while (i < SIZE) {
        if (fputc(readreg(FLASH_DATA), fp) == -1) {
            printf("\nError writing to file \"%s\"\n", argv[1]);
            fclose(fp);
            return 1;
        }
        ++i;
        if (!(i % 1024)) {
            printf(".");
        }
    }

    printf(" %ld\n", i);
    fclose(fp);

    return 0;
}
