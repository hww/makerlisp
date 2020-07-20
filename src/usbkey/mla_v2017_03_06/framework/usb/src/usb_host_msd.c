// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright 2015 Microchip Technology Inc. (www.microchip.com)

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
//DOM-IGNORE-END


#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "usb_host_msd.h"


// *****************************************************************************
// *****************************************************************************
// Section: Configuration
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Max Number of Supported Devices

This value represents the maximum number of attached devices this class driver
can support.  If the user does not define a value, it will be set to 1.
Currently this must be set to 1, due to limitations in the USB Host layer.
*/
#ifndef USB_MAX_MASS_STORAGE_DEVICES
    #define USB_MAX_MASS_STORAGE_DEVICES        1
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// Section: State Machine Constants
// *****************************************************************************

#ifndef USB_ENABLE_TRANSFER_EVENT

    #define STATE_MASK                          0x00F0      //
    #define SUBSTATE_MASK                       0x000F      //

    #define NEXT_STATE                          0x0010      //
    #define NEXT_SUBSTATE                       0x0001      //


    #define STATE_DETACHED                      0x0000      //

    #define STATE_INITIALIZE_DEVICE             0x0010      //
    #define SUBSTATE_WAIT_FOR_ENUMERATION       0x0000      //
    #define SUBSTATE_SEND_GET_MAX_LUN           0x0001      //
    #define SUBSTATE_WAIT_FOR_MAX_LUN           0x0002      //
    #define SUBSTATE_GET_MAX_LUN_COMPLETE       0x0003      //

    #define STATE_RUNNING                       0x0020      //
    #define SUBSTATE_HOLDING                    0x0000      //
    #define SUBSTATE_SEND_CBW                   0x0001      //
    #define SUBSTATE_CBW_WAIT                   0x0002      //
    #define SUBSTATE_TRANSFER_DATA              0x0003      //
    #define SUBSTATE_TRANSFER_WAIT              0x0004      //
    #define SUBSTATE_REQUEST_CSW                0x0005      //
    #define SUBSTATE_CSW_WAIT                   0x0006      //
    #define SUBSTATE_TRANSFER_DONE              0x0007      //

    #define STATE_MSD_RESET_RECOVERY            0x0030      //
    #define SUBSTATE_SEND_RESET                 0x0000      //
    #define SUBSTATE_WAIT_FOR_RESET             0x0001      //
    #define SUBSTATE_RESET_COMPLETE             0x0002      //

    #define STATE_MSD_CLEAR_DATA_IN             0x0040      //
    #define SUBSTATE_SEND_CLEAR_IN              0x0000      //
    #define SUBSTATE_WAIT_FOR_CLEAR_IN          0x0001      //
    #define SUBSTATE_CLEAR_IN_COMPLETE          0x0002      //

    #define STATE_MSD_CLEAR_DATA_OUT            0x0050      //
    #define SUBSTATE_SEND_CLEAR_OUT             0x0000      //
    #define SUBSTATE_WAIT_FOR_CLEAR_OUT         0x0001      //
    #define SUBSTATE_CLEAR_OUT_COMPLETE         0x0002      //

    #define STATE_HOLDING                       0x0060      // Holding due to an error

#else

    #define STATE_DETACHED                      0x0000      //

    #define STATE_WAIT_FOR_MAX_LUN              0x0001      //

    #define STATE_RUNNING                       0x0002      //
    #define STATE_CBW_WAIT                      0x0003      //
    #define STATE_TRANSFER_WAIT                 0x0004      //
    #define STATE_CSW_WAIT                      0x0005      //

    #define STATE_WAIT_FOR_RESET                0x0006      //
    #define STATE_WAIT_FOR_CLEAR_IN             0x0007      //
    #define STATE_WAIT_FOR_CLEAR_OUT            0x0008      //

    #define STATE_HOLDING                       0x0009      //  Holding due to an error

    #define STATE_REQUEST_CSW                   0x000A      //  Dummy state

#endif


// *****************************************************************************
// Section: Other Constants
// *****************************************************************************

#define CBW_SIZE                            31              // Number of bytes in the CBW.
#define CSW_SIZE                            13              // Number of bytes in the CSW.

#define CSW_RECEIVE_ATTEMPTS                100             // Number of attempts to obtain the CSW.

#define USB_MSD_GET_MAX_LUN                 0xFE            // Device Request code to get the maximum LUN.
#define USB_MSD_RESET                       0xFF            // Device Request code to reset the device.
#define USB_MSD_DCBWSIGNATURE               0x43425355ul    // Signature value for the CBW.
#define USB_MSD_DCSWSIGNATURE               0x53425355ul    // Signature value for the CSW.

#define MARK_RESET_RECOVERY                 0x0E            // Maintain with USB_MSD_DEVICE_INFO


//******************************************************************************
//******************************************************************************
// Section: Data Structures
//******************************************************************************
//******************************************************************************

// *****************************************************************************
// *****************************************************************************
/* Command Block Wrapper

This is the structure of the Command Block Wrapper, required at the beginning of
each mass storage transfer.
*/
typedef struct __attribute__((packed)) _USB_MSD_CBW
{
    uint32_t       dCBWSignature;              // Signature, must be a specific constant.
    uint32_t       dCBWTag;                    // Transaction tag. Value in the CSW must match.
    uint32_t       dCBWDataTransferLength;     // Length of the following data transfer.
    union
    {
        struct
        {
            uint8_t                : 7;
            uint8_t    bfDirection : 1;        // Direction of data transfer (0=OUT, 1=IN).
        };
        uint8_t        val;
    }           bmCBWflags;
    uint8_t        bCBWLUN;                    // Logical Unit Number (bits 3:0 only).
    uint8_t        bCBWCBLength;               // Length of command block (bits 4:0 only).
    uint8_t        CBWCB[16];                  // Command block.
} USB_MSD_CBW;

// *****************************************************************************
/* Command Status Wrapper

This is the structure of the Command Status Wrapper, required at the end of
each mass storage transfer.
*/
typedef struct __attribute__((packed)) _USB_MSD_CSW
{
    uint32_t   dCSWSignature;                  // Signature, must be a specific constant.
    uint32_t   dCSWTag;                        // Transaction tag. Must match the CBW.
    uint32_t   dCSWDataResidue;                // Count of data bytes not transferred.
    uint8_t    dCSWStatus;                     // Result of requested operation.
} USB_MSD_CSW;


/* USB Mass Storage Device Information
This structure is used to hold all the information about an attached Mass Storage device.
*/
typedef struct _USB_MSD_DEVICE_INFO
{
    union __attribute__((packed)) {
        uint8_t                            data[31];          // Data buffer for device communication.
        USB_MSD_CBW                     cbw;
        USB_MSD_CSW                     csw;
    }block;

    uint8_t                                deviceAddress;          // Address of the device on the bus.
    uint8_t                                clientDriverID;         // Client driver ID for device requests.
    uint8_t                                errorCode;              // Error code of last error.
    uint8_t                                state;                  // State machine state of the device.
    uint8_t                                returnState;            // State to return to after performing error handling.
    union
    {
        struct
        {
            uint8_t                        bfDirection     : 1;    // Direction of current transfer (0=OUT, 1=IN).
            uint8_t                        bfReset         : 1;    // Flag indicating to perform Mass Storage Reset.
            uint8_t                        bfClearDataIN   : 1;    // Flag indicating to clear the IN endpoint.
            uint8_t                        bfClearDataOUT  : 1;    // Flag indicating to clear the OUT endpoint.
        };
        uint8_t                            val;
    }                                   flags;
    uint8_t                                maxLUN;                 // The maximum Logical Unit Number of the device.
    uint8_t                                interface;              // Interface number we are using.
    uint8_t                                endpointIN;             // Bulk IN endpoint.
    uint8_t                                endpointOUT;            // Bulk OUT endpoint.
    uint8_t                                endpointDATA;           // Endpoint to use for the current transfer.
    uint8_t                                *userData;              // Pointer to the user's data buffer.
    uint32_t                               userDataLength;         // Length of the user's data buffer.
    uint32_t                               bytesTransferred;       // Number of bytes transferred to/from the user's data buffer.
    uint32_t                               dCBWTag;                // The value of the dCBWTag to verify against the dCSWtag.
    uint8_t                                attemptsCSW;            // Number of attempts to retrieve the CSW.
} USB_MSD_DEVICE_INFO;

