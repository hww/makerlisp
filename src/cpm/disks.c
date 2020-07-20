//----------------------------------------------------------------------------
// Copyright (c) 2019, by Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <ctype.h>
#include <defines.h>
#include <string.h>
//----------------------------------------------------------------------------
#include "diskio.h"
#include "ff.h"
//----------------------------------------------------------------------------
#include "cpmdiskio.h"
#include "cpmtable.h"
#include "disks.h"
#include "sdparams.h"
//----------------------------------------------------------------------------
static UINT8          disk_;
static UINT16         track_;
static UINT16         sector_;
static unsigned char *dma_;
//------------------------------------------------------------------------
static char sectorBuffer[LOGICAL_SECTOR_SIZE];
//------------------------------------------------------------------------
static FATFS fatFileSystem;
//----------------------------------------------------------------------------
static int        imageFileOpen[SD_CARD_DISKS] = FALSE;
static FIL        imageFile[SD_CARD_DISKS];
static TCHAR const *imageFileName[SD_CARD_DISKS] =
{
    "cpmdiska.img"
  , "cpmdiskb.img"
  , "cpmdiskc.img"
  , "cpmdiskd.img"
};
//----------------------------------------------------------------------------
#define WRITE_TYPE_CAN_DEFER_WRITE ( 0 )
#define WRITE_TYPE_IMMEDIATE_WRITE ( 1 )
#define WRITE_TYPE_NO_PREREED      ( 2 )
//----------------------------------------------------------------------------
#define READ_RESULT_SUCCESS        ( 0 )
#define READ_RESULT_FAILURE        ( 1 )
#define READ_RESULT_MEDIA_CHANGE   ( 0xFF )
//----------------------------------------------------------------------------
#define WRITE_RESULT_SUCCESS       ( 0 )
#define WRITE_RESULT_FAILURE       ( 1 )
#define WRITE_RESULT_READ_ONLY     ( 2 )
#define WRITE_RESULT_MEDIA_CHANGE  ( 0xFF )
//----------------------------------------------------------------------------
static void closeImageFiles( void )
{
    UINT8 disk;

    f_unmount("");
    for ( disk = 0U; disk < SD_CARD_DISKS; ++disk )
    {
        imageFileOpen[disk] = FALSE;
    }
}
//----------------------------------------------------------------------------
static void openImageFiles( void )
{
    UINT8 disk;
    FRESULT r;
    TCHAR nbuf[FF_MAX_LFN + 1];

    f_mount(&fatFileSystem, "", (BYTE)1);
    for ( disk = 0U; disk < SD_CARD_DISKS; ++disk )
    {
        /* Try root and "/etc" */
        strcpy(nbuf, imageFileName[disk]);
        r = f_open(&imageFile[disk], nbuf, ( FA_READ | FA_WRITE ));
        if (!(r == FR_OK)) {
            strcpy(nbuf, "/etc/");
            strcat(nbuf, imageFileName[disk]);
            r = f_open(&imageFile[disk], nbuf, ( FA_READ | FA_WRITE ));
        }

        imageFileOpen[disk] = (r == FR_OK);
    }
}
//----------------------------------------------------------------------------
UINT16 selectDisk( UINT8 newLog, UINT8 disk )
{
    UINT16 r;

    r = getDphP( disk );
    if (!r) {
        return 0;
    }

    /* Check for disk change or no disk */
    if (disk_status(0)) {
        closeImageFiles();
        if (disk_initialize(0)) {
            return 0;
        }
        openImageFiles();
    }

    if (!imageFileOpen[disk]) {
        return 0;
    }

    disk_ = disk;
    return r;
}
//----------------------------------------------------------------------------
void homeHead( void )
{
    track_ = 0;
}
//----------------------------------------------------------------------------
void setTrack( UINT16 track )
{
    track_ = track;
}
//----------------------------------------------------------------------------
void setSector( UINT16 sector )
{
    sector_ = sector;
}
//----------------------------------------------------------------------------
void setDma( unsigned char *dma )
{
    dma_ = dma;
}
//----------------------------------------------------------------------------
static UINT32 calculateFilePosition(
    unsigned track
  , unsigned sector )
{
    UINT32 filePosition;
    filePosition  = track;
    filePosition *= SD_SECTORS_PER_TRACK;
    filePosition += sector;
    filePosition *= LOGICAL_SECTOR_SIZE;
    return filePosition;
}
//----------------------------------------------------------------------------
UINT8 readSector( void )
{
    UINT bytes;
    UINT32 filePosition;

    /* Check for disk change or no disk */
    if (disk_status(0)) {
        closeImageFiles();
        if (disk_initialize(0)) {
            return READ_RESULT_MEDIA_CHANGE;
        }
        openImageFiles();
    }

    if (!imageFileOpen[disk_]) {
        return READ_RESULT_FAILURE;
    }

    filePosition = calculateFilePosition(track_, sector_);

    if (f_lseek(&imageFile[disk_], filePosition)) {
        return READ_RESULT_FAILURE;
    }

    if (f_read(&imageFile[disk_], sectorBuffer, LOGICAL_SECTOR_SIZE, &bytes)) {
        return READ_RESULT_FAILURE;
    }

    memcpy(dma_, sectorBuffer, LOGICAL_SECTOR_SIZE);

    return READ_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------
UINT8 writeSector( UINT8 writeType )
{
    UINT bytes;
    UINT32 filePosition;

    /* Check for disk change or no disk */
    if (disk_status(0)) {
        closeImageFiles();
        if (disk_initialize(0)) {
            return WRITE_RESULT_MEDIA_CHANGE;
        }
        openImageFiles();
    }

    if (!imageFileOpen[disk_]) {
        return WRITE_RESULT_FAILURE;
    }

    filePosition = calculateFilePosition(track_, sector_);

    if (f_lseek(&imageFile[disk_], filePosition)) {
        return WRITE_RESULT_FAILURE;
    }

    memcpy(sectorBuffer, dma_, LOGICAL_SECTOR_SIZE);

    if (f_write(&imageFile[disk_], sectorBuffer, LOGICAL_SECTOR_SIZE,&bytes)) {
        return WRITE_RESULT_FAILURE;
    }

    return WRITE_RESULT_SUCCESS;
}
//----------------------------------------------------------------------------
UINT16 translateSector( UINT16 logical )
{
    return logical;
}
//----------------------------------------------------------------------------
void initCpmDiskio( void )
{
    openImageFiles();
}
//----------------------------------------------------------------------------
