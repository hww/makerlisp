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

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#include "usb.h"
#include "usb_host_hid.h"

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "system.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fwupd.h"
#include "timer_1ms.h"
#include "uart1.h"

/* ASCII control characters */
#define CTRL(c) (c - 64)

#define NUL CTRL('@')
#define SOH CTRL('A')
#define STX CTRL('B')
#define ETX CTRL('C')
#define EOT CTRL('D')
#define ENQ CTRL('E')
#define ACK CTRL('F')
#define BEL CTRL('G')
#define BS  CTRL('H')
#define TAB CTRL('I')
#define LF CTRL('J')
#define VT CTRL('K')
#define FF CTRL('L')
#define CR CTRL('M')
#define SO CTRL('N')
#define SI CTRL('O')
#define DLE CTRL('P')
#define DC1 CTRL('Q')
#define DC2 CTRL('R')
#define DC3 CTRL('S')
#define DC4 CTRL('T')
#define NAK CTRL('U')
#define SYN CTRL('V')
#define ETB CTRL('W')
#define CAN CTRL('X')
#define EM CTRL('Y')
#define SUB CTRL('Z')
#define ESC CTRL('[')
#define FS CTRL('\\')
#define GS CTRL(']')
#define RS CTRL('^')
#define US CTRL('_')

/* Delete, "rub-out" */
#define DEL 127

// *****************************************************************************
// *****************************************************************************
// Type definitions
// *****************************************************************************
// *****************************************************************************

typedef enum _APP_STATE
{
    DEVICE_NOT_CONNECTED,
    WAITING_FOR_DEVICE,
    DEVICE_CONNECTED, /* Device Enumerated  - Report Descriptor Parsed */
    GET_INPUT_REPORT, /* perform operation on received report */
    INPUT_REPORT_PENDING,
    SEND_OUTPUT_REPORT, /* Not needed in case of mouse */
    OUTPUT_REPORT_PENDING,
    ERROR_REPORTED
} KEYBOARD_STATE;

#define PARSED_DATA_LEN 8
struct parsed_key
{
    HID_DATA_DETAILS details;
    HID_USER_DATA_SIZE data[PARSED_DATA_LEN];
    HID_USER_DATA_SIZE oldData[PARSED_DATA_LEN];
};

typedef struct
{
    uint8_t address;
    KEYBOARD_STATE state;
    bool inUse;

    struct
    {
        uint16_t id;
        uint16_t size;
        uint16_t pollRate;
        uint8_t *buffer;

        struct parsed_key normal;
        struct parsed_key modifier;
    } keys;

    struct
    {
        bool updated;
        
        union
        {
            uint8_t value;
            struct
            {
                uint8_t  numLock       : 1;
                uint8_t  capsLock      : 1;
                uint8_t  scrollLock    : 1;
                uint8_t                : 5;
            } bits;
        } report;

        HID_DATA_DETAILS details;

    } leds;
} KEYBOARD;

typedef struct
{
    USB_HID_KEYBOARD_KEYPAD key;
    char unmodified[4];
    char shifted[4];
    char controlled[4];
} HID_KEY_TRANSLATION_TABLE_ENTRY;

#define MAX_ERROR_COUNTER               (10)

/* Protect variables shared between base and interrupt level */
#define GET_KB_VAR(var, member) ( __builtin_disable_interrupts(), _kb_var.member = var, __builtin_enable_interrupts(), _kb_var.member )
#define SET_KB_VAR(var, val) { __builtin_disable_interrupts(); var = val; __builtin_enable_interrupts(); }

// *****************************************************************************
// *****************************************************************************
// Local Variables
// *****************************************************************************
// *****************************************************************************

/* Auto-repeat */
#define FIRST_DELAY 60
static HID_USER_DATA_SIZE keydown = 0x00;
static char downkey[4] = "";
static int downcnt = 0;
static int rptdelay = FIRST_DELAY;

/* Identifiers in response to ENQ */
static char kb_ident[] = "$Id: 0123456789$";
static char vga_ident[64];

/* ESC command parameters */
static int escpn;
static uint8_t escparm[8];