//******************************************************************************
//******************************************************************************
// Section: Local Prototypes
//******************************************************************************
//******************************************************************************

uint32_t   _USBHostMSD_GetNextTag( void );
void    _USBHostMSD_ResetStateJump( uint8_t i );


//******************************************************************************
//******************************************************************************
// Section: Macros
//******************************************************************************
//******************************************************************************

#ifndef USB_ENABLE_TRANSFER_EVENT
  #define _USBHostMSD_SetNextState()                { deviceInfoMSD[i].state = (deviceInfoMSD[i].state & STATE_MASK) + NEXT_STATE; }
  #define _USBHostMSD_SetNextSubState()             { deviceInfoMSD[i].state += NEXT_SUBSTATE; }
  #define _USBHostMSD_TerminateTransfer( error )    {                                                                           \
                                                        deviceInfoMSD[i].errorCode  = error;                                    \
                                                        deviceInfoMSD[i].state      = STATE_RUNNING | SUBSTATE_TRANSFER_DONE;   \
                                                    }
#else
  #ifdef USB_MSD_ENABLE_TRANSFER_EVENT
    #define _USBHostMSD_TerminateTransfer( error )  {                                                                                                           \
                                                        deviceInfoMSD[i].errorCode  = error;                                                                    \
                                                        deviceInfoMSD[i].state      = STATE_RUNNING;                                         \
                                                        usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_TRANSFER, NULL, 0 );     \
                                                    }
  #else
    #define _USBHostMSD_TerminateTransfer( error )  {                                                                                                           \
                                                        deviceInfoMSD[i].errorCode  = error;                                                                    \
                                                        deviceInfoMSD[i].state      = STATE_RUNNING;                                         \
                                                    }
  #endif
#endif


//******************************************************************************
//******************************************************************************
// Section: MSD Host External Variables
//******************************************************************************
//******************************************************************************

extern CLIENT_DRIVER_TABLE usbMediaInterfaceTable;  // This table contains the initialization
                                                    // routine and event handler for the media
                                                    // interface layer of the application.
                                                    // It is defined in USBConfig.c.

//******************************************************************************
//******************************************************************************
// Section: MSD Host Global Variables
//******************************************************************************
//******************************************************************************

static uint32_t                       dCBWTagNext     = 0x12345678ul;
static USB_MSD_DEVICE_INFO         deviceInfoMSD[USB_MAX_MASS_STORAGE_DEVICES] __attribute__ ((aligned));


// *****************************************************************************
// *****************************************************************************
// Section: Application Callable Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    uint8_t USBHostMSDDeviceStatus( uint8_t deviceAddress )

  Description:
    This function determines the status of a mass storage device.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress - address of device to query

  Return Values:
    USB_MSD_DEVICE_NOT_FOUND -  Illegal device address, or the device is not
                                an MSD
    USB_MSD_INITIALIZING     -  MSD is attached and in the process of
                                initializing
    USB_MSD_NORMAL_RUNNING   -  MSD is in normal running mode
    USB_MSD_RESETTING_DEVICE -  MSD is resetting
    USB_MSD_DEVICE_DETACHED  -  MSD detached.  Should not occur
    USB_MSD_ERROR_STATE      -  MSD is holding due to an error.  No
                                communication is allowed.

    Other                    -  Return codes from USBHostDeviceStatus() will
                                also be returned if the device is in the
                                process of enumerating.

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostMSDDeviceStatus( uint8_t deviceAddress )
{
    uint8_t    i;
    uint8_t    status;

    // Make sure a valid device is being requested.
    if ((deviceAddress == 0) || (deviceAddress > 127))
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    // Find the correct device.
    for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_MASS_STORAGE_DEVICES)
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    status = USBHostDeviceStatus( deviceAddress );
    if (status != USB_DEVICE_ATTACHED)
    {
        return status;
    }
    else
    {
        // The device is attached and done enumerating.  We can get more specific now.

        #ifndef USB_ENABLE_TRANSFER_EVENT
            switch (deviceInfoMSD[i].state & STATE_MASK)
            {
                case STATE_INITIALIZE_DEVICE:
                    return USB_MSD_INITIALIZING;
                    break;

                case STATE_RUNNING:
                    return USB_MSD_NORMAL_RUNNING;
                    break;

                case STATE_HOLDING:
                    return USB_MSD_ERROR_STATE;
                    break;

                case STATE_MSD_RESET_RECOVERY:
                case STATE_MSD_CLEAR_DATA_IN:
                case STATE_MSD_CLEAR_DATA_OUT:
                    return USB_MSD_RESETTING_DEVICE;
                    break;

                default:
                    return USB_MSD_DEVICE_DETACHED;
                    break;
            }
        #else
            switch (deviceInfoMSD[i].state)
            {
                case STATE_WAIT_FOR_MAX_LUN:
                    return USB_MSD_INITIALIZING;
                    break;

                case STATE_RUNNING:
                case STATE_CBW_WAIT:
                case STATE_TRANSFER_WAIT:
                case STATE_CSW_WAIT:
                case STATE_REQUEST_CSW:
                    return USB_MSD_NORMAL_RUNNING;
                    break;

                case STATE_HOLDING:
                    return USB_MSD_ERROR_STATE;
                    break;

                case STATE_WAIT_FOR_RESET:
                case STATE_WAIT_FOR_CLEAR_IN:
                case STATE_WAIT_FOR_CLEAR_OUT:
                    return USB_MSD_RESETTING_DEVICE;
                    break;

                default:
                    return USB_MSD_DEVICE_DETACHED;
                    break;
            }
        #endif
    }
}


