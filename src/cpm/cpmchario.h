//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#ifndef CPMCHARIO_H
#define CPMCHARIO_H
//------------------------------------------------------------------------
#include <uart.h>
//------------------------------------------------------------------------
#define GETC( c, n ) read_UART0( &c, &n )
#define PUTC( c ) write_UART0( &c, 1 )
#define PRINT( s ) write_UART0( s, strlen(s) )
//------------------------------------------------------------------------
void cpmConsoleCharInReady( void );
void cpmConsoleCharIn( void );
void cpmConsoleCharOutReady( void );
void cpmConsoleCharOut( void );
void cpmConsoleStringOut( void );
void cpmListCharOutReady( void );
void cpmListCharOut( void );
void cpmReaderCharInReady( void );
void cpmReaderCharIn( void );
void cpmPunchCharOutReady( void );
void cpmPunchCharOut( void );
//------------------------------------------------------------------------
#endif // CPMCHARIO_H
//------------------------------------------------------------------------
