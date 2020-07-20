/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for MakerLisp Machine port of FatFs         */
/*-----------------------------------------------------------------------*/

#include <ez80.h>
#include <string.h>

#include "mlmhw.h"
#include "diskio.h"   /* FatFs lower layer API */

/* Sector size */
#define SECTSIZ 512

/* Drive status */
static DSTATUS dstat = STA_NOINIT;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv         /* Physical drive number to identify the drive */
)
{
    BYTE buf[SECTSIZ];

    if (pdrv) {
        return RES_PARERR;
    }

    /* If two seconds elapsed since last access, maybe card has changed */
    if (!dstat) {
        if (!(TMR0_CTL & 0x01)) {

            /* Card is out or just inserted and isn't initialized yet */
            if (read_sdcard(buf, 0, 1)) {
                dstat = STA_NOINIT;
            } else {
                /* Still in - restart two second countdown timer */
                TMR0_CTL = 0x23;
            }
        }
    }

    return dstat;
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv         /* Physical drive number to identify the drive */
)
{
    if (pdrv) {
        return RES_PARERR;
    }

    /* Initialize SD card */
    if (init_sdcard()) {
        dstat = STA_NOINIT;
    } else {
        /* Start two second count down timer */
        dstat = 0;
        TMR0_CTL = 0x23;
    }
        
    return dstat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive number to identify the drive */
    BYTE *buff,       /* Data buffer to store read data */
    DWORD sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
)
{
    if (pdrv) {
        return RES_PARERR;
    }

    if (dstat == STA_NOINIT) {
        return RES_NOTRDY;
    }

    if (read_sdcard(buff, sector, count)) {
        return RES_ERROR;
    }

    /* Restart two second countdown timer */
    TMR0_CTL = 0x23;

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
    BYTE pdrv,        /* Physical drive number to identify the drive */
    const BYTE *buff, /* Data to be written */
    DWORD sector,     /* Start sector in LBA */
    UINT count        /* Number of sectors to write */
)
{
    BYTE *b;
    UINT i;
    DWORD s;

    if (pdrv) {
        return RES_PARERR;
    }

    if (dstat == STA_NOINIT) {
        return RES_NOTRDY;
    }

    b = buff;
    s = sector;
    i = 0;
    while (i < count) {
        if (write_sdcard(b, s)) {
            return RES_ERROR;
        }
        b += SECTSIZ;
        ++s;
        ++i;
    }

    /* Restart two second countdown timer */
    TMR0_CTL = 0x23;

    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive number (0..) */
    BYTE cmd,         /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    switch (cmd) {
        case CTRL_SYNC :
            return RES_OK;
        default :
            return RES_PARERR;
    }
}

/* FAT filesystem time */
static DWORD bcd2bin(UINT8 bcdn)
{
    return (((DWORD)bcdn & 0xF0) >> 4)*10 + ((DWORD)bcdn & 0x0F);
}
DWORD get_fattime()
{
    UINT8 *t;
    DWORD fattime, year, month, dom, hour, minute, second2;

    /* Grab the time */
    t = get_time();

    /* Put in FAT fs format */
    year = bcd2bin(t[6])*100 + bcd2bin(t[5]) - 1980; 
    month = bcd2bin(t[4]);
    dom = bcd2bin(t[3]);
    hour = bcd2bin(t[2]);
    minute = bcd2bin(t[1]);
    second2 = bcd2bin(t[0]) / 2;

    fattime = (year << 25) | (month << 21) | (dom << 16) | \
        (hour << 11) | (minute << 5) | second2;

    return fattime;
}

/* Force recognition of medium change */
void disk_change()
{
    dstat = STA_NOINIT;
}