/****************************************************************************
  Function:
    uint8_t USBHostMSDResetDevice( uint8_t deviceAddress )

  Summary:
    This function starts a bulk-only mass storage reset.

  Description:
    This function starts a bulk-only mass storage reset.  A reset can be
    issued only if the device is attached and not being initialized.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress - Device address

  Return Values:
    USB_SUCCESS                 - Reset started
    USB_MSD_DEVICE_NOT_FOUND    - No device with specified address
    USB_MSD_ILLEGAL_REQUEST     - Device is in an illegal state for reset

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostMSDResetDevice( uint8_t deviceAddress )
{
    uint8_t    i;

    // Make sure a valid device is being requested.
    if ((deviceAddress == 0) || (deviceAddress > 127))
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    // Find the correct device.
    for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_MASS_STORAGE_DEVICES)
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    #ifndef USB_ENABLE_TRANSFER_EVENT
        if (((deviceInfoMSD[i].state & STATE_MASK) != STATE_DETACHED) &&
            ((deviceInfoMSD[i].state & STATE_MASK) != STATE_INITIALIZE_DEVICE))
    #else
        if ((deviceInfoMSD[i].state == STATE_RUNNING) ||
            (deviceInfoMSD[i].state == STATE_HOLDING ))
    #endif
    {
        deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].returnState = STATE_RUNNING | SUBSTATE_HOLDING;
        #else
            deviceInfoMSD[i].returnState = STATE_RUNNING;
        #endif
        _USBHostMSD_ResetStateJump( i );
        return USB_SUCCESS;
    }
    return USB_MSD_ILLEGAL_REQUEST;
}


/****************************************************************************
  Function:
    void USBHostMSDTasks( void )

  Summary:
    This function performs the maintenance tasks required by the mass storage
    class.

  Description:
    This function performs the maintenance tasks required by the mass storage
    class.  If transfer events from the host layer are not being used, then
    it should be called on a regular basis by the application.  If transfer
    events from the host layer are being used, this function is compiled out,
    and does not need to be called.

  Precondition:
    USBHostMSDInitialize() has been called.

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void USBHostMSDTasks( void )
{

#ifndef USB_ENABLE_TRANSFER_EVENT

    uint32_t   byteCount;
    uint8_t    errorCode;
    uint8_t    i;

    for (i=0; i<USB_MAX_MASS_STORAGE_DEVICES; i++)
    {
        if (deviceInfoMSD[i].deviceAddress != 0)
        {
            switch (deviceInfoMSD[i].state & STATE_MASK)
            {
                case STATE_DETACHED:
                    // No device attached.
                    break;

                case STATE_INITIALIZE_DEVICE:
                    switch (deviceInfoMSD[i].state & SUBSTATE_MASK)
                    {
                        case SUBSTATE_WAIT_FOR_ENUMERATION:
                            if (USBHostDeviceStatus( deviceInfoMSD[i].deviceAddress ) == USB_DEVICE_ATTACHED)
                            {
                                _USBHostMSD_SetNextSubState();
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Getting max LUN...\r\n" );
                                #endif
                            }
                            break;

                        case SUBSTATE_SEND_GET_MAX_LUN:
                            // If we are currently sending a token, we cannot do anything.
                            if (U1CONbits.TOKBUSY)
                                break;

                            if (!USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                                    USB_MSD_GET_MAX_LUN, 0, deviceInfoMSD[i].interface, 1, deviceInfoMSD[i].block.data, USB_DEVICE_REQUEST_GET, deviceInfoMSD[i].clientDriverID ))
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            else
                            {
                                // We'll try again.
                            }
                            break;

                        case SUBSTATE_WAIT_FOR_MAX_LUN:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, 0, &errorCode, &byteCount ))
                            {
                                deviceInfoMSD[i].maxLUN = 0;
                                if (!errorCode)
                                {
                                    deviceInfoMSD[i].maxLUN = deviceInfoMSD[i].block.data[0];
                                }
                                else
                                {
                                    // Clear the STALL.  Since it is EP0, we do not have to clear the stall.
                                    USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, 0 );
                                }
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_GET_MAX_LUN_COMPLETE:
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Max LUN is " );
                                UART2PutHex( deviceInfoMSD[i].maxLUN );
                                UART2PrintString( "\r\nMSD: Running...\r\n" );
                            #endif

                            // Tell the media interface layer that we have a MSD attached.
                            if (usbMediaInterfaceTable.Initialize( deviceInfoMSD[i].deviceAddress, usbMediaInterfaceTable.flags, 0 ))
                            {
                                usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_MAX_LUN, &(deviceInfoMSD[i].maxLUN), 1 );
                                _USBHostMSD_SetNextState();
                            }
                            else
                            {
                                // The media interface layer cannot support the device.
                                deviceInfoMSD[i].errorCode = USB_MSD_MEDIA_INTERFACE_ERROR;
                                deviceInfoMSD[i].state     = STATE_HOLDING;
                            }
                            break;
                    }
                    break;

                case STATE_RUNNING:
                    switch (deviceInfoMSD[i].state & SUBSTATE_MASK)
                    {
                        case SUBSTATE_HOLDING:
                            break;

                        case SUBSTATE_SEND_CBW:
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Writing CBW\r\n" );
                            #endif
                            errorCode = USBHostWrite( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointOUT, deviceInfoMSD[i].block.data, CBW_SIZE );
                            if (errorCode)
                            {
                                _USBHostMSD_TerminateTransfer( errorCode );
                            }
                            else
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_CBW_WAIT:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointOUT, &errorCode, &byteCount ))
                            {
                                if (errorCode)
                                {
                                    #ifdef DEBUG_MODE
                                        UART2PrintString( "MSD: Error with sending CBW\r\n" );
                                    #endif
                                    _USBHostMSD_TerminateTransfer( errorCode );
                                }
                                else if (byteCount != CBW_SIZE)
                                {
                                    #ifdef DEBUG_MODE
                                        UART2PrintString( "MSD: CBW size not correct\r\n" );
                                    #endif
                                    _USBHostMSD_TerminateTransfer( USB_MSD_CBW_ERROR );
                                }
                                else
                                {
                                    if (deviceInfoMSD[i].block.cbw.dCBWDataTransferLength == 0)
                                    {
                                        #ifdef DEBUG_MODE
                                            UART2PrintString( "MSD: Transfer length=0\r\n" );
                                        #endif
                                        // Skip to get the CSW
                                        deviceInfoMSD[i].state = STATE_RUNNING | SUBSTATE_REQUEST_CSW;
                                    }
                                    else
                                    {
                                        #ifdef DEBUG_MODE
                                            UART2PrintString( "MSD: Going on...\r\n" );
                                        #endif
                                        _USBHostMSD_SetNextSubState();
                                    }
                                }
                            }
                            break;

                        case SUBSTATE_TRANSFER_DATA:
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Transferring data, length ");
                                UART2PutHexDWord( deviceInfoMSD[i].userDataLength );
                                UART2PrintString( "\r\n" );
                            #endif
                            if (deviceInfoMSD[i].userDataLength == 0)
                            {
                                deviceInfoMSD[i].state = STATE_RUNNING | SUBSTATE_REQUEST_CSW;
                            }
                            else
                            {
                                if (!deviceInfoMSD[i].flags.bfDirection) // OUT
                                {
                                    errorCode = USBHostWrite( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA, deviceInfoMSD[i].userData, deviceInfoMSD[i].userDataLength );
                                }
                                else
                                {
                                    errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA, deviceInfoMSD[i].userData, deviceInfoMSD[i].userDataLength );
                                }

                                if (errorCode)
                                {
                                    _USBHostMSD_TerminateTransfer( errorCode );
                                }
                                else
                                {
                                    _USBHostMSD_SetNextSubState();
                                }
                            }
                            break;

                        case SUBSTATE_TRANSFER_WAIT:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA, &errorCode, &byteCount ))
                            {
                                if (errorCode)
                                {
                                    if (errorCode == USB_ENDPOINT_STALLED)
                                    {
                                        // Clear the stall, then try to get the CSW.
                                        #ifdef DEBUG_MODE
                                            UART2PrintString( "MSD: Stall on data\r\n" );
                                        #endif
                                        USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA );
                                        if (!deviceInfoMSD[i].flags.bfDirection) // OUT
                                        {
                                            deviceInfoMSD[i].flags.bfClearDataOUT = 1;
                                        }
                                        else
                                        {
                                            deviceInfoMSD[i].flags.bfClearDataIN = 1;
                                        }
                                        deviceInfoMSD[i].returnState = STATE_RUNNING | SUBSTATE_REQUEST_CSW;
                                        _USBHostMSD_ResetStateJump( i );

                                    }
                                    else
                                    {
                                        //Error recovery here is not explicitly covered in the spec. Unfortunately, some
                                        // thumb drives generate a turn-around time error here sometimes.
                                        //_USBHostMSD_TerminateTransfer( errorCode );
                                        deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
                                        deviceInfoMSD[i].returnState = STATE_RUNNING | SUBSTATE_SEND_CBW;   // Try the transfer again.
                                        _USBHostMSD_ResetStateJump( i );
                                    }
                                }
                                else
                                {
                                    _USBHostMSD_SetNextSubState();
                                }
                            }
                            break;

                        case SUBSTATE_REQUEST_CSW:
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Getting CSW\r\n" );
                            #endif

                            errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN, deviceInfoMSD[i].block.data, CSW_SIZE );
                            if (errorCode)
                            {
                                _USBHostMSD_TerminateTransfer( errorCode );
                            }
                            else
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_CSW_WAIT:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN, &errorCode, &byteCount ))
                            {
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Got CSW-" );
                                #endif
                                if (errorCode)
                                {
                                    deviceInfoMSD[i].attemptsCSW--;
                                    if (deviceInfoMSD[i].attemptsCSW)
                                    {
                                        USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN );
                                        deviceInfoMSD[i].flags.bfClearDataIN = 1;
                                        deviceInfoMSD[i].returnState = STATE_RUNNING | SUBSTATE_REQUEST_CSW;
                                        _USBHostMSD_ResetStateJump( i );
                                    }
                                    else
                                    {
                                        _USBHostMSD_TerminateTransfer( errorCode );
                                    }
                                }
                                else if ((byteCount != CSW_SIZE) |
                                         (deviceInfoMSD[i].block.csw.dCSWSignature != USB_MSD_DCSWSIGNATURE) |
                                         (deviceInfoMSD[i].block.csw.dCSWTag       != deviceInfoMSD[i].dCBWTag) )
                                {
                                    _USBHostMSD_TerminateTransfer( USB_MSD_CSW_ERROR );
                                }
                                else
                                {
                                    deviceInfoMSD[i].bytesTransferred = deviceInfoMSD[i].userDataLength - deviceInfoMSD[i].block.csw.dCSWDataResidue;

                                    if (deviceInfoMSD[i].block.csw.dCSWStatus != 0x00)
                                    {
                                        _USBHostMSD_TerminateTransfer( deviceInfoMSD[i].block.csw.dCSWStatus | USB_MSD_ERROR );
                                    }
                                    else
                                    {
                                        _USBHostMSD_TerminateTransfer( USB_SUCCESS );
                                    }

                                    // If we have a phase error, we need to perform corrective action instead of
                                    // returning to normal running.
                                    if (deviceInfoMSD[i].block.csw.dCSWStatus == MSD_PHASE_ERROR)
                                    {
                                        deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
                                        deviceInfoMSD[i].returnState = STATE_RUNNING | SUBSTATE_HOLDING;
                                        _USBHostMSD_ResetStateJump( i );
                                    }
                                }
                            }
                            break;

                        case SUBSTATE_TRANSFER_DONE:
                            deviceInfoMSD[i].state = STATE_RUNNING | SUBSTATE_HOLDING;
                            #ifdef USB_MSD_ENABLE_TRANSFER_EVENT
                                usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_TRANSFER, NULL, 0 );
                            #endif
                            break;
                    }
                    break;

                case STATE_MSD_RESET_RECOVERY:
                    switch (deviceInfoMSD[i].state & SUBSTATE_MASK)
                    {
                        case SUBSTATE_SEND_RESET:
                            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                                            USB_MSD_RESET, 0, deviceInfoMSD[i].interface, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

                            if (errorCode)
                            {
                                //Error recovery here is not explicitly covered in the spec.
                                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                            }
                            else
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_WAIT_FOR_RESET:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, 0, &errorCode, &byteCount ))
                            {
                                if (errorCode)
                                {
                                    //Error recovery here is not explicitly covered in the spec.
                                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                                }
                                else
                                {
                                    deviceInfoMSD[i].flags.bfReset = 0;
                                    _USBHostMSD_SetNextSubState();
                                }
                            }
                            break;

                        case SUBSTATE_RESET_COMPLETE:
                            _USBHostMSD_ResetStateJump( i );
                            break;
                    }
                    break;

                case STATE_MSD_CLEAR_DATA_IN:
                    switch (deviceInfoMSD[i].state & SUBSTATE_MASK)
                    {
                        case SUBSTATE_SEND_CLEAR_IN:
                            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_ENDPOINT,
                                            USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, deviceInfoMSD[i].endpointIN, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

                            if (errorCode)
                            {
                                //Error recovery here is not explicitly covered in the spec.
                                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                            }
                            else
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_WAIT_FOR_CLEAR_IN:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, 0, &errorCode, &byteCount ))
                            {
                                if (errorCode)
                                {
                                    //Error recovery here is not explicitly covered in the spec.
                                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                                }
                                else
                                {
                                    deviceInfoMSD[i].flags.bfClearDataIN = 0;
                                    _USBHostMSD_SetNextSubState();
                                }
                            }
                            break;

                        case SUBSTATE_CLEAR_IN_COMPLETE:
                            _USBHostMSD_ResetStateJump( i );
                            break;
                    }
                    break;

                case STATE_MSD_CLEAR_DATA_OUT:
                    switch (deviceInfoMSD[i].state & SUBSTATE_MASK)
                    {
                        case SUBSTATE_SEND_CLEAR_OUT:
                            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_ENDPOINT,
                                            USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, deviceInfoMSD[i].endpointOUT, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

                            if (errorCode)
                            {
                                //Error recovery here is not explicitly covered in the spec.
                                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                            }
                            else
                            {
                                _USBHostMSD_SetNextSubState();
                            }
                            break;

                        case SUBSTATE_WAIT_FOR_CLEAR_OUT:
                            if (USBHostTransferIsComplete( deviceInfoMSD[i].deviceAddress, 0, &errorCode, &byteCount ))
                            {
                                if (errorCode)
                                {
                                    //Error recovery here is not explicitly covered in the spec.
                                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                                }
                                else
                                {
                                    deviceInfoMSD[i].flags.bfClearDataOUT = 0;
                                    _USBHostMSD_SetNextSubState();
                                }
                            }
                            break;

                        case SUBSTATE_CLEAR_OUT_COMPLETE:
                            _USBHostMSD_ResetStateJump( i );
                            break;
                    }
                    break;

                case STATE_HOLDING:
                    break;
            }
        }
    }
#endif
}


/****************************************************************************
  Function:
    void USBHostMSDTerminateTransfer( uint8_t deviceAddress )

  Description:
    This function terminates a mass storage transfer.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address

  Returns:
    None

  Remarks:
    After executing this function, the application may have to reset the
    device in order for the device to continue working properly.
  ***************************************************************************/

