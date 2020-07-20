//------------------------------------------------------------------------
// Copyright (c) 2019, Christopher D. Farrar
//------------------------------------------------------------------------
// I here grant permission to any and all to copy and use this software for
// any purpose as long as my copyright message is retained.
//------------------------------------------------------------------------
#include <ez80.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "cpmload.h"
#include "mlmhw.h"
//------------------------------------------------------------------------

/* Timeout on VGA ready */
#define WAIT4VGA 2000000

int main()
{
    int i;

    /* Initialize MakerLisp CPU */
    init_mlmhw();

    /* Expansion board attached ? */
    i = 0;
    if (mlmeb()) {

        /* Wait for VGA ready */
        while (i < WAIT4VGA) {
            if (PB_DR & (1 << 1)) {
                break;
            }
            ++i;
        }
        if (!(i < WAIT4VGA)) {
            i = 0;
        }
    }

    /* If not VGA cold boot */
    if (!i) {

        /* Set graphic attributes, beep */
        printf("\x1b[0m\a");
    }

    /* Set non-newline mode */
    printf("\x1b[20l");

    /* Start CP/M */
    for ( ;; )
    {
        startCpm();
    }
}
//------------------------------------------------------------------------
