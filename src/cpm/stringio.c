//------------------------------------------------------------------------
// Copyright (c) 2018, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//----------------------------------------------------------------------------
#include "ascii.h"
#include "chario.h"
#include "stringio.h"
//------------------------------------------------------------------------
void consoleStringOut( char *stringP )
{
    while ( *stringP )
    {
        consoleCharOut( *stringP++ );
    }
}
//------------------------------------------------------------------------
unsigned consoleStringIn( char *bufferP, unsigned size )
{
    char       *base;
    char        characterBuffer;
    unsigned    textLength;

    base = bufferP;
    textLength = 0;
   *bufferP = '\0';

    for ( ;; )
    {
        // get character from serial Port
        characterBuffer = consoleCharIn();

        // make sure 7 bit only
        characterBuffer &= 0x7F;

        // interpret control characters in input streem
        if ( characterBuffer < ' ' || characterBuffer == ASCII_DELETE )
        {
            switch ( characterBuffer )
            {

                case ASCII_DELETE :
                case ASCII_BACKSPACE :

                    // delete a character from input bufferP
                    if ( textLength != 0 )
                    {
                        consoleCharOut( ASCII_BACKSPACE );
                        consoleCharOut( ' ' );
                        consoleCharOut( ASCII_BACKSPACE );
                     *--bufferP = '\0';
                      --textLength;
                    }
                    else
                    {
                        consoleCharOut( ASCII_BELL );
                    }
                    continue;

                case ASCII_ETXCEL :

                    // delete characters from bufferP until bufferP empty
                    while ( textLength != 0 )
                    {
                        consoleCharOut( ASCII_BACKSPACE );
                        consoleCharOut( ' ' );
                        consoleCharOut( ASCII_BACKSPACE );
                     *--bufferP = '\0';
                      --textLength;
                    }
                    continue;

                case ASCII_ESCAPE :

                    // cancel input and return with empty bufferP
                   *base = '\0';
                    return 0;

                case ASCII_LINEFEED :
                case ASCII_ENTER :

                    // return with input in text bufferP
                    return textLength;

                default :

                    // any other control characters get you a slap on the wrist
                    consoleCharOut( ASCII_BELL );
                    continue;

            }
        }
        else
        {
            // if bufferP not full then put char in bufferP
            if ( textLength < (size-1) )
            {
                consoleCharOut( characterBuffer );
               *bufferP++ = characterBuffer;
               *bufferP   = '\0';
              ++textLength;
            }
            // else slap user on wrist
            else
            {
                consoleCharOut( ASCII_BELL );
            }
        }
    }
}
//------------------------------------------------------------------------