void USBHostMSDTerminateTransfer( uint8_t deviceAddress )
{
    uint8_t    i;

    // Find the correct device.
    for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_MASS_STORAGE_DEVICES)
    {
        return;
    }

    // We can only terminate transfers if we are in STATE_RUNNING.  Otherwise, we
    // will cause major problems.

    #ifndef USB_ENABLE_TRANSFER_EVENT
        if ((deviceInfoMSD[i].state & STATE_MASK) == STATE_RUNNING)
    #else
        if ((deviceInfoMSD[i].state == STATE_RUNNING) ||
            (deviceInfoMSD[i].state == STATE_CBW_WAIT) ||
            (deviceInfoMSD[i].state == STATE_TRANSFER_WAIT) ||
            (deviceInfoMSD[i].state == STATE_CSW_WAIT))
    #endif
    {
        // Terminate any endpoint tranfers that are occurring.
        USBHostTerminateTransfer( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN );
        USBHostTerminateTransfer( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointOUT );

        // Set the state back to running and waiting for a transfer request.
        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].state = STATE_RUNNING | SUBSTATE_HOLDING;
        #else
            deviceInfoMSD[i].state = STATE_RUNNING;
        #endif
    }
    return;
}

/****************************************************************************
  Function:
    uint8_t USBHostMSDTransfer( uint8_t deviceAddress, uint8_t deviceLUN,
                uint8_t direction, uint8_t *commandBlock, uint8_t commandBlockLength,
                uint8_t *data, uint32_t dataLength )

  Summary:
    This function starts a mass storage transfer.

  Description:
    This function starts a mass storage transfer.  Usually, applications will
    probably utilize a read/write wrapper to access this function.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress      - Device address
    uint8_t deviceLUN          - Device LUN to access
    uint8_t direction          - 1=read, 0=write
    uint8_t *commandBlock      - Pointer to the command block for the CBW
    uint8_t commandBlockLength - Length of the command block
    uint8_t *data              - Pointer to the data buffer
    uint32_t dataLength        - Byte size of the data buffer


  Return Values:
    USB_SUCCESS                 - Request started successfully
    USB_MSD_DEVICE_NOT_FOUND    - No device with specified address
    USB_MSD_DEVICE_BUSY         - Device not in proper state for performing
                                    a transfer
    USB_MSD_INVALID_LUN         - Specified LUN does not exist

  Remarks:
    None
  ***************************************************************************/

