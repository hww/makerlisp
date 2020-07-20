//----------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <defines.h>
//----------------------------------------------------------------------------
#include "cpmtable.h"
#include "cpmregisters.h"
#include "sdparams.h"
//----------------------------------------------------------------------------
typedef struct DiskParameterHeaderStruct
{
    UINT16  xlt;
    UINT16  w1;
    UINT16  w2;
    UINT16  w3;
    UINT16  dirbuf;
    UINT16  dpb;
    UINT16  csv;
    UINT16  alv;
} DiskParameterHeader;
//----------------------------------------------------------------------------
typedef struct DiskParameterBlockStruct
{
    UINT16  spt;
    UINT8   bsh;
    UINT8   blm;
    UINT8   exm;
    UINT16  dsm;
    UINT16  drm;
    UINT8   al0;
    UINT8   al1;
    UINT16  cks;
    UINT16  off;
} DiskParameterBlock;
//----------------------------------------------------------------------------
static unsigned char *nextByte_;
static DiskParameterHeader *dphP[SD_CARD_DISKS];
static DiskParameterBlock  *dpbsdP;
static unsigned char *dirbufP;
static unsigned char *csvP[SD_CARD_DISKS];
static unsigned char *allP[SD_CARD_DISKS];
//----------------------------------------------------------------------------
UINT16 getDphP( UINT8 disk )
{
    if ( disk < SD_CARD_DISKS )
    {
        return (UINT16) dphP[disk];
    }
    return 0x0000;
}
//----------------------------------------------------------------------------
void cpmSetRamBase( void )
{
    UINT8 disk;
    UINT24 addr = (UINT24) ( mbregister & 0xFF );
    addr <<= 16;
    addr += (UINT24) ( bcregister & 0xFFFF );

    // beginning of allocatable ram space above bios
    nextByte_ = (unsigned char *) addr;

    // disk parameter blocks
    for ( disk = 0; disk < SD_CARD_DISKS; ++disk )
    {
        dphP[disk] = (DiskParameterHeader *) nextByte_;
        nextByte_ += sizeof (DiskParameterHeader);
    }

    // allocation vector tables
    for ( disk = 0; disk < SD_CARD_DISKS; ++disk )
    {
        allP[disk] = nextByte_;
        nextByte_ += SD_ALLCATION_VECTOR_SIZE;
    }

    // directory checksum tables
    for ( disk = 0; disk < SD_CARD_DISKS; ++disk )
    {
        csvP[disk] = nextByte_;
        nextByte_ += SD_CHECKSUM_VECTOR_SIZE;
    }

    // allocate space for and build disk
    // disk parameter block for sd card hard drive images
    dpbsdP = (DiskParameterBlock *) nextByte_;
    nextByte_ += sizeof (DiskParameterBlock);
    dirbufP = nextByte_;
    nextByte_ += LOGICAL_SECTOR_SIZE;

    // initiate the sd card disk parameter block
    dpbsdP->spt = SD_SECTORS_PER_TRACK;     // 1024
    dpbsdP->bsh = SD_BLOCK_SHIFT_FACTOR;    // 5;
    dpbsdP->blm = SD_BLOCK_MASK;            // 31;
    dpbsdP->exm = SD_EXTENT_MASK;           // 1;
    dpbsdP->dsm = SD_DISK_BLOCKS - 1;       // 2047;
    dpbsdP->drm = SD_DIRECTORY_ENTRIES - 1; // 511;
    dpbsdP->al0 = SD_ALLOCATION_BITMAP_0;   // 0xf0;
    dpbsdP->al1 = SD_ALLOCATION_BITMAP_1;   // 0x00;
    dpbsdP->cks = SD_CHECKSUM_VECTOR_SIZE;  // 0;
    dpbsdP->off = SD_TRACK_OFFSET;          // 1;

    // initiate the disk parameter headers
    for ( disk = 0; disk < SD_CARD_DISKS; ++disk )
    {
        dphP[disk]->xlt = 0;
        dphP[disk]->w1 = 0;
        dphP[disk]->w2 = 0;
        dphP[disk]->w3 = 0;
        dphP[disk]->dirbuf = (UINT16) ( dirbufP );
        dphP[disk]->dpb = (UINT16) ( dpbsdP );
        dphP[disk]->csv = (UINT16) ( csvP[disk] );
        dphP[disk]->alv = (UINT16) ( allP[disk] );
    }
}
//----------------------------------------------------------------------------
