//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <defines.h>
//------------------------------------------------------------------------
#include "chario.h"
#include "cpmchario.h"
#include "cpmregisters.h"
#include "stringio.h"
//------------------------------------------------------------------------
void cpmConsoleCharInReady( void )
{
    afregister = consoleCharInReady() << 8;
}
//------------------------------------------------------------------------
void cpmConsoleCharIn( void )
{
    afregister = consoleCharIn() << 8;
}
//------------------------------------------------------------------------
void cpmConsoleCharOutReady( void )
{
    afregister = consoleCharOutReady() << 8;
}
//------------------------------------------------------------------------
void cpmConsoleCharOut( void )
{
    consoleCharOut( bcregister );
}
//------------------------------------------------------------------------
void cpmConsoleStringOut( void )
{
    UINT24 stringP;

    stringP = (UINT24) ( mbregister & 0xFF );
    stringP <<= 16;
    stringP += (UINT24) ( bcregister & 0xFFFF );

    consoleStringOut( (char *) stringP );
}
//------------------------------------------------------------------------
void cpmListCharOutReady( void )
{
    afregister = 0xFF00;
}
//------------------------------------------------------------------------
void cpmListCharOut( void )
{
}
//------------------------------------------------------------------------
void cpmReaderCharInReady( void )
{
    afregister = 0xFF00;
}
//------------------------------------------------------------------------
void cpmReaderCharIn( void )
{
    int a;

    /* This routine has been hijacked for an eZ80-space input */
    a = bcregister;
    afregister = (*(volatile unsigned char __INTIO *)a) << 8; 
}
//------------------------------------------------------------------------
void cpmPunchCharOutReady( void )
{
    afregister = 0xFF00;
}
//------------------------------------------------------------------------
void cpmPunchCharOut( void )
{
    int a, d;

    /* This routine has been hijacked for an eZ80-space output */
    a = bcregister;
    d = deregister;
    *(volatile unsigned char __INTIO *)a = d;
}
//------------------------------------------------------------------------