uint8_t USBHostMSDTransfer( uint8_t deviceAddress, uint8_t deviceLUN, uint8_t direction, uint8_t *commandBlock,
                        uint8_t commandBlockLength, uint8_t *data, uint32_t dataLength )
{
    uint8_t    i;
    uint8_t    j;

    #ifdef DEBUG_MODE
        UART2PrintString( "MSD: Transfer: " );
        if (direction)
        {
            UART2PrintString( "Read, " );
        }
        else
        {
            UART2PrintString( "Write, " );
        }
    #endif

    // Make sure a valid device is being requested.
    if ((deviceAddress == 0) || (deviceAddress > 127))
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    // Find the correct device.
    for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != deviceAddress); i++);
    if (i == USB_MAX_MASS_STORAGE_DEVICES)
    {
        return USB_MSD_DEVICE_NOT_FOUND;
    }

    // Make sure the device is in a state ready to read/write.
    #ifndef USB_ENABLE_TRANSFER_EVENT
        if (deviceInfoMSD[i].state != (STATE_RUNNING | SUBSTATE_HOLDING))
    #else
        if (deviceInfoMSD[i].state != STATE_RUNNING)
    #endif
    {
        return USB_MSD_DEVICE_BUSY;
    }

    // Verify the selected LUN.
    if (deviceLUN > deviceInfoMSD[i].maxLUN)
    {
        return USB_MSD_INVALID_LUN;
    }

    // Initialize the transfer information.
    deviceInfoMSD[i].attemptsCSW       = CSW_RECEIVE_ATTEMPTS;
    deviceInfoMSD[i].bytesTransferred  = 0;
    deviceInfoMSD[i].errorCode         = USB_SUCCESS;
    deviceInfoMSD[i].flags.val         = 0;
    deviceInfoMSD[i].flags.bfDirection = direction;
    deviceInfoMSD[i].userData          = data;
    deviceInfoMSD[i].userDataLength    = dataLength;
    deviceInfoMSD[i].dCBWTag           = _USBHostMSD_GetNextTag();
    deviceInfoMSD[i].endpointDATA      = deviceInfoMSD[i].endpointIN;
    if (!direction) // OUT
    {
        deviceInfoMSD[i].endpointDATA  = deviceInfoMSD[i].endpointOUT;
    }
    #ifdef DEBUG_MODE
        UART2PrintString( "Data EP: " );
        UART2PutHex( deviceInfoMSD[i].endpointDATA );
        UART2PrintString( "\r\n" );
    #endif

    // Prepare the CBW so we can give the user back his command block RAM.
    deviceInfoMSD[i].block.cbw.dCBWSignature             = USB_MSD_DCBWSIGNATURE;
    deviceInfoMSD[i].block.cbw.dCBWTag                   = deviceInfoMSD[i].dCBWTag;
    deviceInfoMSD[i].block.cbw.dCBWDataTransferLength    = deviceInfoMSD[i].userDataLength;
    deviceInfoMSD[i].block.cbw.bmCBWflags.val            = 0;
    deviceInfoMSD[i].block.cbw.bmCBWflags.bfDirection    = direction;
    deviceInfoMSD[i].block.cbw.bCBWLUN                   = deviceLUN;
    deviceInfoMSD[i].block.cbw.bCBWCBLength              = commandBlockLength;
    for (j=0; j<commandBlockLength; j++)
    {
        deviceInfoMSD[i].block.cbw.CBWCB[j]              = commandBlock[j];
    }

    #ifndef USB_ENABLE_TRANSFER_EVENT
        // Jump to the transfer state.
        deviceInfoMSD[i].state             = STATE_RUNNING | SUBSTATE_SEND_CBW;
    #else
        j = USBHostWrite( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointOUT, deviceInfoMSD[i].block.data, CBW_SIZE );
        if (j)
        {
            _USBHostMSD_TerminateTransfer( j );
        }
        else
        {
            deviceInfoMSD[i].state = STATE_CBW_WAIT;
        }
    #endif

    return USB_SUCCESS;
}


/****************************************************************************
  Function:
    bool USBHostMSDTransferIsComplete( uint8_t deviceAddress,
                        uint8_t *errorCode, uint32_t *byteCount )

  Summary:
    This function indicates whether or not the last transfer is complete.

  Description:
    This function indicates whether or not the last transfer is complete.  If
    the functions returns true, the returned byte count and error code are
    valid. Since only one transfer can be performed at once and only one
    endpoint can be used, we only need to know the device address.

  Precondition:
    None

  Parameters:
    uint8_t deviceAddress  - Device address
    uint8_t *errorCode     - Error code from last transfer
    uint32_t *byteCount    - Number of bytes transferred

  Return Values:
    true    - Transfer is complete, errorCode is valid
    false   - Transfer is not complete, errorCode is not valid

  Remarks:
    None
  ***************************************************************************/

bool USBHostMSDTransferIsComplete( uint8_t deviceAddress, uint8_t *errorCode, uint32_t *byteCount )
{
    uint8_t    i;

     // Make sure a valid device is being requested.
    if ((deviceAddress == 0) || (deviceAddress > 127))
    {
        *errorCode = USB_MSD_DEVICE_NOT_FOUND;
        *byteCount = 0;
        return true;
    }

    // Find the correct device.
    for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != deviceAddress); i++);
    if ((i == USB_MAX_MASS_STORAGE_DEVICES) || (deviceInfoMSD[i].state == STATE_DETACHED))
    {
        *errorCode = USB_MSD_DEVICE_NOT_FOUND;
        *byteCount = 0;
        return true;
    }

    #ifndef USB_ENABLE_TRANSFER_EVENT
        if  ( (deviceInfoMSD[i].state               == (STATE_RUNNING | SUBSTATE_HOLDING)) ||
             ((deviceInfoMSD[i].state & STATE_MASK) == STATE_HOLDING))
    #else
        if  ((deviceInfoMSD[i].state == STATE_RUNNING) ||
             (deviceInfoMSD[i].state == STATE_HOLDING))
    #endif
    {
        *byteCount = deviceInfoMSD[i].bytesTransferred;
        *errorCode = deviceInfoMSD[i].errorCode;

        return true;
    }
    else
    {
        return false;
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Host Stack Interface Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    bool USBHostMSDInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )

  Summary:
    This function is the initialization routine for this client driver.

  Description:
    This function is the initialization routine for this client driver.  It
    is called by the host layer when the USB device is being enumerated.  For
    a mass storage device, we need to make sure that we have room for a new
    device, and that the device has at least one bulk IN and one bulk OUT
    endpoint.

  Precondition:
    None

  Parameters:
    uint8_t address        - Address of the new device
    uint32_t flags         - Initialization flags
    uint8_t clientDriverID - ID to send when issuing a Device Request via
                            USBHostIssueDeviceRequest(), USBHostSetDeviceConfiguration(),
                            or USBHostSetDeviceInterface().

  Return Values:
    true   - We can support the device.
    false  - We cannot support the device.

  Remarks:
    None
  ***************************************************************************/

