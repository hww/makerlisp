//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#ifndef CHARIO_H
#define CHARIO_H
//------------------------------------------------------------------------
#include <defines.h>
#include <uart.h>
//------------------------------------------------------------------------
#define GETC( c, n ) read_UART0( &c, &n )
#define PUTC( c ) write_UART0( &c, 1 )
#define PRINT( s ) write_UART0( s, strlen(s) )
//------------------------------------------------------------------------
UINT8 consoleCharInReady( void );
char consoleCharIn( void );
UINT8 consoleCharOutReady( void );
void consoleCharOut( char c );
//------------------------------------------------------------------------
#endif // CHARIO_H
//------------------------------------------------------------------------
