//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include <defines.h>
#include <uart.h>
//------------------------------------------------------------------------
#include "chario.h"
//------------------------------------------------------------------------
UINT8 consoleCharInReady( void )
{
    return ( kbhit() ) ? 0xFF : 0x00;
}
//------------------------------------------------------------------------
char consoleCharIn( void )
{
    char c;
    unsigned n = 1;
    do
    {
        read_UART0( &c, &n );
    }
    while ( 0 == n );
    return c;
}
//------------------------------------------------------------------------
UINT8 consoleCharOutReady( void )
{
    return 0xFF;
}
//------------------------------------------------------------------------
void consoleCharOut( char c )
{
    UCHAR result;
    do
    {
        result = write_UART0( &c, 1 );
    }
    while ( result != UART_ERR_NONE );
}
//------------------------------------------------------------------------