bool USBHostMSDInitialize( uint8_t address, uint32_t flags, uint8_t clientDriverID )
{
    uint8_t   *descriptor;
    uint8_t    device;
    #ifdef USB_ENABLE_TRANSFER_EVENT
        uint8_t    errorCode;
    #endif
    uint16_t    i;
    uint8_t    endpointIN;
    uint8_t    endpointOUT;

    #ifdef DEBUG_MODE
        UART2PrintString( "MSD: USBHostMSDClientInitialize(0x" );
        UART2PutHex( flags );
        UART2PrintString( ")\r\n" );
    #endif

    // Find the free slot in the table.  If we cannot find one, kick off the device.
    for (device = 0; (device < USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[device].deviceAddress != 0); device++);
    if (device == USB_MAX_MASS_STORAGE_DEVICES)
    {
        #ifdef DEBUG_MODE
            UART2PrintString( "MSD: No free slots available for MSD.\r\n" );
        #endif
        // Kick off the device
        return false;
    }

    #ifdef DEBUG_MODE
        UART2PrintString( "MSD: MSD attached.\r\n" );
    #endif

    descriptor = USBHostGetCurrentConfigurationDescriptor( address );

    i = 0;

    #ifdef DEBUG_MODE
        UART2PrintString("MSD: Checking descriptor " );
        UART2PutDec( descriptor[i+5] );
        UART2PrintString(" ...\r\n" );
    #endif

    // Find the next interface descriptor.
    while (i < ((USB_CONFIGURATION_DESCRIPTOR *)descriptor)->wTotalLength)
    {
        #ifdef DEBUG_MODE
            UART2PrintString("MSD:  Checking interface...\r\n" );
        #endif
        // See if we are pointing to an interface descriptor.
        if (descriptor[i+1] == USB_DESCRIPTOR_INTERFACE)
        {
            // See if the interface is a Mass Storage Device interface.
            if (descriptor[i+5] == DEVICE_CLASS_MASS_STORAGE)
            {
                // See if the interface subclass and protocol are correct.
                if (((descriptor[i+6] == DEVICE_SUBCLASS_SCSI) || (descriptor[i+6] == DEVICE_SUBCLASS_REMOVABLE)) &&
                    (descriptor[i+7] == DEVICE_INTERFACE_PROTOCOL_BULK_ONLY))
                {
                    deviceInfoMSD[device].interface = descriptor[i+2];

                    // Look for bulk IN and OUT endpoints.
                    endpointIN  = 0;
                    endpointOUT = 0;

                    // Scan for endpoint descriptors.
                    i += descriptor[i];
                    while (descriptor[i+1] == USB_DESCRIPTOR_ENDPOINT)
                    {
                        if (descriptor[i+3] == 0x02) // Bulk
                        {
                            if (((descriptor[i+2] & 0x80) == 0x80) && (endpointIN == 0))
                            {
                                endpointIN = descriptor[i+2];
                            }
                            if (((descriptor[i+2] & 0x80) == 0x00) && (endpointOUT == 0))
                            {
                                endpointOUT = descriptor[i+2];
                            }
                        }
                        i += descriptor[i];
                    }

                    if ((endpointIN != 0) && (endpointOUT != 0))
                    {
                        // Initialize the device information.
                        deviceInfoMSD[device].deviceAddress    = address;
                        deviceInfoMSD[device].clientDriverID   = clientDriverID;
                        deviceInfoMSD[device].endpointIN       = endpointIN;
                        deviceInfoMSD[device].endpointOUT      = endpointOUT;
                        #ifdef DEBUG_MODE
                            UART2PrintString( "MSD: Bulk endpoint IN: " );
                            UART2PutHex( endpointIN );
                            UART2PrintString( " Bulk endpoint OUT: " );
                            UART2PutHex( endpointOUT );
                            UART2PrintString( "\r\n" );
                        #endif
                        USBHostSetNAKTimeout( address, endpointIN,  1, USB_NUM_BULK_NAKS );
                        USBHostSetNAKTimeout( address, endpointOUT, 1, USB_NUM_BULK_NAKS );

                        #ifndef USB_ENABLE_TRANSFER_EVENT
                            deviceInfoMSD[device].state = STATE_INITIALIZE_DEVICE;
                        #else
                            // Initiate the request to get the max LUN.  If we can't initiate the request,
                            // then we can't enumerate the device.
                            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[device].deviceAddress,
                                    USB_SETUP_DEVICE_TO_HOST | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                                    USB_MSD_GET_MAX_LUN, 0, deviceInfoMSD[device].interface, 1, deviceInfoMSD[device].block.data,
                                    USB_DEVICE_REQUEST_GET, deviceInfoMSD[device].clientDriverID );
                            if (errorCode)
                            {
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Cannot get Max LUN - " );
                                    UART2PutHex( errorCode );
                                    UART2PrintString( "\r\n" );
                                #endif
                                return false;
                            }
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Getting Max LUN\r\n" );
                            #endif
                            deviceInfoMSD[device].state = STATE_WAIT_FOR_MAX_LUN;
                        #endif

                        return true;
                    }
                }
            }
        }

        // Jump to the next descriptor in this configuration.
        i += descriptor[i];
    }

    // This configuration is not valid for a Mass storage device.
    return false;
}


/****************************************************************************
  Function:
    bool USBHostMSDEventHandler( uint8_t address, USB_EVENT event,
                            void *data, uint32_t size )

  Summary:
    This function is the event handler for this client driver.

  Description:
    This function is the event handler for this client driver.  It is called
    by the host layer when various events occur.

  Precondition:
    The device has been initialized.

  Parameters:
    uint8_t address    - Address of the device
    USB_EVENT event - Event that has occurred
    void *data      - Pointer to data pertinent to the event
    uint16_t size       - Size of the data

  Return Values:
    true   - Event was handled
    false  - Event was not handled

  Remarks:
    None
  ***************************************************************************/

