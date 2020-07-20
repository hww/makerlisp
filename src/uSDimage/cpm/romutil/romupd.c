/* Read rom image file and update flash rom */
#include <stdio.h>
#include "ez80.h"

#define SIZE 262144
#define keyp() bios(2, 0, 0)

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

    /* Warning */
    while (keyp()) getchar();
    printf("Warning ! This will erase the flash rom COMPLETELY !\n");
    printf("A new image will then be loaded to the flash rom.\n");
    printf("Press any key to continue, or ^C to abort ... ");
    c = getchar();
    printf("\n");

    /* Set frequency divider for 50 MHz */
    while (!(readreg(FLASH_FDIV) == 255)) {
        writereg(FLASH_KEY, 0xB6);
        writereg(FLASH_KEY, 0x49);
        writereg(FLASH_FDIV, 255);
    }

    /* Unprotect flash rom */
    while (!(readreg(FLASH_PROT) == 0)) {
        writereg(FLASH_KEY, 0xB6);
        writereg(FLASH_KEY, 0x49);
        writereg(FLASH_PROT, 0);
    }

    /* Mass erase */
    writereg(FLASH_PGCTL, 1);

    /* Reset flash I/O access counter */
    writereg(FLASH_PAGE, 0);
    writereg(FLASH_ROW, 0);
    writereg(FLASH_COL, 0);

    /* Read file and write to flash, byte by byte */
    printf("Writing flash ");
    i = 0;
    while (i < SIZE) {
        if ((c = fgetc(fp)) ==  -1) {
            printf("\nError reading file \"%s\"\n", argv[1]);
            fclose(fp);
            return 1;
        }
        writereg(FLASH_DATA, c);
        ++i;
        if (!(i % 1024)) {
            printf(".");
        }
    }

    fclose(fp);
    printf(" %ld\n\n", i);
    printf("Flash rom updated, reset to continue ... ");
    while (1);
}
