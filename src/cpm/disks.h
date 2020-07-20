//----------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#ifndef DISKS_H
#define DISKS_H
//----------------------------------------------------------------------------
#include <defines.h>
//----------------------------------------------------------------------------
UINT16 selectDisk( UINT8 newLog, UINT8 disk );
void homeHead( void );
void setTrack( UINT16 track );
void setSector( UINT16 sector );
void setDma( unsigned char *dma );
UINT8 readSector( void );
UINT8 writeSector( UINT8 writeType );
UINT16 translateSector( UINT16 logical );
void setRamBase( unsigned char *ramBase );
//----------------------------------------------------------------------------
void initCpmDiskio( void );
//----------------------------------------------------------------------------
#endif // DISKS_H
//----------------------------------------------------------------------------
