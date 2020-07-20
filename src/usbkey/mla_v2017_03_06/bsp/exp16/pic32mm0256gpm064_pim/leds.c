/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#include <xc.h>

#include <stdbool.h>

#include "leds.h"

#define LED_D3_LAT      LATDbits.LATD0
#define LED_D4_LAT      LATDbits.LATD1
#define LED_D5_LAT      LATDbits.LATD2
#define LED_D6_LAT      LATDbits.LATD3
#define LED_D7_LAT      LATDbits.LATD4
#define LED_D8_LAT      //Not connected
#define LED_D9_LAT      LATBbits.LATB14     //Overlaps USB Type-C VBUS Output Enable
#define LED_D10_LAT     LATCbits.LATC13     

#define LED_D3_TRIS     TRISDbits.TRISD0
#define LED_D4_TRIS     TRISDbits.TRISD1
#define LED_D5_TRIS     TRISDbits.TRISD2
#define LED_D6_TRIS     TRISDbits.TRISD3
#define LED_D7_TRIS     TRISDbits.TRISD4
#define LED_D8_TRIS     //Not connected
#define LED_D9_TRIS     TRISBbits.TRISB14   //Overlaps USB Type-C VBUS Output Enable
#define LED_D10_TRIS    TRISCbits.TRISC13   

#define LED_ON  1
#define LED_OFF 0

#define INPUT  1
#define OUTPUT 0

/*********************************************************************
* Function: void LED_On(LED led);
*
* Overview: Turns requested LED on
*
* PreCondition: LED configured via LED_Configure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_On(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_On(LED led)
{
    switch(led)
    {
        case LED_D3:
            LED_D3_LAT = LED_ON ;
            break;

        case LED_D4:
            LED_D4_LAT = LED_ON ;
            break;

        case LED_D5:
            LED_D5_LAT = LED_ON ;
            break ;

        case LED_D6:
            LED_D6_LAT = LED_ON ;
            break ;

        case LED_D7:
            LED_D7_LAT = LED_ON ;
            break ;
        case LED_D9:
            LED_D9_LAT = LED_ON ;
            break;

        case LED_D10:
            LED_D10_LAT = LED_ON ;
            break;

        default:
            break;
    }
}

/*********************************************************************
* Function: void LED_Off(LED led);
*
* Overview: Turns requested LED off
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Off(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_Off(LED led)
{
    switch(led)
    {
        case LED_D3:
            LED_D3_LAT = LED_OFF ;
            break;

        case LED_D4:
            LED_D4_LAT = LED_OFF ;
            break;

        case LED_D5:
            LED_D5_LAT = LED_OFF ;
            break ;

        case LED_D6:
            LED_D6_LAT = LED_OFF ;
            break ;

        case LED_D7:
            LED_D7_LAT = LED_OFF ;
            break ;
        case LED_D9:
            LED_D9_LAT = LED_OFF ;
            break;

        case LED_D10:
            LED_D10_LAT = LED_OFF ;
            break;

        default:
            break;
    }
}

/*********************************************************************
* Function: void LED_Toggle(LED led);
*
* Overview: Toggles the state of the requested LED
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Toggle(LED_CONNECTION_DETECTED);
*
* Output: none
*
********************************************************************/
void LED_Toggle(LED led)
{
    switch(led)
    {
        case LED_D3:
            LED_D3_LAT ^= 1 ;
            break;

        case LED_D4:
            LED_D4_LAT ^= 1 ;
            break;

        case LED_D5:
            LED_D5_LAT ^= 1 ;
            break ;

        case LED_D6:
            LED_D6_LAT ^= 1 ;
            break ;

        case LED_D7:
            LED_D7_LAT ^= 1 ;
            break ;

        case LED_D9:
            LED_D9_LAT ^= 1 ;
            break;

        case LED_D10:
            LED_D10_LAT ^= 1 ;
            break;

        default:
            break;
    }
}

/*********************************************************************
* Function: bool LED_Get(LED led);
*
* Overview: Returns the current state of the requested LED
*
* PreCondition: LED configured via LEDConfigure()
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*         i.e. - LED_Get(LED_CONNECTION_DETECTED);
*
* Output: true if on, false if off
*
********************************************************************/
bool LED_Get(LED led)
{
    switch(led)
    {

        case LED_D3:
            return ( ( LED_D3_LAT == LED_ON ) ? true : false ) ;

        case LED_D4:
            return ( ( LED_D4_LAT == LED_ON ) ? true : false ) ;

        case LED_D5:
            return ( ( LED_D5_LAT == LED_ON ) ? true : false ) ;

        case LED_D6:
            return ( ( LED_D6_LAT == LED_ON ) ? true : false ) ;

        case LED_D7:
            return ( ( LED_D7_LAT == LED_ON ) ? true : false ) ;

        case LED_D9:
            return ( ( LED_D9_LAT == LED_ON ) ? true : false ) ;

        case LED_D10:
            return ( ( LED_D10_LAT == LED_ON ) ? true : false ) ;

        default:
            break;
    }
    return false;
}

/*********************************************************************
* Function: void LED_Enable(LED led);
*
* Overview: Configures the LED for use by the other LED API
*
* PreCondition: none
*
* Input: LED led - enumeration of the LEDs available in this
*        demo.  They should be meaningful names and not the names of
*        the LEDs on the silkscreen on the board (as the demo code may
*        be ported to other boards).
*
* Output: none
*
********************************************************************/
void LED_Enable(LED led)
{
    switch(led)
    {

        case LED_D3:
            LED_D3_TRIS = OUTPUT ;
            break;

        case LED_D4:
            LED_D4_TRIS = OUTPUT ;
            break;

        case LED_D5:
            LED_D5_TRIS = OUTPUT ;
            break ;

        case LED_D6:
            LED_D6_TRIS = OUTPUT ;
            break ;

        case LED_D7:
            LED_D7_TRIS = OUTPUT ;
            break ;

        case LED_D9:
            LED_D9_TRIS = OUTPUT ;
            break;

        case LED_D10:
            LED_D10_TRIS = OUTPUT ;
            break;

        default:
            break;
    }
}

