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

#ifndef PHDC_H
#define PHDC_H

/** I N C L U D E S **********************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "usb.h"
#include "usb_config.h"
#include "phd_config.h"

/** D E F I N I T I O N S ****************************************************/


/* Class-Specific Requests */

// Upon receipt of the SET_FEATURE (FEATURE_PHDC_METADATA), the device shall turn on
// the Meta-Data Message Preamble feature.
#define SET_FEATURE

// Upon receipt of the CLEAR_FEATURE (FEATURE_PHDC_METADATA), the device shall turn off
// the Meta-Data Message Preamble feature
#define CLEAR_FEATURE

//An application may choose to use the Get DataStatus request to determine which 
//endpoints on the device have data
#define GET_DATA_STATUS   0x00


/* Device Class Code */
#define PHDC_DEVICE					0x0F

/* Class Protocol Codes */
#define NO_PROTOCOL                 0x00    // No class specific protocol required



#define DIR_IN 1
#define DIR_OUT 0

#define MEM_ROM 1
#define MEM_RAM 0


/* PHDC Bulk IN transfer states */
//#define PHDC_TX_READY                0
//#define PHDC_TX_BUSY                 1
//#define PHDC_TX_BUSY_ZLP             2       // ZLP: Zero Length Packet
//#define PHDC_TX_COMPLETING           3

#define PHDC_RX_ENDPOINTS			1
#define PHDC_TX_ENDPOINTS			2
#define PHDC_BULK_IN_QOS	0x08
#define PHDC_BULK_OUT_QOS	0x08
#define PHDC_INT_IN_QOS	0x01


/* Events to the Application */
#define USB_APP_SEND_COMPLETE               1
#define USB_APP_DATA_RECEIVED               2
#define USB_APP_GET_TRANSFER_SIZE           3


extern volatile FAR unsigned char phdc_data_rx[PHDC_DATA_OUT_EP_SIZE];
extern volatile FAR unsigned char phdc_data_tx[PHDC_DATA_IN_EP_SIZE];

/* callback function pointer structure for Application to handle events */
typedef void(* USB_PHDC_CB)(uint8_t, void *);

extern volatile uint8_t CtrlTrfData[USB_EP0_BUFF_SIZE]; 
/** E X T E R N S ************************************************************/
typedef struct PHDC_RX_ENDPOINT_STRUCT
{
	uint8_t ep_num;
	uint16_t transfer_size;
	uint8_t qos;
	uint16_t offset;
	uint8_t size;
	uint8_t len;	
	uint8_t *app_buff;
	USB_HANDLE PHDCDataOutHandle;
}PHDC_RX_ENDPOINT,*PTR_PHDC_RX_ENDPOINT;

typedef struct PHDC_TX_ENDPOINT_STRUCT
{
	uint8_t ep_num;
	uint16_t transfer_size;
	bool memtype;
	uint8_t qos;
	uint16_t offset;	
	uint16_t bytes_to_send;	
	uint8_t size;
	uint8_t len;	
	uint8_t *app_buff;
	USB_HANDLE PHDCDataInHandle;
}PHDC_TX_ENDPOINT,*PTR_PHDC_TX_ENDPOINT;

extern volatile CTRL_TRF_SETUP SetupPkt;
extern const uint8_t configDescriptor1[];

/******************************************************************************
 	Function:
 		void USBDevicePHDCCheckRequest(void)
 
 	Description:
 		This routine checks the setup data packet to see if it
 		is class specific request or vendor specific request
		and handles it
 		
 	PreCondition:
 		None

	Parameters:
		None
		
	Return Values:
		None
		
	Remarks:
		None
		 
  *****************************************************************************/
void USBDevicePHDCCheckRequest(void);

/**************************************************************************
  Function:
        void PHDCInitEP(void)
    
  Summary:
    This function initializes the PHDC function driver. This function should
    be called after the SET_CONFIGURATION command.
  Description:
    This function initializes the PHDC function driver. This function sets
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
            PHDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                   
  **************************************************************************/
extern void USBDevicePHDCInit(USB_PHDC_CB);
/**********************************************************************************
  Function:
        uint8_t USBDevicePHDCReceiveData(uint8_t qos, uint8_t *buffer, uint16_t len)
    
  Summary:
    USBDevicePHDCReceiveData copies a string of BYTEs received through USB PHDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.

  Description:
    USBDevicePHDCReceiveData copies a string of BYTEs received through USB PHDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.
    
    Typical Usage:
    <code>
        uint8_t numBytes;
        uint8_t buffer[64]
    
        numBytes = USBDevicePHDCReceiveData(buffer,sizeof(buffer)); //until the buffer is free.
        if(numBytes \> 0)
        {
            //we received numBytes bytes of data and they are copied into
            //  the "buffer" variable.  We can do something with the data
            //  here.
        }
    </code>
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for PHDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
	qos - quality of service
    buffer -  Pointer to where received BYTEs are to be stored
    len -     The number of BYTEs expected.
                                                                                   
  **********************************************************************************/

uint8_t USBDevicePHDCReceiveData(uint8_t qos, uint8_t *buffer, uint16_t len);

/******************************************************************************
  Function:
	void USBDevicePHDCSendData(uint8_t qos, uint8_t *data, uint8_t Length)
		
  Summary:
    USBDevicePHDCSendData writes an array of data to the USB. 

  Description:
    USBDevicePHDCSendData writes an array of data to the USB.
    
    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
            USBDevicePHDCSendData(1,data,5);
        }
    </code>
    
    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. USBDevicePHDCTxRXService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return true. This indicates that the last
    transfer is complete and is ready to receive a new block of data.

  Input:
	qos - Quality of service information
    *data - pointer to a RAM array of data to be transfered to the host
    length - the number of bytes to be transfered.
		
 *****************************************************************************/
void USBDevicePHDCSendData(uint8_t qos, uint8_t *data, uint16_t length, bool memtype);
/************************************************************************
  Function:
        void USBDevicePHDCTxRXService(void)
    
  Summary:
    USBDevicePHDCTxRXService handles device-to-host transaction(s) and host-to-device transaction(s).
	This function should be called once per Main Program loop after the device reaches
    the configured state.
  Description:
    USBDevicePHDCTxRXService handles device-to-host transaction(s) and host-to-device transaction(s).
	This function should be called once per Main Program loop after the device reaches
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
               (USBIsDeviceSuspended() == true))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep trying to send data to the PC as required
                USBDevicePHDCTxRXService();
    
                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    None
  Remarks:
    None                                                                 
  ************************************************************************/
void USBDevicePHDCTxRXService(USTAT_FIELDS* event);


/************************************************************************
  Function:
        void USBDevicePHDCUpdateStatus (uint16_t EndpointNo, bool Status)
    
  Summary:
    USBDevicePHDCUpdateStatus Function Gets the current status of an Endpoint and holds the status in variable phdcEpDataBitmap. The Status is sent to the host upon the 
    "Get Data Status" request from the host. 
    
  Description:
    USBDevicePHDCUpdateStatus Function helps to handle the "Get Data Status" PHDC specfic request received from the Host as mentioned in the 
    section 7.1.2 of the Personal Healthcare Devices Specification. This function Gets the current status of an Endpoint and holds the status in 
    variable phdcEpDataBitmap. 
    
  Input:
     uint16_t EndpointNo : The number of the endpoint, for which the status is requested. 
     
     bool Status:  Current status of the Endpoint. 
  
  Conditions:
    None
  Remarks:
    None                                                                 
  ************************************************************************/
void USBDevicePHDCUpdateStatus (uint16_t EndpointNo, bool Status);
#endif //PHDC_H
