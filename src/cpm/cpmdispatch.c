//------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//------------------------------------------------------------------------
//#define DEBUG
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//------------------------------------------------------------------------
#include <defines.h>
//------------------------------------------------------------------------
#include "cpmdispatch.h"
#include "cpmregisters.h"
//------------------------------------------------------------------------
// files containing bios functions to dispatch to...
//------------------------------------------------------------------------
#include "cpmload.h"
#include "cpmchario.h"
#include "cpmdiskio.h"
#include "cpmtable.h"
//------------------------------------------------------------------------
#ifdef DEBUG
#include <stdio.h>
#include "stringio.h"
#endif
//------------------------------------------------------------------------
#ifdef DEBUG
static char debugOutput[255];
#endif
//------------------------------------------------------------------------
typedef void (*functionP) (void);
//------------------------------------------------------------------------
// biosDispatchTable - contains pointers to bios functions
//------------------------------------------------------------------------
static functionP const biosDispatchTable[] =
{
    cpmLoadCcpBdos          // reloads ccp and bdos for bios warm boot

  , cpmConsoleCharInReady
  , cpmConsoleCharIn
  , cpmConsoleCharOutReady
  , cpmConsoleCharOut
  , cpmConsoleStringOut
  , cpmListCharOutReady
  , cpmListCharOut
  , cpmPunchCharOutReady
  , cpmPunchCharOut
  , cpmReaderCharInReady
  , cpmReaderCharIn
  , cpmSelectDisk
  , cpmHomeHead
  , cpmSetTrack
  , cpmSetSector
  , cpmSetDma
  , cpmReadSector
  , cpmWriteSector
  , cpmTranslateSector
  , cpmSetRamBase        // load disk tables into ram at supplied addr
};
//------------------------------------------------------------------------
// adl_biosDispatch - this function dispatches bios calls
//------------------------------------------------------------------------
void adl_biosDispatch( void )
{
    UINT8 vector = hlregister & 0xFF;

    biosDispatchTable[vector]();
}
//------------------------------------------------------------------------
