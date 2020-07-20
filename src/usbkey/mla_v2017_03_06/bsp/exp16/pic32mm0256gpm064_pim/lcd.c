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
#include "lcd.h"
#include <stdint.h>

#ifndef SYSTEM_PERIPHERAL_CLOCK
#define SYSTEM_PERIPHERAL_CLOCK 8000000
#pragma message "This module requires a definition for the peripheral clock frequency.  Assuming 8MHz Fcy (8MHz Fosc).  Define value if this is not correct."
#endif

/* This defines the number of cycles per loop through the delay routine.  Spans
 * between 12-18 depending on optimization mode.*/
#define CYCLES_PER_DELAY_LOOP 10

/* Private Definitions ***********************************************/

/* Define a delay for the various time intervals required between various signal
 * transitions (E pulse, R/W setup, etc.).  This will be rounded up to 1us for 
 * the sake of easy calculation though all of these are less than that
 * typically 60-300ns. */
#define LCD_SIGNAL_TIMING         (((SYSTEM_PERIPHERAL_CLOCK/1000)*1)/1000)/CYCLES_PER_DELAY_LOOP

// Define a fast instruction execution time in terms of loop time
// typically > 40us
#define LCD_F_INSTR         (((SYSTEM_PERIPHERAL_CLOCK/1000)*40)/1000)/CYCLES_PER_DELAY_LOOP

// Define a slow instruction execution time in terms of loop time
// typically > 1.64ms
#define LCD_S_INSTR         (((SYSTEM_PERIPHERAL_CLOCK/1000)*1640)/1000)/CYCLES_PER_DELAY_LOOP

// Define the startup time for the LCD in terms of loop time
// typically > 60ms (double than 8 bit mode)
#define LCD_STARTUP         (((SYSTEM_PERIPHERAL_CLOCK/1000)*60000)/1000)/CYCLES_PER_DELAY_LOOP

#define LCD_MAX_COLUMN      16

#define LCD_COMMAND_CLEAR_SCREEN        0x01
#define LCD_COMMAND_RETURN_HOME         0x02
#define LCD_COMMAND_ENTER_DATA_MODE     0x06
#define LCD_COMMAND_CURSOR_OFF          0x0C
#define LCD_COMMAND_CURSOR_ON           0x0F
#define LCD_COMMAND_MOVE_CURSOR_LEFT    0x10
#define LCD_COMMAND_MOVE_CURSOR_RIGHT   0x14
#define LCD_COMMAND_SET_MODE_4_BIT      0x28
#define LCD_COMMAND_SET_MODE_8_BIT      0x38
#define LCD_COMMAND_ROW_0_HOME          0x80
#define LCD_COMMAND_ROW_1_HOME          0xC0
#define LCD_START_UP_COMMAND_1          0x33    
#define LCD_START_UP_COMMAND_2          0x32  

#define LCD_D4_Enable()             TRISCbits.TRISC14 = 0;
#define LCD_D4_Write(value)         LATCbits.LATC14 = value;
#define LCD_D5_Enable()             TRISAbits.TRISA13 = 0;
#define LCD_D5_Write(value)         LATAbits.LATA13 = value;
#define LCD_D6_Enable()             TRISAbits.TRISA14 = 0;
#define LCD_D6_Write(value)         LATAbits.LATA14 = value;
#define LCD_D7_Enable()             TRISAbits.TRISA15 = 0;
#define LCD_D7_Write(value)         LATAbits.LATA15 = value;

#define LCD_RSSignal_Set()        LATCbits.LATC10 = 1 //set Register Select bit
#define LCD_RSSignal_Clear()      LATCbits.LATC10 = 0 //clear Register Select bit
#define LCD_RWSignal_Set()        LATAbits.LATA1 = 1  //set Read/Write bit
#define LCD_RWSignal_Clear()      LATAbits.LATA1 = 0  //clear Read/Write bit
#define LCD_EnableSignal_Set()    LATAbits.LATA5 = 1  //set Enable bit
#define LCD_EnableSignal_Clear()  LATAbits.LATA5 = 0  //clear Enable bit
#define LCD_RSSignal_Input()      TRISCbits.TRISC10 = 1 //set Register Select bit
#define LCD_RSSignal_Output()     TRISCbits.TRISC10 = 0 //clear Register Select bit
#define LCD_RWSignal_Input()      TRISAbits.TRISA1 = 1  //set Read/Write bit
#define LCD_RWSignal_Output()     TRISAbits.TRISA1 = 0  //clear Read/Write bit
#define LCD_EnableSignal_Input()  TRISAbits.TRISA5 = 1  //set Enable bit
#define LCD_EnableSignal_Output() TRISAbits.TRISA5 = 0  //clear Enable bit

typedef union
{
    uint8_t value;
    struct
    {
        unsigned b0 :1;
        unsigned b1 :1;
        unsigned b2 :1;
        unsigned b3 :1;
        unsigned b4 :1;
        unsigned b5 :1;
        unsigned b6 :1;
        unsigned b7 :1;
    } bits;
} BIT_MAPPING;

