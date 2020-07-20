/* MakerLisp Machine low-level hardware support */
#include <defines.h>
#include <ez80.h>
#include <stddef.h>
#include <string.h>

#include "mlmhw.h"

/* Delays, SPI and SD card retry constants */
#define WAIT4RESET 10000
#define WAIT4CARD 10000
#define SDRETRY 2000
#define SPIRETRY 1000

/* Clear, set bits in registers */
#define CLEAR_BIT(reg, n) reg &= ~(1 << n)
#define SET_BIT(reg, n) reg |= (1 << n)

/* Reads and writes are SPI full duplex exchanges */
#define read_spi() _xchg_spi(0xFF)
#define write_spi(d) _xchg_spi(d)

/* Select and de-select card with GPIO pin */
#define select() CLEAR_BIT(PB_DR, 4)
#define deselect() (SET_BIT(PB_DR, 4), read_spi())

/* SD card commands */
typedef struct sdcmd {
    char num;
    UINT32 arg;
    char crc;
} SDCMD;
static SDCMD cmd0 = { 0x40, 0, 0x95 };
static SDCMD cmd8 = { 0x48, 0x1AA, 0x87 };
static SDCMD cmd12 = { 0x4c, 0, 0x87 };
static SDCMD cmd17 = { 0x51, 0, 0 };
static SDCMD cmd18 = { 0x52, 0, 0 };
static SDCMD cmd24 = { 0x58, 0, 0 };
static SDCMD cmd41 = { 0x69, 0x40000000, 0x77 };
static SDCMD cmd55 = { 0x77, 0, 0x65 };
static SDCMD cmd58 = { 0x7a, 0, 0 };
static SDCMD cmd59 = { 0x7b, 0, 0x91 };

/* RTC date and time array */
static unsigned char datetime[7];

/* Expansion board detected */
static char ebhere;

/* Short delay */
static void delay()
{
    volatile int delay;

    delay = 0;
    while (delay < 1) {
        ++delay;
    }
}

/* Reset system devices */
void init_mlmhw()
{
    long i;

    /* Port B pin 5 is 1 if expansion board is present */
    ebhere = PB_DR & (1 << 5);

    /* Set Port B pin 5 as output, assert /sysreset, SD card power off */
    CLEAR_BIT(PB_DR, 5);
    CLEAR_BIT(PB_ALT1, 5);
    CLEAR_BIT(PB_ALT2, 5);
    CLEAR_BIT(PB_DDR, 5);

    /* Set port B pins 7 (MOSI), 6 (MISO), 3 (SCK), 2 (/SS) to SPI */
    CLEAR_BIT(PB_ALT1, 7);
    CLEAR_BIT(PB_ALT1, 6);
    CLEAR_BIT(PB_ALT1, 3);
    CLEAR_BIT(PB_ALT1, 2);
    SET_BIT(PB_ALT2, 7);
    SET_BIT(PB_ALT2, 6);
    SET_BIT(PB_ALT2, 3);
    SET_BIT(PB_ALT2, 2);

    /* Set port B pin 4 as output, high - use for /CS */
    SET_BIT(PB_DR, 4);
    CLEAR_BIT(PB_ALT1, 4);
    CLEAR_BIT(PB_ALT2, 4);
    CLEAR_BIT(PB_DDR, 4);

    /* Leave /sysreset asserted for a while */
    i = 0;
    while (i < WAIT4RESET) {
        ++i;
    }

    /* Now let it go - and turn on SD card power */
    SET_BIT(PB_DR, 5);

    /* Wait for the SD card to power up */
    i = 0;
    while (i < WAIT4CARD) {
        ++i;
    }
}

/* SPI exchange transfer */
char _xchg_spi(char d)
{
    int i;

    /* Write the data to exchange */
    SPI_TSR = d;

    /* Wait for slave data to arrive */
    i = 0;
    while (i < SPIRETRY) {
        if (SPI_SR & (1 << 7)) {
            break;
        }
        ++i;
    }

    /* Delay, then read data */
    delay();

    return SPI_RBR;
}

/* Send a command to the SD card and get response, move data */
static int cmd_sdcard(SDCMD *cmd, BYTE *buf, int tok, int dlen, int ddir)
{
    char d, r;
    int i, l;

    /* Starting a command ? */
    r = 0xFF;
    if (cmd) {

        /* Select card, wait for ready */
        select();
        i = 0;
        while (i < SDRETRY) {
            d = read_spi();
            if (d == (char)0xFF) {
                break;
            }
            ++i;
        }
        if (!(i < SDRETRY)) {
            deselect();
            return -1;
        }

        /* Write command */
        write_spi(cmd->num);
        write_spi(cmd->arg >> 24);
        write_spi(cmd->arg >> 16);
        write_spi(cmd->arg >> 8);
        write_spi(cmd->arg >> 0);
        write_spi(cmd->crc);

        /* Wait for response */
        if (cmd->num == cmd12.num) {

            /* CMD12 stuff byte */
            read_spi();
        }
        i = 0;
        while (i < SDRETRY) {
            r = read_spi();
            if (!(r < 0)) {
                break;
            }
            ++i;
        }
        if (!(i < SDRETRY)) {
            deselect();
            return -1;
        }
    }

    /* Wait for or write data token */
    if (!(tok < 0)) {
        if (!ddir) {
            i = 0;
            while (i < SDRETRY) {
                r = read_spi();
                if (r == (char)tok) {
                    break;
                }
                ++i;
            }
            if (!(i < SDRETRY)) {
                deselect();
                return -1;
            }
        } else {
            read_spi();
            write_spi(tok);
        }
    }

    /* Move response or data */
    i = 0;
    l = (dlen < 0) ? -dlen : dlen;
    while (i < l) {
        if (!ddir) {
            buf[i] = read_spi();
        } else {
            write_spi(buf[i]);
        }
        ++i;
    }

    /* Wait for not busy, then deselect, if this is the end */
    if (!(dlen < 0)) {
        i = 0;
        while (i < SDRETRY) {
            d = read_spi();
            if (d == (char)0xFF) {
                break;
            }
            ++i;
        }
        deselect();
    }

    return (unsigned char)r;
}