bool USBHostMSDEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
    #if defined( USB_ENABLE_TRANSFER_EVENT )
        uint8_t    errorCode;
    #endif
    uint8_t    i;

    switch (event)
    {
        case EVENT_NONE:             // No event occured (NULL event)
            USBTasks();
            return true;
            break;

        case EVENT_DETACH:           // USB cable has been detached (data: uint8_t, address of device)
            #ifdef DEBUG_MODE
                UART2PrintString( "MSD: Detach\r\n" );
            #endif

            // Find the device in the table.  If found, clear the important fields.
            for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != address); i++);
            if (i < USB_MAX_MASS_STORAGE_DEVICES)
            {
                deviceInfoMSD[i].deviceAddress    = 0;
                deviceInfoMSD[i].state            = STATE_DETACHED;

                // Inform the next higher layer of the event.
                usbMediaInterfaceTable.EventHandler( address, EVENT_DETACH, NULL, 0 );
            }
            return true;
            break;

        case EVENT_TRANSFER:         // A USB transfer has completed - optional
            #if defined( USB_ENABLE_TRANSFER_EVENT )
                #ifdef DEBUG_MODE
                    UART2PrintString( "MSD: transfer event: " );
                    UART2PutHex( address );
                    UART2PrintString( "\r\n" );
                #endif

                for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != address); i++) {}
                if (i == USB_MAX_MASS_STORAGE_DEVICES)
                {
                    #ifdef DEBUG_MODE
                        UART2PrintString( "MSD: Unknown device\r\n" );
                    #endif
                    return false;
                }

                #ifdef DEBUG_MODE
                    UART2PrintString( "MSD: Device state: " );
                    UART2PutHex( deviceInfoMSD[i].state );
                    UART2PrintString( "\r\n" );
                #endif
                switch (deviceInfoMSD[i].state)
                {
                    case STATE_WAIT_FOR_MAX_LUN:
                        #ifdef DEBUG_MODE
                            UART2PrintString( "MSD: Got Max LUN\r\n" );
                        #endif
                        deviceInfoMSD[i].maxLUN = 0;
                        if (!((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            deviceInfoMSD[i].maxLUN = deviceInfoMSD[i].block.data[0];
                        }
                        else
                        {
                            // Clear the STALL.  Since it is EP0, we do not have to clear the stall.
                            USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, 0 );
                        }

                        // Tell the media interface layer that we have a MSD attached.
                        if (usbMediaInterfaceTable.Initialize( deviceInfoMSD[i].deviceAddress, usbMediaInterfaceTable.flags, 0 ))
                        {
                            usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_MAX_LUN, &(deviceInfoMSD[i].maxLUN), 1 );
                            deviceInfoMSD[i].state = STATE_RUNNING;
                        }
                        else
                        {
                            // The media interface layer cannot support the device.
                            deviceInfoMSD[i].errorCode = USB_MSD_MEDIA_INTERFACE_ERROR;
                            deviceInfoMSD[i].state     = STATE_HOLDING;
                        }
                        return true;
                        break;

                    case STATE_RUNNING:
                        // Shouldn't get any transfer events here.
                        return false;
                        break;

                    case STATE_CBW_WAIT:
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Error with sending CBW\r\n" );
                            #endif
                            _USBHostMSD_TerminateTransfer( ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                        }
                        else if (((HOST_TRANSFER_DATA *)data)->dataCount != CBW_SIZE)
                        {
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: CBW size not correct\r\n" );
                            #endif
                            _USBHostMSD_TerminateTransfer( USB_MSD_CBW_ERROR );
                        }
                        else
                        {
                            if ((((USB_MSD_CBW *)(deviceInfoMSD[i].block.data))->dCBWDataTransferLength == 0) ||
                                (deviceInfoMSD[i].userDataLength == 0))
                            {
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Transfer length=0\r\n" );
                                #endif

                                // Skip to getting the CSW
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Getting CSW\r\n" );
                                #endif

                                errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN, deviceInfoMSD[i].block.data, CSW_SIZE );
                                if (errorCode)
                                {
                                    _USBHostMSD_TerminateTransfer( errorCode );
                                }
                                else
                                {
                                    deviceInfoMSD[i].state = STATE_CSW_WAIT;
                                }
                            }
                            else
                            {
                                #ifdef DEBUG_MODE
                                    UART2PrintString( "MSD: Going on...\r\n" );
                                #endif
                                if (!deviceInfoMSD[i].flags.bfDirection) // OUT
                                {
                                    errorCode = USBHostWrite( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA, deviceInfoMSD[i].userData, deviceInfoMSD[i].userDataLength );
                                }
                                else
                                {
                                    errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA, deviceInfoMSD[i].userData, deviceInfoMSD[i].userDataLength );
                                }

                                if (errorCode)
                                {
                                    _USBHostMSD_TerminateTransfer( errorCode );
                                }
                                else
                                {
                                    deviceInfoMSD[i].state = STATE_TRANSFER_WAIT;
                                }
                            }
                        }
                        return true;
                        break;

                    case STATE_TRANSFER_WAIT:
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            if (((HOST_TRANSFER_DATA *)data)->bErrorCode == USB_ENDPOINT_STALLED)
                            {
                                // Clear the stall, then try to get the CSW.
                                USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA );
                                if (!deviceInfoMSD[i].flags.bfDirection) // OUT
                                {
                                    deviceInfoMSD[i].flags.bfClearDataOUT = 1;
                                }
                                else
                                {
                                    deviceInfoMSD[i].flags.bfClearDataIN = 1;
                                }
                                deviceInfoMSD[i].returnState = STATE_REQUEST_CSW;
                                _USBHostMSD_ResetStateJump( i );

                            }
                            else
                            {
                                //Error recovery here is not explicitly covered in the spec.
                                //_USBHostMSD_TerminateTransfer( errorCode );
                                deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
                                deviceInfoMSD[i].returnState = STATE_RUNNING;
                                _USBHostMSD_ResetStateJump( i );
                            }
                        }
                        else
                        {
                            #ifdef DEBUG_MODE
                                UART2PrintString( "MSD: Getting CSW\r\n" );
                            #endif

                            errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN, deviceInfoMSD[i].block.data, CSW_SIZE );
                            if (errorCode)
                            {
                                _USBHostMSD_TerminateTransfer( errorCode );
                            }
                            else
                            {
                                deviceInfoMSD[i].state = STATE_CSW_WAIT;
                            }
                        }
                        break;

                    case STATE_CSW_WAIT:
                        #ifdef DEBUG_MODE
                            UART2PrintString( "MSD: Got CSW - " );
                        #endif
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            deviceInfoMSD[i].attemptsCSW--;
                            if (deviceInfoMSD[i].attemptsCSW)
                            {
                                USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN );
                                deviceInfoMSD[i].flags.bfClearDataIN = 1;
                                deviceInfoMSD[i].returnState = STATE_REQUEST_CSW;
                                _USBHostMSD_ResetStateJump( i );
                            }
                            else
                            {
                                _USBHostMSD_TerminateTransfer( ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                            }
                        }
                        else if ((((HOST_TRANSFER_DATA *)data)->dataCount != CSW_SIZE) |
                                 (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWSignature != USB_MSD_DCSWSIGNATURE) |
                                 (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWTag       != deviceInfoMSD[i].dCBWTag) )
                        {
                            _USBHostMSD_TerminateTransfer( USB_MSD_CSW_ERROR );
                        }
                        else
                        {
                            deviceInfoMSD[i].bytesTransferred = deviceInfoMSD[i].userDataLength - ((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWDataResidue;

                            if (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWStatus != 0x00)
                            {
                                _USBHostMSD_TerminateTransfer( ((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWStatus | USB_MSD_ERROR );
                            }
                            else
                            {
                                _USBHostMSD_TerminateTransfer( USB_SUCCESS );
                            }

                            // If we have a phase error, we need to perform corrective action instead of
                            // returning to normal running.
                            if (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWStatus == MSD_PHASE_ERROR)
                            {
                                deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
                                deviceInfoMSD[i].returnState = STATE_RUNNING;
                                _USBHostMSD_ResetStateJump( i );
                            }
                        }
                        break;

                    case STATE_WAIT_FOR_RESET:
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            //Error recovery here is not explicitly covered in the spec.
                            _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                        }
                        else
                        {
                            deviceInfoMSD[i].flags.bfReset = 0;
                            _USBHostMSD_ResetStateJump( i );
                        }
                        break;

                    case STATE_WAIT_FOR_CLEAR_IN:
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            //Error recovery here is not explicitly covered in the spec.
                            _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                        }
                        else
                        {
                            deviceInfoMSD[i].flags.bfClearDataIN = 0;
                            _USBHostMSD_ResetStateJump( i );
                        }
                        break;

                    case STATE_WAIT_FOR_CLEAR_OUT:
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                        {
                            //Error recovery here is not explicitly covered in the spec.
                            _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                        }
                        else
                        {
                            deviceInfoMSD[i].flags.bfClearDataOUT = 0;
                            _USBHostMSD_ResetStateJump( i );
                        }
                        break;

                    case STATE_HOLDING:
                        break;
                }
            #endif

        case EVENT_SOF:              // Start of frame - NOT NEEDED
        case EVENT_RESUME:           // Device-mode resume received
        case EVENT_SUSPEND:          // Device-mode suspend/idle event received
        case EVENT_RESET:            // Device-mode bus reset received
        case EVENT_STALL:            // A stall has occured
            return true;
            break;

        case EVENT_BUS_ERROR:
            #if defined( USB_ENABLE_TRANSFER_EVENT )
            #ifdef DEBUG_MODE
                UART2PrintString( "MSD: transfer event: " );
                UART2PutHex( address );
                UART2PrintString( "\r\n" );
            #endif

            for (i=0; (i<USB_MAX_MASS_STORAGE_DEVICES) && (deviceInfoMSD[i].deviceAddress != address); i++) {}
            if (i == USB_MAX_MASS_STORAGE_DEVICES)
            {
                #ifdef DEBUG_MODE
                    UART2PrintString( "MSD: Unknown device\r\n" );
                #endif
                return false;
            }

            #ifdef DEBUG_MODE
                UART2PrintString( "MSD: Device state: " );
                UART2PutHex( deviceInfoMSD[i].state );
                UART2PrintString( "\r\n" );
            #endif
            switch (deviceInfoMSD[i].state)
            {
                case STATE_WAIT_FOR_MAX_LUN:
                    #ifdef DEBUG_MODE
                        UART2PrintString( "MSD: Got Max LUN\r\n" );
                    #endif

                     // Clear the STALL.  Since it is EP0, we do not have to clear the stall.
                     USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, 0 );

                    // Tell the media interface layer that we have a MSD attached.
                    if (usbMediaInterfaceTable.Initialize( deviceInfoMSD[i].deviceAddress, usbMediaInterfaceTable.flags, 0 ))
                    {
                        usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_MAX_LUN, &(deviceInfoMSD[i].maxLUN), 1 );
                        deviceInfoMSD[i].state = STATE_RUNNING;
                    }
                    else
                    {
                        // The media interface layer cannot support the device.
                        deviceInfoMSD[i].errorCode = USB_MSD_MEDIA_INTERFACE_ERROR;
                        deviceInfoMSD[i].state     = STATE_HOLDING;
                    }
                    return true;
                    break;

                case STATE_RUNNING:
                    // Shouldn't get any transfer events here.
                    return false;
                    break;

                case STATE_CBW_WAIT:
                    if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                    {
                        #ifdef DEBUG_MODE
                            UART2PrintString( "MSD: Error with sending CBW\r\n" );
                        #endif
                        _USBHostMSD_TerminateTransfer( ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                    }
                    else if (((HOST_TRANSFER_DATA *)data)->dataCount != CBW_SIZE)
                    {
                        #ifdef DEBUG_MODE
                            UART2PrintString( "MSD: CBW size not correct\r\n" );
                        #endif
                        _USBHostMSD_TerminateTransfer( USB_MSD_CBW_ERROR );
                    }
                    return true;
                    break;

                case STATE_TRANSFER_WAIT:
                    if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                    {
                        if (((HOST_TRANSFER_DATA *)data)->bErrorCode == USB_ENDPOINT_STALLED)
                        {
                            // Clear the stall, then try to get the CSW.
                            USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointDATA );
                            if (!deviceInfoMSD[i].flags.bfDirection) // OUT
                            {
                                deviceInfoMSD[i].flags.bfClearDataOUT = 1;
                            }
                            else
                            {
                                deviceInfoMSD[i].flags.bfClearDataIN = 1;
                            }
                            deviceInfoMSD[i].returnState = STATE_REQUEST_CSW;
                            _USBHostMSD_ResetStateJump( i );

                        }
                        else
                        {
                            //Error recovery here is not explicitly covered in the spec.
                            //_USBHostMSD_TerminateTransfer( errorCode );
                            deviceInfoMSD[i].flags.val |= MARK_RESET_RECOVERY;
                            deviceInfoMSD[i].returnState = STATE_RUNNING;
                            _USBHostMSD_ResetStateJump( i );
                        }
                    }
                    break;

                case STATE_CSW_WAIT:
                    if (((HOST_TRANSFER_DATA *)data)->bErrorCode)
                    {
                        deviceInfoMSD[i].attemptsCSW--;
                        if (deviceInfoMSD[i].attemptsCSW)
                        {
                            USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN );
                            deviceInfoMSD[i].flags.bfClearDataIN = 1;
                            deviceInfoMSD[i].returnState = STATE_REQUEST_CSW;
                            _USBHostMSD_ResetStateJump( i );
                        }
                        else
                        {
                            _USBHostMSD_TerminateTransfer( ((HOST_TRANSFER_DATA *)data)->bErrorCode );
                        }
                    }
                    else if ((((HOST_TRANSFER_DATA *)data)->dataCount != CSW_SIZE) |
                             (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWSignature != USB_MSD_DCSWSIGNATURE) |
                             (((USB_MSD_CSW *)(deviceInfoMSD[i].block.data))->dCSWTag       != deviceInfoMSD[i].dCBWTag) )
                    {
                        _USBHostMSD_TerminateTransfer( USB_MSD_CSW_ERROR );
                    }
                    break;

                case STATE_WAIT_FOR_RESET:
                    //Error recovery here is not explicitly covered in the spec.
                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                    break;

                case STATE_WAIT_FOR_CLEAR_IN:
                    //Error recovery here is not explicitly covered in the spec.
                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                    break;

                case STATE_WAIT_FOR_CLEAR_OUT:
                    //Error recovery here is not explicitly covered in the spec.
                    _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
                    break;

                case STATE_HOLDING:
                    break;
            }
            return true;
            #endif
        default:
            return false;
            break;
    }

    return false;
}


