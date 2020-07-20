//----------------------------------------------------------------------------
// Copyright (c) 2019, by Christopher D. Farrar
//----------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <defines.h>
//----------------------------------------------------------------------------
#include "cpmdiskio.h"
#include "cpmregisters.h"
#include "disks.h"
//----------------------------------------------------------------------------
void cpmSelectDisk( void )
{
    hlregister = (UINT24) selectDisk( (UINT8) ( ( deregister & 0x0001 ) != 0 ), (UINT8) bcregister );
}
//----------------------------------------------------------------------------
void cpmHomeHead( void )
{
    homeHead();
}
//----------------------------------------------------------------------------
void cpmSetTrack( void )
{
    setTrack( (UINT16) bcregister );
}
//----------------------------------------------------------------------------
void cpmSetSector( void )
{
    setSector( (UINT16) bcregister );
}
//----------------------------------------------------------------------------
void cpmSetDma( void )
{
    UINT24 dma = mbregister & 0xFF;
    dma <<= 16;
    dma += ( bcregister & 0xFFFF );
    setDma( (unsigned char *) dma );
}
//----------------------------------------------------------------------------
void cpmReadSector( void )
{
    afregister = (UINT24) readSector() << 8;
}
//----------------------------------------------------------------------------
void cpmWriteSector( void )
{
    afregister = (UINT24) writeSector( (UINT8) bcregister ) << 8;
}
//----------------------------------------------------------------------------
void cpmTranslateSector( void )
{
    hlregister = (UINT24) translateSector( (UINT16) bcregister );
}
//----------------------------------------------------------------------------