/* Private Functions *************************************************/
static void LCD_CarriageReturn ( void ) ;
static void LCD_ShiftCursorLeft ( void ) ;
static void LCD_ShiftCursorRight ( void ) ;
static void LCD_ShiftCursorUp ( void ) ;
static void LCD_ShiftCursorDown ( void ) ;
static void LCD_Wait ( uint32_t ) ;
static void LCD_SendData ( char ) ;
static void LCD_SendCommand ( char command, uint32_t delay );

/* Private variables ************************************************/
static uint8_t row ;
static uint8_t column ;
/*********************************************************************
 * Function: bool LCD_Initialize(void);
 *
 * Overview: Initializes the LCD screen.  Can take several hundred
 *           milliseconds.
 *
 * PreCondition: none
 *
 * Input: None
 *
 * Output: true if initialized, false otherwise
 *
 ********************************************************************/
bool LCD_Initialize ( void )
{   
    LCD_D4_Write(0);
    LCD_D5_Write(0);
    LCD_D6_Write(0);
    LCD_D7_Write(0);
    
    LCD_D4_Enable();
    LCD_D5_Enable();
    LCD_D6_Enable();
    LCD_D7_Enable();
    
    // Control signal data pins
    LCD_RWSignal_Clear ( ) ; // LCD R/W signal
    LCD_RSSignal_Clear ( ) ; // LCD RS signal
    LCD_EnableSignal_Clear ( ) ;     // LCD E signal

    // Control signal pin direction
    LCD_RSSignal_Output ( )  ;
    LCD_RWSignal_Output ( )  ;
    LCD_EnableSignal_Output ( ) ;

    LCD_EnableSignal_Set ( )  ;

    LCD_Wait ( LCD_STARTUP ) ;
    
    LCD_SendCommand ( LCD_START_UP_COMMAND_1 , LCD_F_INSTR) ;
    LCD_SendCommand ( LCD_START_UP_COMMAND_2 , LCD_F_INSTR) ;
    
    LCD_SendCommand ( LCD_COMMAND_SET_MODE_4_BIT , LCD_F_INSTR  ) ;
    LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF , LCD_F_INSTR  ) ;
    LCD_SendCommand ( LCD_COMMAND_ENTER_DATA_MODE , LCD_F_INSTR ) ;

    LCD_ClearScreen ( ) ;

    return true ;
}
/*********************************************************************
 * Function: void LCD_PutString(char* inputString, uint16_t length);
 *
 * Overview: Puts a string on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.  Will terminate when either a
 *           null terminator character (0x00) is reached or the length number
 *           of characters is printed, which ever comes first.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: char* - string to print
 *        uint16_t - length of string to print
 *
 * Output: None
 *
 ********************************************************************/
void LCD_PutString ( char* inputString , uint16_t length )
{
    while (length--)
    {
        switch (*inputString)
        {
            case 0x00:
                return ;

            default:
                LCD_PutChar ( *inputString++ ) ;
                break ;
        }
    }
}
/*********************************************************************
 * Function: void LCD_PutChar(char);
 *
 * Overview: Puts a character on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: char - character to print
 *
 * Output: None
 *
 ********************************************************************/