/* Set up SD card for reads and writes - expect SD v2, HC */
int init_sdcard()
{
    BYTE response[4];
    int i;

    /* Set SPI in master mode, CPOL 0/CPHA 0 transfers, 200 kHz */
    SPI_BRG_H = 125 / 256;
    SPI_BRG_L = 125 % 256;
    SPI_CTL = 0x30;

    /* Write 80 clock pulses with /CS and DI (MOSI) high */
    deselect();
    i = 0;
    while (i < 10) {
        write_spi(0xFF);
        ++i;
    }

    /* Reset, put card in "SPI" mode */
    if (cmd_sdcard(&cmd0, NULL, -1, 0, 0) != 0x01) {
        return 1;
    }

    /* Check voltage range, SDC version 2 */
    if (cmd_sdcard(&cmd8, response, -1, 4, 0) != 0x01) {
        return 1;
    }

    /* Turn off CRC checking */
    if (cmd_sdcard(&cmd59, NULL, -1, 0, 0) != 0x01) {
        return 1;
    }

    /* Initialize */
    i = 0;
    while (i < SDRETRY) {
        cmd_sdcard(&cmd55, NULL, -1, 0, 0);
        if (cmd_sdcard(&cmd41, NULL, -1, 0, 0) == 0x00) {
            break;
        }
        ++i;
    }
    if (!(i < SDRETRY)) {
        return 1;
    }

    /* Read OCR register, check for block addressing */
    if (cmd_sdcard(&cmd58, response, -1, 4, 0) != 0x00) {
        return 1;
    }
    if (!(response[0] & 0x40)) {
        return 1;
    }

    /* Set SPI data rate to 8.33 MHz */
    SPI_CTL = 0;
    SPI_BRG_H = 3 / 256;
    SPI_BRG_L = 3 % 256;
    SPI_CTL = 0x30;
    delay();

    return 0;
}

/* Read blocks */
int read_sdcard(BYTE *buf, UINT32 n, UINT blks)
{
    BYTE crc[2];
    int i, r;

    if (blks == 1) {

        /* Single block ? */
        cmd17.arg = n;
        r = cmd_sdcard(&cmd17, buf, 0xFE, -512, 0);
        if (!(r == 0xFE)) {
            return 1;
        }
        cmd_sdcard(NULL, crc, -1, sizeof(crc), 0);
    } else {

        /* Multiple blocks */
        cmd18.arg = n;
        r = cmd_sdcard(&cmd18, NULL, -1, -1, 0);
        i = 0;
        while (i < blks) {
            r = cmd_sdcard(NULL, buf + i*512, 0xFE, -512, 0);;
            if (!(r == 0xFE)) {
                return 1;
            }
            cmd_sdcard(NULL, crc, -1, -sizeof(crc), 0);
            ++i;
        }
        r = cmd_sdcard(&cmd12, NULL, -1, 0, 0);
        if (r) {
            return 1;
        }
    }

    return 0;
}

/* Write a block */
int write_sdcard(BYTE *buf, UINT32 n)
{
    BYTE dr, crc[2];
    int i, r;

    cmd24.arg = n;
    r = cmd_sdcard(&cmd24, buf, 0xFE, -512, 1);
    cmd_sdcard(NULL, crc, -1, -sizeof(crc), 1);
    cmd_sdcard(NULL, &dr, -1, 1, 0);
        
    if (!(r == 0x00)) {
        return 1;
    }
    if (!((dr & 0x1F) == 0x05)) {
        return 1;
    } 

    return 0;
}

/* Read and write registers */
int _readreg(int regaddr)
{
    return *(volatile unsigned char __INTIO *)regaddr;
}
void _writereg(int regaddr, int d)
{
    *(volatile unsigned char __INTIO *)regaddr = d;
}

/* Get time from RTC */
UINT8 *get_time()
{
    /* Snapshot the time */
    RTC_CTRL = 0x21;
    datetime[6] = RTC_CEN;
    datetime[5] = RTC_YR;
    datetime[4] = RTC_MON;
    datetime[3] = RTC_DOM;
    datetime[2] = RTC_HRS;
    datetime[1] = RTC_MIN;
    datetime[0] = RTC_SEC;
    RTC_CTRL = 0x20;

    return &datetime;
}

/* Is expansion board attached ? */
int mlmeb()
{
    return ebhere ? 1 : 0;
}

/* SPI mode and rate */
int _mode_spi(int d)
{
    int oldmr;

    /* Get old mode and rate */
    oldmr = (SPI_BRG_H << 16) | (SPI_BRG_L << 8) | SPI_CTL;

    /* Set new mode and rate */
    SPI_CTL = 0;
    SPI_BRG_H = d >> 16;
    SPI_BRG_L = d >> 8;
    SPI_CTL = d;
    delay();

    return oldmr;
}
