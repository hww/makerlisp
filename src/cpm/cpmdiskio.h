//----------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#ifndef CPMDISKIO_H
#define CPMDISKIO_H
//----------------------------------------------------------------------------
#define LOGICAL_SECTOR_SIZE  (128)
//----------------------------------------------------------------------------
void cpmSelectDisk( void );
void cpmHomeHead( void );
void cpmSetTrack( void );
void cpmSetSector( void );
void cpmSetDma( void );
void cpmReadSector( void );
void cpmWriteSector( void );
void cpmTranslateSector( void );
//----------------------------------------------------------------------------
#endif // CPMDISKIO_H
//----------------------------------------------------------------------------
