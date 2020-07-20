/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/



/** CONFIGURATION Bits **********************************************/
// FDEVOPT
#pragma config SOSCHP = OFF             // Secondary Oscillator High Power Enable bit (SOSC oprerates in normal power mode.)
#pragma config ALTI2C = OFF             // Alternate I2C1 Pins Location Enable bit (Primary I2C1 pins are used)
#pragma config FUSBIDIO = ON            // USBID pin control (USBID pin is controlled by the port function)
#pragma config FVBUSIO = OFF            // VBUS Pin Control (VBUS pin is controlled by the USB module)
#pragma config USERID = 0xFFFF          // User ID bits (User ID bits)

// FICD
#pragma config JTAGEN = OFF             // JTAG Enable bit (JTAG is disabled)
#pragma config ICS = PGx1               // ICE/ICD Communication Channel Selection bits (Communicate on PGEC1/PGED1)

// FPOR
#pragma config BOREN = BOR0             // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware; SBOREN bit disabled)
#pragma config RETVR = OFF              // Retention Voltage Regulator Enable bit (Retention regulator is disabled)
#pragma config LPBOREN = ON             // Downside Voltage Protection Enable bit (Low power BOR is enabled, when main BOR is disabled)

// FWDT
#pragma config SWDTPS = PS1048576       // Sleep Mode Watchdog Timer Postscale Selection bits (1:1048576)
#pragma config FWDTWINSZ = PS25_0       // Watchdog Timer Window Size bits (Watchdog timer window size is 25%)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Watchdog timer is in non-window mode)
#pragma config RWDTPS = PS1048576       // Run Mode Watchdog Timer Postscale Selection bits (1:1048576)
#pragma config RCLKSEL = LPRC           // Run Mode Watchdog Timer Clock Source Selection bits (Clock source is LPRC (same as for sleep mode))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT is disabled)

// FOSCSEL
#pragma config FNOSC = FRCDIV           // Oscillator Selection bits (Fast RC oscillator (FRC) with divide-by-N)
#pragma config PLLSRC = PRI             // System PLL Input Clock Selection bit (Primary oscillator is selected as PLL reference input on device reset)
#pragma config SOSCEN = OFF             // Secondary Oscillator Enable bit (Secondary oscillator (SOSC) is disabled)
#pragma config IESO = OFF               // Two Speed Startup Enable bit (Two speed startup is disabled)
#pragma config POSCMOD = HS             // Primary Oscillator Selection bit (HS oscillator mode is selected)
#pragma config OSCIOFNC = OFF           // System Clock on CLKO Pin Enable bit (OSCO pin operates as a normal I/O)
#pragma config SOSCSEL = OFF            // Secondary Oscillator External Clock Enable bit (Crystal is used (RA4 and RB4 are controlled by SOSC))
#pragma config FCKSM = CSECME           // Clock Switching and Fail-Safe Clock Monitor Enable bits (Clock switching is enabled; Fail-safe clock monitor is enabled)

// FSEC
#pragma config CP = OFF                 // Code Protection Enable bit (Code protection is disabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include "usb.h"
#include "system.h"
#include "io_mapping.h"
#include <sys/attribs.h>

/*********************************************************************
 * Prototypes
 *********************************************************************/
static void SYSTEM_SetOscToUSBCompatible(void);

/*********************************************************************
 * Functions
 *********************************************************************/

/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    switch(state)
    {
        case SYSTEM_STATE_USB_HOST:
            __builtin_enable_interrupts();
            SYSTEM_SetOscToUSBCompatible();
            PRINT_SetConfiguration(PRINT_CONFIGURATION_LCD);
            break;
            
        case SYSTEM_STATE_USB_HOST_HID_KEYBOARD:
            LED_Enable(LED_USB_HOST_HID_KEYBOARD_DEVICE_READY);
            PRINT_SetConfiguration(PRINT_CONFIGURATION_LCD);
            LCD_CursorEnable(true);
            TIMER_SetConfiguration(TIMER_CONFIGURATION_1MS);
            break;
        
        case SYSTEM_STATE_UART1:
            UART1_Initialize();
            break;
    }
}

/*********************************************************************
* Function: static void SYSTEM_SetOscToUSBCompatible(void)
*
* Overview: Configures the PIC32MM0256GPM064 PIM on the Explorer 16/32 to run 
*     in PRI+SPLL at 24MHz CPU, 48MHz USB module clock frequencies, both 
*     derived from the 8MHz crystal on the demo board.
*
* PreCondition: None
*
* Input:  None
*
* Output: None
*
********************************************************************/
static void SYSTEM_SetOscToUSBCompatible(void)
{
    // configure REFO to request POSC
    REFO1CONbits.ROSEL = 2; // POSC = 2
    REFO1CONbits.OE = 0; // disable output
    REFO1CONbits.ON = 1; // enable module

    // wait for POSC stable clock
    // this delay may vary depending on different application conditions
    // such as voltage, temperature, layout, XT or HS mode and components
    { // delay for 9 ms
        unsigned int start = __builtin_mfc0(_CP0_COUNT, _CP0_COUNT_SELECT);
        
        while((__builtin_mfc0(_CP0_COUNT, _CP0_COUNT_SELECT)) - start < (unsigned int)(0.009*8000000/2))
        {
        }
    }
    // unlock OSCCON
    SYSKEY = 0;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    
    //Configure the PLL to run from the POSC (8MHz on Explorer 16), and output 24MHz for the CPU + Peripheral Bus (and 48MHz for the USB module)
    SPLLCON = 0x02050000;     //PLLODIV = /4, PLLMULT = 12x, PLL source = POSC, so: 8MHz FRC * 12x / 4 = 24MHz CPU and peripheral bus frequency.
    
    // switch to POSC = 2
    OSCCONCLR = _OSCCON_NOSC_MASK | _OSCCON_CLKLOCK_MASK | _OSCCON_OSWEN_MASK;
    OSCCONSET = (1<<_OSCCON_NOSC_POSITION) | _OSCCON_OSWEN_MASK;
    while(OSCCONbits.OSWEN == 1){} // wait for switch  
}

/* Interrupt handler for USB host. */
void __ISR(_USB_VECTOR) _USB1Interrupt()
{
    USB_HostInterruptHandler();
}

