//------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <defines.h>
#include <ez80.h>
#include <string.h>
//------------------------------------------------------------------------
#include "cpmload.h"
#include "cpmregisters.h"
#include "disks.h"
#include "isrwrap.h"
#include "middle.h"
//------------------------------------------------------------------------
// Following allows emulation of a cp/m system ram that is smaller
// than 64k although why?  The alignment however, must always be 64k
// That is why two seperate constants are used.
//------------------------------------------------------------------------
UINT8 cpmRam[CPM_RAM_SIZE] _Align( CPM_RAM_ALIGNMENT );
//------------------------------------------------------------------------
void cpmLoadCcpBdos( void )
{
    memcpy(
       &cpmRam[CCP]
      , (char *) CCP
      , BIOS - CCP );
}
//------------------------------------------------------------------------
void loadCcpBdosBios( void )
{
    memcpy(
       &cpmRam[CCP]
      , (char *) CCP
      , 0x10000 - CCP );
}
//------------------------------------------------------------------------
void loadEntryAndStart( void )
{
    // make real entities to copy
    static UINT24 biosEntry = (UINT24) adl_biosEntry;
    static UINT16 start = BIOS;

    // address of middle ware bios services
    memcpy(
        &cpmRam[0x0000]
      , &biosEntry
      , sizeof biosEntry );

    // jump instruction
    cpmRam[0x0100] = 0xC3;

    // bios cold boot entry point
    memcpy(
        &cpmRam[0x0101]
      , &start
      , sizeof start );
}
//------------------------------------------------------------------------
void startCpm( void )
{
    initCpmDiskio();

    loadCcpBdosBios();
    loadEntryAndStart();

    /* Wrap UART ISRs */
    di();
    _set_vector( UART0_IVECT, isruart0 );
    _set_vector( UART1_IVECT, isruart1 );

    /* Enable interrupts and enter CP/M in mixed mode */
    mbregister = ( (UINT24) cpmRam ) >> 16;
    adl_startCpm();
}
//------------------------------------------------------------------------