/* ROM read/write command program flash address */
static uint32_t romaddr;

/* For wrapper that accesses a protected state variable */
static union
{
    KEYBOARD_STATE state;
} _kb_var;

static KEYBOARD keyboard;

/* Model a DEC vt5xx terminal, enhanced PC keyboard, in SCO UNIX console + newline mode */
static HID_KEY_TRANSLATION_TABLE_ENTRY keyTranslationTable[] = 
{
    /* Ordinary ASCII keys */
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A,                                         "a", "A", {CTRL('A')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_B,                                         "b", "B", {CTRL('B')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_C,                                         "c", "C", {CTRL('C')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_D,                                         "d", "D", {CTRL('D')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_E,                                         "e", "E", {CTRL('E')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F,                                         "f", "F", {CTRL('F')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_G,                                         "g", "G", {CTRL('G')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_H,                                         "h", "H", {CTRL('H')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_I,                                         "i", "I", {CTRL('I')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_J,                                         "j", "J", {CTRL('J')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_K,                                         "k", "K", {CTRL('K')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_L,                                         "l", "L", {CTRL('L')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_M,                                         "m", "M", {CTRL('M')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_N,                                         "n", "N", {CTRL('N')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_O,                                         "o", "O", {CTRL('O')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_P,                                         "p", "P", {CTRL('P')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Q,                                         "q", "Q", {CTRL('Q')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_R,                                         "r", "R", {CTRL('R')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_S,                                         "s", "S", {CTRL('S')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_T,                                         "t", "T", {CTRL('T')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_U,                                         "u", "U", {CTRL('U')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_V,                                         "v", "V", {CTRL('V')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_W,                                         "w", "W", {CTRL('W')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_X,                                         "x", "X", {CTRL('X')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Y,                                         "y", "Y", {CTRL('Y')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z,                                         "z", "Z", {CTRL('Z')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_1_AND_EXCLAMATION_POINT,                   "1", "!", "1" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_2_AND_AT,                                  "2", "@", {CTRL('@')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_3_AND_HASH,                                "3", "#", "3" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_4_AND_DOLLAR,                              "4", "$", "4" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_5_AND_PERCENT,                             "5", "%", "5" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_6_AND_CARROT,                              "6", "^", {CTRL('^')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_7_AND_AMPERSAND,                           "7", "&", "7" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_8_AND_ASTERISK,                            "8", "*", "8" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_9_AND_OPEN_PARENTHESIS,                    "9", "(", "9" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_0_AND_CLOSE_PARENTHESIS,                   "0", ")", "0" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_MINUS_AND_UNDERSCORE,                      "-", "_", {CTRL('_')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_EQUAL_AND_PLUS,                            "=", "+", "=" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_OPEN_BRACKET_AND_OPEN_CURLY_BRACE,         "[", "{", {CTRL('[')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CLOSE_BRACKET_AND_CLOSE_CURLY_BRACE,       "]", "}", {CTRL(']')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_BACK_SLASH_AND_PIPE,                       "\\", "|", {CTRL('\\')} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_NON_US_HASH_AND_TILDE,                     "`", "~", "`" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SEMICOLON_AND_COLON,                       ";", ":", ";" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_APOSTROPHE_AND_QUOTE,                      "'", "\"","'" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_GRAVE_ACCENT_AND_TILDE,                    "`", "~", "`" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_COMMA_AND_LESS_THAN,                       ",", "<", "," },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_PERIOD_AND_GREATER_THAN,                   ".", ">", "." },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_FORWARD_SLASH_AND_QUESTION_MARK,           "/", "?", "/" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_SPACEBAR,                                  " ", " ", " " },

    /* Some special keys */
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER,                              "\r\n", "\r\n", "\r\n" },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE,                                    "\b", "\b", "\b" },

    /* Editing keys */
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_UP_ARROW,                                  {ESC, '[', 'A'}, {ESC, '[', 'A'}, {ESC, '[', 'A'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DOWN_ARROW,                                {ESC, '[', 'B'}, {ESC, '[', 'B'}, {ESC, '[', 'B'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_ARROW,                               {ESC, '[', 'C'}, {ESC, '[', 'C'}, {ESC, '[', 'C'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_ARROW,                                {ESC, '[', 'D'}, {ESC, '[', 'D'}, {ESC, '[', 'D'} },

    /* Function keys */
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F1,                                        {ESC, '[', 'M'}, {ESC, '[', 'Y'}, {ESC, '[', 'k'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F2,                                        {ESC, '[', 'N'}, {ESC, '[', 'Z'}, {ESC, '[', 'l'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F3,                                        {ESC, '[', 'O'}, {ESC, '[', 'a'}, {ESC, '[', 'm'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F4,                                        {ESC, '[', 'P'}, {ESC, '[', 'b'}, {ESC, '[', 'n'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F5,                                        {ESC, '[', 'Q'}, {ESC, '[', 'c'}, {ESC, '[', 'o'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F6,                                        {ESC, '[', 'R'}, {ESC, '[', 'd'}, {ESC, '[', 'p'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F7,                                        {ESC, '[', 'S'}, {ESC, '[', 'e'}, {ESC, '[', 'q'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F8,                                        {ESC, '[', 'T'}, {ESC, '[', 'f'}, {ESC, '[', 'r'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F9,                                        {ESC, '[', 'U'}, {ESC, '[', 'g'}, {ESC, '[', 's'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F10,                                       {ESC, '[', 'V'}, {ESC, '[', 'h'}, {ESC, '[', 't'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F11,                                       {ESC, '[', 'W'}, {ESC, '[', 'i'}, {ESC, '[', 'u'} },
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_F12,                                       {ESC, '[', 'X'}, {ESC, '[', 'j'}, {ESC, '[', 'v'} },

    /* Editing */
    { USB_HID_KEYBOARD_KEYPAD_KEYBOARD_DELETE_FORWARD,                            {DEL}, {DEL}, {DEL} },

    /* Keypad */
    { USB_HID_KEYBOARD_KEYPAD_KEYPAD_PERIOD_AND_DELETE,                           {DEL}, ".", {DEL} }
};


// *****************************************************************************
// *****************************************************************************
// Local Function Prototypes
// *****************************************************************************
// *****************************************************************************
static void App_ProcessInputReport(void);

// *****************************************************************************
// *****************************************************************************
// Functions
// *****************************************************************************
// *****************************************************************************

/* Send character string value of key to display / UART1 */
static void SendStr(char *s)
{
    int i, n;

    n = strlen(s);
    if (!n)
    {
        n = 1;
    }
    i = 0;
    while (i < n)
    {
        UART1_Write(s[i]);
        PRINT_Char(s[i]);
        ++i;
    }
}
static void NRSendStr(char *s)
{
    HID_USER_DATA_SIZE savekeydown;
    
    SET_KB_VAR(keydown, (savekeydown = keydown, 0x00));
    
    SendStr(s);
    
    SET_KB_VAR(keydown, savekeydown);
}

/* Load entry in translation table */
static void keyload(USB_HID_KEYBOARD_KEYPAD key, char *seq1, char *seq2, char *seq3)
{
    int i, n;
    HID_KEY_TRANSLATION_TABLE_ENTRY entry;
    
    /* Find the entry */
    i = 0;
    n = sizeof(keyTranslationTable)/sizeof(HID_KEY_TRANSLATION_TABLE_ENTRY);
    while (i < n)
    {
        if (keyTranslationTable[i].key == key)
        {
            break;
        }
        ++i;
    }
    if (!(i < n))
    {
        /* No matching entry in table */
        return;
    }
    
    /* Load it */
    strcpy(keyTranslationTable[i].unmodified, seq1);
    strcpy(keyTranslationTable[i].shifted, seq2);
    strcpy(keyTranslationTable[i].controlled, seq3);
}

/* Get ESC command parameters */
static uint8_t get1p(uint8_t i)
{
    int c, n, r;

    c = n = r = 0;
    while (isdigit((c = UART1_Read())))
    {
        r = r*10;
        r = r + (c - '0');
        n = 1;
    }

    escparm[i] = r;
    escpn += n;

    return c;
}
static uint8_t getescp()
{
    int c, i;

    i = 0;
    escpn = 0;
    while (i < sizeof(escparm))
    {
        c = get1p(i);
        if (!isdigit(c) && !(c == ';'))
        {
            break;
        }
        ++i;
    }

    return c;
}

/*********************************************************************
* Function: void APP_HostHIDTimerHandler(void);
*
* Overview: Switches over the state machine state to get a new report
*           periodically if the device is idle
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/

static void APP_HostHIDTimerHandler(void)
{
    /* Poll keyboard, at 120 Hz */
    if(keyboard.state == DEVICE_CONNECTED)
    {
        keyboard.state = GET_INPUT_REPORT;

        /* If last event was key press, long enough ago, repeat */
        if (keydown)
        {
            ++downcnt;
            if (!(downcnt < rptdelay))
            {
                SendStr(downkey);
                downcnt = 0;
                
                /* Subsequent repeats at 20 Hz */
                rptdelay = 6;
            }
        }
    }
}

/*********************************************************************
* Function: void APP_HostHIDKeyboardInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_HostHIDKeyboardInitialize()
{
    romaddr = 0;
    vga_ident[0] = '\0';

    keyboard.state = DEVICE_NOT_CONNECTED;
    keyboard.inUse = false;
    keyboard.keys.buffer = NULL;
    keyboard.address = 0;
    
    LED_Enable(LED_USB_HOST_HID_KEYBOARD_CAPS_LOCK);
    LED_Off(LED_USB_HOST_HID_KEYBOARD_CAPS_LOCK);
}

/*********************************************************************
* Function: void APP_HostHIDKeyboardTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized via
*   the APP_HostHIDKeyboardInitialize()
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_HostHIDKeyboardTasks()
{
    char idstr[128];
    uint8_t c, d, dc[4];
    uint8_t error;
    uint8_t count;
    
    if(keyboard.address == 0)
    {
        keyboard.address = USBHostHIDDeviceDetect();
    }
    else
    {
        if(USBHostHIDDeviceStatus(keyboard.address) == USB_HID_DEVICE_NOT_FOUND)
        {
            SET_KB_VAR(keyboard.state, DEVICE_NOT_CONNECTED);
            keyboard.address = 0;
            keyboard.inUse = false;

            if(keyboard.keys.buffer != NULL)
            {
                free(keyboard.keys.buffer);
                keyboard.keys.buffer = NULL;
            }
        }      
    }

    switch (GET_KB_VAR(keyboard.state, state))
    {
        case DEVICE_NOT_CONNECTED:
            PRINT_ClearScreen();
            PRINT_String("Attach keyboard\r\n", 17);
            memset(&keyboard.keys, 0x00, sizeof(keyboard.keys));
            memset(&keyboard.leds, 0x00, sizeof(keyboard.leds));
            SET_KB_VAR(keyboard.state, WAITING_FOR_DEVICE);
            LED_Off(LED_USB_HOST_HID_KEYBOARD_DEVICE_READY);
            LED_Off(LED_USB_HOST_HID_KEYBOARD_CAPS_LOCK);
            PRINT_CursorEnable(false);
            TIMER_CancelTick(&APP_HostHIDTimerHandler);
            break;
            
        case WAITING_FOR_DEVICE:
            if( (keyboard.address != 0) &&
                (USBHostHIDDeviceStatus(keyboard.address) == USB_HID_NORMAL_RUNNING)
              ) /* True if report descriptor is parsed with no error */
            {
                PRINT_ClearScreen();
                PRINT_CursorEnable(true);
                LED_On(LED_USB_HOST_HID_KEYBOARD_DEVICE_READY);
                
                SET_KB_VAR(keyboard.state, DEVICE_CONNECTED);
                
                /* Timer runs at 960 Hz, our handler at 120 Hz */
                TIMER_RequestTick(&APP_HostHIDTimerHandler, 8);
            }
            break;
            
        case DEVICE_CONNECTED:
            
            /* Check for inquiry, XON/XOFF, or command */
            if (UART1_CharReady())
            {
                c = UART1_Read();
                switch (c)
                {
                    case ENQ :
                        /* Inquiry - send identification strings */
                        strcpy(idstr, "MakerLisp Keyboard ");
                        strcat(idstr, kb_ident);
                        strcat(idstr, " Display ");
                        strcat(idstr, vga_ident);
                        NRSendStr(idstr);
                        break;
                    case DC1 :
                    case DC1 + 128 :
                    case DC3 :
                    case DC3 + 128 :
                        /* XON/XOFF */
                        sprintf(dc, "%c%c", c, 0);
                        NRSendStr(dc);
                        break;
                    case ESC :
                        /* Possibly a command - look for CSI */
                        c = UART1_Read();
                        if (c == '[')
                        {
                            /* Get parameters, switch on command */
                            c = getescp();
                            switch (c)
                            {
                                case ENQ :
                                    vga_ident[0] = '\0';
                                    break;
                                case 'h' :
                                    /* Turn on newline mode */
                                    if ((escpn == 1) && (escparm[0] == 20))
                                    {
                                        keyload
                                        (
                                            USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER,
                                            "\r\n", "\r\n", "\r\n"
                                        );
                                    }
                                    break;
                                case 'l' :
                                    /* Turn off newline mode */
                                    if ((escpn == 1) && (escparm[0] == 20))
                                    {
                                        keyload
                                        (
                                            USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RETURN_ENTER,
                                            "\r", "\r", "\r"
                                        );
                                    }
                                    break;
                                case 'R' :
                                    /* A ROM read command */
                                    if (escpn == 4)
                                    {
                                        romaddr = \
                                           ((uint32_t)escparm[0] <<  0) + \
                                           ((uint32_t)escparm[1] <<  8) + \
                                           ((uint32_t)escparm[2] << 16) + \
                                           ((uint32_t)escparm[3] << 24);
                                    }
                                    d = *(uint8_t *)romaddr++;
                                    sprintf(dc, "%03d", d);
                                    NRSendStr(dc);
                                    break;
                                case 'U' :
                                    /* Response to VGA ROM read */
                                    if (escpn == 1)
                                    {
                                        d = escparm[0];
                                        sprintf(dc, "%03d", d);
                                        NRSendStr(dc);
                                    }
                                    break;
                                case 'V' :
                                    /* Response to VGA ROM write */
                                    sprintf(dc, "%c%c", '!', 0);
                                    NRSendStr(dc);
                                    break;
                                case 'W' :
                                    /* Firmware update command */
                                    fwupd();
                                    break;
                                default :
                                    break;
                            }
                        }
                        break;
                    default :
                        /* VGA identification string */
                        if (strlen(vga_ident) < (sizeof(vga_ident) - 1)) {
                            sprintf(&vga_ident[strlen(vga_ident)], "%c%c", c, 0);
                        }
                        break;
                }
            }
            break;

        case GET_INPUT_REPORT:
            if(USBHostHIDRead(
                                keyboard.address,
                                keyboard.keys.id,
                                keyboard.keys.normal.details.interfaceNum,
                                keyboard.keys.size,
                                keyboard.keys.buffer
                             )
              )
            {
                /* Host may be busy/error -- keep trying */
            }
            else
            {
                SET_KB_VAR(keyboard.state, INPUT_REPORT_PENDING);
            }
            break;

        case INPUT_REPORT_PENDING:
            if(USBHostHIDReadIsComplete(keyboard.address, &error, &count))
            {
                if(error || (count == 0))
                {
                    SET_KB_VAR(keyboard.state, DEVICE_CONNECTED);
                }
                else
                {
                    SET_KB_VAR(keyboard.state, DEVICE_CONNECTED);

                    App_ProcessInputReport();
                    if(keyboard.leds.updated == true)
                    {
                        SET_KB_VAR(keyboard.state, SEND_OUTPUT_REPORT);
                    }
                }
            }
            break;

        case SEND_OUTPUT_REPORT: /* Will be done while implementing Keyboard */
            if(USBHostHIDWrite(    
                                keyboard.address,
                                keyboard.leds.details.reportID,
                                keyboard.leds.details.interfaceNum,
                                keyboard.leds.details.reportLength,
                                (uint8_t*)&keyboard.leds.report
                               )
              )
            {
                /* Host may be busy/error -- keep trying */
            }
            else
            {
                SET_KB_VAR(keyboard.state, OUTPUT_REPORT_PENDING);
            }
            break;

        case OUTPUT_REPORT_PENDING:
            if(USBHostHIDWriteIsComplete(keyboard.address, &error, &count))
            {
                keyboard.leds.updated = false;
                SET_KB_VAR(keyboard.state, DEVICE_CONNECTED);
            }
            break;

        case ERROR_REPORTED:
            break;

        default:
            break;

    }
}

/****************************************************************************
  Function:
    BOOL USB_HID_DataCollectionHandler(void)
  Description:
    This function is invoked by HID client , purpose is to collect the
    details extracted from the report descriptor. HID client will store
    information extracted from the report descriptor in data structures.
    Application needs to create object for each report type it needs to
    extract.
    For ex: HID_DATA_DETAILS keyboard.keys.modifier.details;
    HID_DATA_DETAILS is defined in file usb_host_hid_appl_interface.h
    Each member of the structure must be initialized inside this function.
    Application interface layer provides functions :
    USBHostHID_ApiFindBit()
    USBHostHID_ApiFindValue()
    These functions can be used to fill in the details as shown in the demo
    code.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    true    - If the report details are collected successfully.
    false   - If the application does not find the the supported format.

  Remarks:
    This Function name should be entered in the USB configuration tool
    in the field "Parsed Data Collection handler".
    If the application does not define this function , then HID cient
    assumes that Application is aware of report format of the attached
    device.
***************************************************************************/
bool APP_HostHIDKeyboardReportParser(void)
{
    uint8_t NumOfReportItem = 0;
    uint8_t i;
    USB_HID_ITEM_LIST* pitemListPtrs;
    USB_HID_DEVICE_RPT_INFO* pDeviceRptinfo;
    HID_REPORTITEM *reportItem;
    HID_USAGEITEM *hidUsageItem;
    uint8_t usageIndex;
    uint8_t reportIndex;
    bool foundLEDIndicator = false;
    bool foundModifierKey = false;
    bool foundNormalKey = false;

    /* The keyboard is already in use. */
    if(keyboard.inUse == true)
    {
        return false;
    }

    pDeviceRptinfo = USBHostHID_GetCurrentReportInfo(); // Get current Report Info pointer
    pitemListPtrs = USBHostHID_GetItemListPointers();   // Get pointer to list of item pointers

    /* Find Report Item Index for Modifier Keys */
    /* Once report Item is located , extract information from data structures provided by the parser */
    NumOfReportItem = pDeviceRptinfo->reportItems;
    for(i=0;i<NumOfReportItem;i++)
    {
        reportItem = &pitemListPtrs->reportItemList[i];
        if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Variable)&&
           (reportItem->globals.usagePage==USB_HID_USAGE_PAGE_KEYBOARD_KEYPAD))
        {
            /* We now know report item points to modifier keys */
            /* Now make sure usage Min & Max are as per application */
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];
            if((hidUsageItem->usageMinimum == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL)
                &&(hidUsageItem->usageMaximum == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_GUI)) //else application cannot suuport
            {
               reportIndex = reportItem->globals.reportIndex;
               keyboard.keys.modifier.details.reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
               keyboard.keys.modifier.details.reportID = (uint8_t)reportItem->globals.reportID;
               keyboard.keys.modifier.details.bitOffset = (uint8_t)reportItem->startBit;
               keyboard.keys.modifier.details.bitLength = (uint8_t)reportItem->globals.reportsize;
               keyboard.keys.modifier.details.count=(uint8_t)reportItem->globals.reportCount;
               keyboard.keys.modifier.details.interfaceNum= USBHostHID_ApiGetCurrentInterfaceNum();
               foundModifierKey = true;
            }

        }
        else if((reportItem->reportType==hidReportInput) && (reportItem->dataModes == HIDData_Array)&&
           (reportItem->globals.usagePage==USB_HID_USAGE_PAGE_KEYBOARD_KEYPAD))
        {
            /* We now know report item points to modifier keys */
            /* Now make sure usage Min & Max are as per application */
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];
           reportIndex = reportItem->globals.reportIndex;
           keyboard.keys.normal.details.reportLength = (pitemListPtrs->reportList[reportIndex].inputBits + 7)/8;
           keyboard.keys.normal.details.reportID = (uint8_t)reportItem->globals.reportID;
           keyboard.keys.normal.details.bitOffset = (uint8_t)reportItem->startBit;
           keyboard.keys.normal.details.bitLength = (uint8_t)reportItem->globals.reportsize;
           keyboard.keys.normal.details.count = (uint8_t)reportItem->globals.reportCount;
           keyboard.keys.normal.details.interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
           foundNormalKey = true;
        }
        else if((reportItem->reportType==hidReportOutput) &&
                (reportItem->globals.usagePage==USB_HID_USAGE_PAGE_LEDS))
        {
            usageIndex = reportItem->firstUsageItem;
            hidUsageItem = &pitemListPtrs->usageItemList[usageIndex];

            reportIndex = reportItem->globals.reportIndex;
            keyboard.leds.details.reportLength = (pitemListPtrs->reportList[reportIndex].outputBits + 7)/8;
            keyboard.leds.details.reportID = (uint8_t)reportItem->globals.reportID;
            keyboard.leds.details.bitOffset = (uint8_t)reportItem->startBit;
            keyboard.leds.details.bitLength = (uint8_t)reportItem->globals.reportsize;
            keyboard.leds.details.count = (uint8_t)reportItem->globals.reportCount;
            keyboard.leds.details.interfaceNum = USBHostHID_ApiGetCurrentInterfaceNum();
            foundLEDIndicator = true;
        }
    }

    if(pDeviceRptinfo->reports == 1)
    {
        keyboard.keys.id = 0;
        keyboard.keys.size = keyboard.keys.normal.details.reportLength;
        keyboard.keys.buffer = (uint8_t*)malloc(keyboard.keys.size);
        keyboard.keys.pollRate = pDeviceRptinfo->reportPollingRate;

        if( (foundNormalKey == true) &&
            (foundModifierKey == true) &&
            (keyboard.keys.buffer != NULL)
        )
        {
            keyboard.inUse = true;
        }
    }

    return(keyboard.inUse);
}

/****************************************************************************
  Function:
    void App_ProcessInputReport(void)

  Description:
    This function processes input report received from HID device.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
static void App_ProcessInputReport(void)
{
    HID_USER_DATA_SIZE key;
    const char *keystr;
    bool capital, ctrl, ctrlable, letter, lock, oldctrl, oldshift, shift;
    int i, j, k;

    /* process input report received from device */
    USBHostHID_ApiImportData(   keyboard.keys.buffer,
                                keyboard.keys.size,
                                keyboard.keys.modifier.data,
                                &keyboard.keys.modifier.details
                            );
    
    USBHostHID_ApiImportData(   keyboard.keys.buffer,
                                keyboard.keys.size,
                                keyboard.keys.normal.data,
                                &keyboard.keys.normal.details
                            );

    /* Shift, then and now ? */
    i = USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_SHIFT - USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL;
    j = USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_SHIFT - USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL;
    oldshift = (keyboard.keys.modifier.oldData[i] == 1) || (keyboard.keys.modifier.oldData[j] == 1);
    shift = (keyboard.keys.modifier.data[i] == 1) || (keyboard.keys.modifier.data[j] == 1);

    /* Control, then and now ? */
    i = USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL - USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL;
    j = USB_HID_KEYBOARD_KEYPAD_KEYBOARD_RIGHT_CONTROL - USB_HID_KEYBOARD_KEYPAD_KEYBOARD_LEFT_CONTROL;
    oldctrl = (keyboard.keys.modifier.oldData[i] == 1) || (keyboard.keys.modifier.oldData[j] == 1);
    ctrl = (keyboard.keys.modifier.data[i] == 1) || (keyboard.keys.modifier.data[j] == 1);

    /* If new modifiers have been added, stop repeating. */
    if ((ctrl && !oldctrl) || (shift && !oldshift))
    {
        SET_KB_VAR(keydown, 0x00);
    }

    /* If repeating, but the repeated key is no longer pressed, stop. */
    if (keydown)
    {
        i = 0;
        while (i < keyboard.keys.normal.details.reportLength)
        {
            key = keyboard.keys.normal.data[i];
            if (key == keydown)
            {
                break;
            }
            ++i;
        }
        if (!(i < keyboard.keys.normal.details.reportLength))
        {
            SET_KB_VAR(keydown, 0x00);
        }
    }
    
    /* Look through the report, send any new key presses found. */
    i = 0;
    while (i < keyboard.keys.normal.details.reportLength)
    {
        /* Skip if we already have this key press */
        key = keyboard.keys.normal.data[i];
        j = 0;
        while (j < keyboard.keys.normal.details.reportLength)
        {
            if (key == keyboard.keys.normal.oldData[j])
            {
                break;
            }
        
            ++j;
        }
        if (j < keyboard.keys.normal.details.reportLength)
        {
            ++i;
            continue;
        }

        /* Caps lock toggle ? */
        if (key == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_CAPS_LOCK)
        {
            keyboard.leds.report.bits.capsLock ^= 1;
            keyboard.leds.updated = true;
                
            if(keyboard.leds.report.bits.capsLock == 1)
            {
                LED_On(LED_USB_HOST_HID_KEYBOARD_CAPS_LOCK);
            }
            else
            {
                LED_Off(LED_USB_HOST_HID_KEYBOARD_CAPS_LOCK);
            }

            /* Stop repeating */
            SET_KB_VAR(keydown, 0x00);

            ++i;
            continue;
        }

        /* Num lock toggle ? */
        if (key == USB_HID_KEYBOARD_KEYPAD_KEYPAD_NUM_LOCK_AND_CLEAR)
        {
            keyboard.leds.report.bits.numLock ^= 1;
            keyboard.leds.updated = true;

            /* Stop repeating */
            SET_KB_VAR(keydown, 0x00);

            ++i;
            continue;
        }

        /* Explorer LCD display */
        if (key == USB_HID_KEYBOARD_KEYPAD_KEYBOARD_ESCAPE)
        {
            PRINT_ClearScreen();
        }

        k = sizeof(keyTranslationTable)/sizeof(HID_KEY_TRANSLATION_TABLE_ENTRY);
        j = 0;
        while (j < k)
        {
            if (key == keyTranslationTable[j].key)
            {
                /* Determine modifier state, find key string */
                keystr = keyTranslationTable[j].shifted;
                ctrlable = (strlen(keystr) == 1) && \
                    !((keystr[0] < 0x40) || (0x5F < keystr[0]));
                if (ctrl && ctrlable)
                {
                    keystr = keyTranslationTable[j].controlled;
                }
                else
                {
                    letter = !((key < USB_HID_KEYBOARD_KEYPAD_KEYBOARD_A) || \
                        (USB_HID_KEYBOARD_KEYPAD_KEYBOARD_Z < key));
                    lock = keyboard.leds.report.bits.capsLock == 1;
                    capital = lock && letter;
                    keystr = (capital ^ shift) ? keyTranslationTable[j].shifted : \
                        keyTranslationTable[j].unmodified;
                }

                /* Send key string, set auto-repeat */
                SET_KB_VAR
                (
                    keydown,
                    (
                        downcnt = 0,
                        rptdelay = FIRST_DELAY,
                        strcpy(downkey, keystr),
                        key
                    )
                );
                NRSendStr(downkey);
                break;
            }

            ++j;
        }

        ++i;
    }

    /* Clear out parsed report data, save a copy of the old data */
    memcpy(keyboard.keys.normal.oldData, keyboard.keys.normal.data, sizeof(keyboard.keys.normal.data));
    memcpy(keyboard.keys.modifier.oldData, keyboard.keys.modifier.data, sizeof(keyboard.keys.modifier.data));
    memset(keyboard.keys.normal.data, 0x00, sizeof(keyboard.keys.normal.data));
    memset(keyboard.keys.modifier.data, 0x00, sizeof(keyboard.keys.modifier.data));
}
