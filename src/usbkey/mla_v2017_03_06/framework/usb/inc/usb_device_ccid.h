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

#ifndef CCID_H
#define CCID_H

/** I N C L U D E S *******************************************************/


/** DEFINITIONS ****************************************************/
/* CCID Bulk OUT transfer states */
#define USB_CCID_BULK_OUT_FIRST_PACKET 		 0
#define USB_CCID_BULK_OUT_SUBSEQUENT_PACKET  1

/* CCID Bulk IN transfer states */
#define USB_CCID_BULK_IN_READY                0
#define USB_CCID_BULK_IN_BUSY                 1
#define USB_CCID_BULK_IN_BUSY_ZLP             2       // ZLP: Zero Length Packet
#define USB_CCID_BULK_IN_COMPLETING           3

/****** CCID Class-Specific Request Codes ************/
#define USB_CCID_ABORT  					0x01
#define USB_CCID_GET_CLOCK_FREQUENCIES      0x02
#define USB_CCID_GET_DATA_RATES  		    0x03

// CCID Commands/response
#define USB_CCID_PC_TO_RDR_ICC_POWER_ON	        0x62
#define USB_CCID_PC_TO_RDR_ICC_POWER_OFF        0x63
#define USB_CCID_PC_TO_RDR_GET_SLOT_STATUS      0x65
#define USB_CCID_PC_TO_RDR_XFR_BLOCK            0x6F
#define USB_CCID_PC_TO_RDR_GET_PARAMETERS       0x6C
#define USB_CCID_PC_TO_RDR_RESET_PARAMETERS	    0x6D
#define USB_CCID_PC_TO_RDR_SET_PARAMETERS	    0x61
#define USB_CCID_PC_TO_RDR_ESCAPE               0x6B
#define USB_CCID_PC_TO_RDR_ICC_CLOCK            0x6E
#define USB_CCID_PC_TO_RDR_T0APDU               0x6A
#define USB_CCID_PC_TO_RDR_SECURE               0x69
#define USB_CCID_PC_TO_RDR_MECHANICAL           0x71
#define USB_CCID_PC_TO_RDR_ABORT                0x72
#define USB_CCID_PC_TO_RDR_SET_DATA_RATE_AND_CLOCK_FREQUENCY  0x73

#define USB_CCID_RDR_TO_PC_DATA_BLOCK		    0x80
#define USB_CCID_RDR_TO_PC_SLOT_STATUS          0x81
#define USB_CCID_RDR_TO_PC_PARAMETERS   	    0x82
#define USB_CCID_RDR_TO_PC_ESCAPE               0x83
#define USB_CCID_RDR_TO_PC_DATA_RATE_AND_CLOCK_FREQUENCY 0x84

//CCID Errors
#define USB_CCID_CMD_ABORTED                 0xFF
#define USB_CCID_ICC_MUTE                    0xFE
#define USB_CCID_XFR_PARITY_ERROR            0xFD
#define USB_CCID_XFR_OVERRUN                 0xFC
#define USB_CCID_HW_ERROR                    0xFB
#define USB_CCID_BAD_ATR_TS                  0xF8
#define USB_CCID_BAD_ATR_TCK                 0xF7
#define USB_CCID_ICC_PROTOCOL_NOT_SUPPORTED  0xF6
#define USB_CCID_ICC_CLASS_NOT_SUPPORTED     0xF5
#define USB_CCID_PROCEDURE_BYTE_CONFLICT     0xF4
#define USB_CCID_DEACTIVATED_PROTOCOL        0xF3
#define USB_CCID_BUSY_WITH_AUTO_SEQUENCE     0xF2
#define USB_CCID_PIN_TIMEOUT                 0xF0
#define USB_CCID_PIN_CANCELLED               0xEF
#define USB_CCID_CMD_SLOT_BUSY               0xE0
#define USB_CCID_CMD_NOT_SUPPORTED           0x00

    
//protocols
#define USB_CCID_T0_PROTOCOL 0
#define USB_CCID_T1_PROTOCOL 1