// *****************************************************************************
// *****************************************************************************
// Section: Internal Functions
// *****************************************************************************
// *****************************************************************************

/****************************************************************************
  Function:
    uint32_t _USBHostMSD_GetNextTag( void )

  Description:
    Each bulk transfer has a transfer ID that is present in the CBW and must
    match in the CWS.  This function gets the next ID number for a transfer.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    uint32_t - Tag to use in the next CBW

  Remarks:
    None
  ***************************************************************************/

uint32_t _USBHostMSD_GetNextTag( void )
{
    return dCBWTagNext++;
}


/****************************************************************************
  Function:
    void _USBHostMSD_ResetStateJump( uint8_t i )

  Summary:


  Description:
    This function determines which portion of the reset processing needs to
    be executed next and jumps to that state.

  Precondition:
    The device information must be in the deviceInfoMSD array.

  Parameters:
    uint8_t i  - Index into the deviceInfoMSD structure for the device to reset.

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/

void _USBHostMSD_ResetStateJump( uint8_t i )
{
    #ifdef USB_ENABLE_TRANSFER_EVENT
        uint8_t    errorCode;
    #endif

    if (deviceInfoMSD[i].flags.bfReset)
    {
        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].state = STATE_MSD_RESET_RECOVERY;
        #else
            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_CLASS | USB_SETUP_RECIPIENT_INTERFACE,
                            USB_MSD_RESET, 0, deviceInfoMSD[i].interface, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

            if (errorCode)
            {
                //Error recovery here is not explicitly covered in the spec.
                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
            }
            else
            {
                deviceInfoMSD[i].state = STATE_WAIT_FOR_RESET;
            }
        #endif
    }
    else if (deviceInfoMSD[i].flags.bfClearDataIN)
    {
        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].state = STATE_MSD_CLEAR_DATA_IN;
        #else
            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_ENDPOINT,
                            USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, deviceInfoMSD[i].endpointIN, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

            if (errorCode)
            {
                //Error recovery here is not explicitly covered in the spec.
                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
            }
            else
            {
                deviceInfoMSD[i].state = STATE_WAIT_FOR_CLEAR_IN;
            }
        #endif
    }
    else if (deviceInfoMSD[i].flags.bfClearDataOUT)
    {
        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].state = STATE_MSD_CLEAR_DATA_OUT;
        #else
            errorCode = USBHostIssueDeviceRequest( deviceInfoMSD[i].deviceAddress, USB_SETUP_HOST_TO_DEVICE | USB_SETUP_TYPE_STANDARD | USB_SETUP_RECIPIENT_ENDPOINT,
                            USB_REQUEST_CLEAR_FEATURE, USB_FEATURE_ENDPOINT_HALT, deviceInfoMSD[i].endpointOUT, 0, NULL, USB_DEVICE_REQUEST_SET, deviceInfoMSD[i].clientDriverID );

            if (errorCode)
            {
                //Error recovery here is not explicitly covered in the spec.
                _USBHostMSD_TerminateTransfer( USB_MSD_RESET_ERROR );
            }
            else
            {
                deviceInfoMSD[i].state = STATE_WAIT_FOR_CLEAR_OUT;
            }
        #endif
    }
    else
    {
        usbMediaInterfaceTable.EventHandler( deviceInfoMSD[i].deviceAddress, EVENT_MSD_RESET, NULL, 0 );

        // Clear the errors so we can try again.
        USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN );
        USBHostClearEndpointErrors( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointOUT );

        #ifndef USB_ENABLE_TRANSFER_EVENT
            deviceInfoMSD[i].state = deviceInfoMSD[i].returnState;
        #else
            if (deviceInfoMSD[i].returnState == STATE_REQUEST_CSW)
            {
                // Request the CSW
                #ifdef DEBUG_MODE
                    UART2PrintString( "MSD: Getting CSW\r\n" );
                #endif

                errorCode = USBHostRead( deviceInfoMSD[i].deviceAddress, deviceInfoMSD[i].endpointIN, deviceInfoMSD[i].block.data, CSW_SIZE );
                if (errorCode)
                {
                    _USBHostMSD_TerminateTransfer( errorCode );
                }
                else
                {
                    deviceInfoMSD[i].state = STATE_CSW_WAIT;
                }
            }
            else
            {
                deviceInfoMSD[i].state = deviceInfoMSD[i].returnState;
            }
        #endif

    }
}