void LCD_PutChar ( char inputCharacter )
{
    static char lastCharacter = 0;
        
    switch (inputCharacter)
    {
        case '\r':
            if(lastCharacter != '\n')
            {
                LCD_CarriageReturn ( ) ;
            }
            break ;

        case '\n': 
            if(lastCharacter != '\r')
            {
                LCD_CarriageReturn ( ) ;
            }
            
            if (row == 0)
            {
                LCD_ShiftCursorDown ( ) ;
            }
            else
            {
                LCD_ShiftCursorUp ( ) ;
            }
            break ;

        case '\b':
            LCD_ShiftCursorLeft ( ) ;
            LCD_PutChar ( ' ' ) ;
            LCD_ShiftCursorLeft ( ) ;
            break ;
            
        case '\f':
            LCD_ClearScreen();
            break;

        default:
            if (column == LCD_MAX_COLUMN)
            {
                column = 0 ;
                if (row == 0)
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME, LCD_F_INSTR ) ;
                    row = 1 ;
                }
                else
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME, LCD_F_INSTR ) ;
                    row = 0 ;
                }
            }
            
            LCD_SendData ( inputCharacter ) ;
            column++ ;
            break ;
    }
    
    lastCharacter = inputCharacter;
}
/*********************************************************************
 * Function: void LCD_ClearScreen(void);
 *
 * Overview: Clears the screen, if possible.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
void LCD_ClearScreen ( void )
{
    LCD_SendCommand ( LCD_COMMAND_CLEAR_SCREEN , LCD_S_INSTR ) ;
    LCD_SendCommand ( LCD_COMMAND_RETURN_HOME , LCD_S_INSTR) ;

    row = 0 ;
    column = 0 ;
}


/*******************************************************************/
/*******************************************************************/
/* Private Functions ***********************************************/
/*******************************************************************/
/*******************************************************************/
/*********************************************************************
 * Function: static void LCD_CarriageReturn(void)
 *
 * Overview: Handles a carriage return
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_CarriageReturn ( void )
{
    if (row == 0)
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
    }
    column = 0 ;
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorLeft(void)
 *
 * Overview: Shifts cursor left one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorLeft ( void )
{
    uint8_t i ;

    if (column == 0)
    {
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR ) ;
            row = 0 ;
        }

        //Now shift to the end of the row
        for (i = 0 ; i < ( LCD_MAX_COLUMN - 1 ) ; i++)
        {
            LCD_ShiftCursorRight ( ) ;
        }
    }
    else
    {
        column-- ;
        LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_LEFT , LCD_F_INSTR ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorRight(void)
 *
 * Overview: Shifts cursor right one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorRight ( void )
{
    LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_RIGHT , LCD_F_INSTR ) ;
    column++ ;

    if (column == LCD_MAX_COLUMN)
    {
        column = 0 ;
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR ) ;
            row = 0 ;
        }
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorUp(void)
 *
 * Overview: Shifts cursor up one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorUp ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorLeft ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorDown(void)
 *
 * Overview: Shifts cursor down one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorDown ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorRight ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_Wait(unsigned int B)
 *
 * Overview: A crude wait function that just cycle burns
 *
 * PreCondition: None
 *
 * Input: unsigned int - arbitrary delay time based on loop counts.
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_Wait ( uint32_t delay )
{
    while (delay)
    {
        Nop();
        delay-- ;
    }
}
/*********************************************************************
 * Function: void LCD_CursorEnable(bool enable)
 *
 * Overview: Enables/disables the cursor
 *
 * PreCondition: None
 *
 * Input: bool - specifies if the cursor should be on or off
 *
 * Output: None
 *
 ********************************************************************/
void LCD_CursorEnable ( bool enable )
{
    if (enable == true)
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_ON , LCD_F_INSTR ) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF , LCD_F_INSTR ) ;
    }
}

/*********************************************************************
 * Function: static void LCD_SendData(char data)
 *
 * Overview: Sends data to LCD
 *
 * PreCondition: None
 *
 * Input: char - contains the data to be sent to the LCD
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_SendData ( char data )
{
    BIT_MAPPING temp;
    
    temp.value = (uint8_t)data;
    
    LCD_RWSignal_Clear ( ) ;
    
    LCD_RSSignal_Set ( ) ;
    
    LCD_D4_Write(temp.bits.b4);
    LCD_D5_Write(temp.bits.b5);
    LCD_D6_Write(temp.bits.b6);
    LCD_D7_Write(temp.bits.b7);
    LCD_Wait( LCD_SIGNAL_TIMING);
    LCD_EnableSignal_Set ( ) ;
    LCD_Wait( LCD_SIGNAL_TIMING);
    LCD_EnableSignal_Clear ( ) ;
    
    LCD_D4_Write(temp.bits.b0);
    LCD_D5_Write(temp.bits.b1);
    LCD_D6_Write(temp.bits.b2);
    LCD_D7_Write(temp.bits.b3);
    LCD_Wait( LCD_SIGNAL_TIMING);
    LCD_EnableSignal_Set ( ) ;
    LCD_Wait( LCD_SIGNAL_TIMING);
    LCD_EnableSignal_Clear ( ) ;
    
    LCD_RSSignal_Clear ( ) ;
 
    LCD_Wait ( LCD_F_INSTR ) ;
}

/*********************************************************************
 * Function: static void LCD_SendCommand(char data)
 *
 * Overview: Sends command to LCD
 *
 * PreCondition: None
 *
 * Input: char - contains the command to be sent to the LCD
 *        unsigned int - has the specific delay for the command
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_SendCommand ( char command , uint32_t delay)
{   
    BIT_MAPPING temp;
    
    LCD_RWSignal_Clear ( ) ;
    LCD_RSSignal_Clear ( ) ;
    
    temp.value = (uint8_t)command;
	
    LCD_D4_Write(temp.bits.b4);
    LCD_D5_Write(temp.bits.b5);
    LCD_D6_Write(temp.bits.b6);
    LCD_D7_Write(temp.bits.b7);
    LCD_EnableSignal_Set ( ) ;
    LCD_Wait( LCD_SIGNAL_TIMING);
    LCD_EnableSignal_Clear ( ) ;
    
    LCD_RWSignal_Clear ( ) ;
    LCD_RSSignal_Clear ( ) ;
    LCD_D4_Write(temp.bits.b0);
    LCD_D5_Write(temp.bits.b1);
    LCD_D6_Write(temp.bits.b2);
    LCD_D7_Write(temp.bits.b3);
    LCD_EnableSignal_Set ( ) ;
    LCD_Wait( LCD_SIGNAL_TIMING );
    
    LCD_EnableSignal_Clear ( ) ;    
    LCD_Wait(delay);
}