/** E X T E R N S ************************************************************/
extern volatile CTRL_TRF_SETUP SetupPkt;
extern USB_HANDLE usbCcidBulkOutHandle;
extern USB_HANDLE usbCcidBulkInHandle;
extern USB_HANDLE usbCcidInterruptInHandle;
extern unsigned char usbCcidBulkOutEndpoint[USB_EP_SIZE];	//User application buffer for receiving and holding OUT packets sent from the host
extern unsigned char usbCcidBulkInEndpoint[USB_EP_SIZE];	//User application buffer for sending IN packets to the host

/** Section: PUBLIC PROTOTYPES **********************************************/

/******************************************************************************
    Function:
        void mUSBCCIDBulkInRam(BYTE *pData, BYTE len)
        
    Description:
        Use this macro to transfer data located in data memory.
        Use this macro when:
            1. Data stream is not null-terminated
            2. Transfer length is known
        Remember: usbCcidBulkInTrfState must == USB_CCID_BULK_IN_READY
   		Unexpected behavior will occur if this function is called when 
		usbCcidBulkInTrfState != USB_CCID_BULK_IN_READY
        
    PreCondition:
        usbCcidBulkInTrfState must be in the USB_CCID_BULK_IN_READY state.
        
    Paramters:
        pDdata  : Pointer to the starting location of data bytes
        len     : Number of bytes to be transferred
        
    Return Values:
        None
        
    Remarks:
        This macro only handles the setup of the transfer. The
        actual transfer is handled by USBCCIDBulkInService().
  
 *****************************************************************************/
#define mUSBCCIDBulkInRam(pData,len)   \
{                                      \
    pCCIDSrc.bRam = pData;               \
    usbCcidBulkInLen = len;             \
    usbCcidBulkInTrfState = USB_CCID_BULK_IN_BUSY;    \
}

/******************************************************************************
 	Function:
 		void USBCheckCCIDRequest(void)
 
 	Description:
 		This routine checks the setup data packet to see if it
 		knows how to handle it
 		
 	PreCondition:
 		None

	Parameters:
		None
		
	Return Values:
		None
		
	Remarks:
		None
		 
  *****************************************************************************/
void USBCheckCCIDRequest(void);

/**************************************************************************
  Function:
        void USBCCIDInitEP(void)
    
  Summary:
    This function initializes the CCID function driver. This function should
    be called after the SET_CONFIGURATION command.
  Description:
    This function initializes the CCID function driver. This function sets
    the default line coding (baud rate, bit parity, number of data bits,
    and format). This function also enables the endpoints and prepares for
    the first transfer from the host.
    
    This function should be called after the SET_CONFIGURATION command.
    This is most simply done by calling this function from the
    USBCBInitEP() function.
    
    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            USBCCIDInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                   
  **************************************************************************/
void USBCCIDInitEP(void);

/************************************************************************
  Function:
        void USBCCIDBulkInService(void)
    
  Summary:
    USBCCIDBulkInService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state.
  Description:
    USBCCIDBulkInService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state.
    
    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Run application code.
                UserApplication();

				//Keep trying to send data to the PC as required
                USBCCIDBulkInService();
            }
        }
    }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                 
  ************************************************************************/
void USBCCIDBulkInService(void);

/******************************************************************************
  Function:
	void USBCCIDSendDataToHost(BYTE *data, WORD length)
		
  Summary:
    USBCCIDSendDataToHost writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).

  Description:
    USBCCIDSendDataToHost writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).
    
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. USBCCIDBulkInService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    

  Input:
    BYTE *data - pointer to a RAM array of data to be transfered to the host
    WORD length - the number of bytes to be transfered 
		
 *****************************************************************************/
void USBCCIDSendDataToHost(BYTE *pData, WORD len);

/** Section: STRUCTURES **********************************************/
typedef union {
    BYTE CCID_BulkOutBuffer[271];
    BYTE CCID_BulkInBuffer[267]; 
} USB_CCID_BUFFER;  

#endif //CCID_H
