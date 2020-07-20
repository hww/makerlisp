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

#ifndef _PHD_COM_H
#define _PHD_COM_H

/** I N C L U D E S **********************************************************/
#include <stdint.h>
#include <stdbool.h>


/** D E F I N I T I O N S ****************************************************/
#define BYTE_SWAP16(a) (uint16_t)((((uint16_t)(a)&0xFF00)>>8) | \
                                    (((uint16_t)(a)&0x00FF)<<8))

/* callback function pointer structure for Application to handle events */
typedef void(* PHDC_APP_CB)(uint8_t);


/* Application States */
#define PHD_DISCONNECTED  0x00
#define PHD_CONNECTING    0x01
#define PHD_CONNECTED     0x02
#define PHD_DISCONNECTING 0x03
#define PHD_MEASUREMENT_SENT 0x04
#define PHD_MEASUREMENT_SENDING 0x05


/* Agent states */
#define  PHD_COM_STATE_DISCONNECTED                  0x00
#define  PHD_COM_STATE_UNASSOCIATED                  0x01
#define  PHD_COM_STATE_ASSOCIATING                   0x02
#define  PHD_COM_STATE_ASSOC_CFG_SENDING_CONFIG      0x03
#define  PHD_COM_STATE_ASSOC_CFG_WAITING_APPROVAL    0x04
#define  PHD_COM_STATE_ASSOC_OPERATING               0x05
#define  PHD_COM_STATE_DISASSOCIATING                0x06

/* requests */
#define PHD_ASSOCIATION_REQUEST     0xE200
#define PHD_ASSOCIATION_RESPONSE    0xE300
#define PHD_RELEASE_REQUEST         0xE400
#define PHD_RELEASE_RESPONSE        0xE500
#define PHD_ABORT_REQUEST           0xE600
#define PHD_PRESET_APDU             0xE700


 /******************************************************************************
 * Function:
 *      void PHDConnect(void)
 *
 * Summary:
 *      This function is used to connect to the PHD Manager.
 *
 * Description:
 *       This function initiates connection to the PHD Manager by sending an
 *   Association request to manager.  The Agent doesn't get connected to
 *	 the Manager immediately after calling this function. Upon receiving
 *	 the association request from an Agent, the PHD Manager responds with
 *	 an association response. The association response tells whether Manager
 *	 accepting the request or rejecting it. The Association response from
 *	 the Manager is handled by the PHD stack. The PHD stack calls a callback
 *	 function (void(* PHDC_APP_CB)(uint8_t)) to the application with status of
 *	 the connection.
 *	 The Manager should respond to the Agent within the specified timeout of
 *	 ASSOCIATION_REQUEST_TIMEOUT. The Agent should send the Association request
 *	 once more if no response is received from Manager and ASSOCIATION_REQUEST_TIMEOUT
 *   is expired. This function starts a Timer for the Association Timeout request.
 *   The timeout is handled by the PHDTimeoutHandler() function.
 *
 * Conditions:
 *       The agent should be in PHD_INITIALIZED state.
 *
 * Parameters:
 *	None
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      None
 *
 *****************************************************************************/
void PHDConnect(void);

/******************************************************************************
 * Function:
 *      void PHDDisConnect(void)
 *
 * Summary:
 *      This function is used to disconnect from the PHD Manager.
 *
 * Description:
 *       This function initiates disconnection of the Agent from the PHD Manager by sending an
 *	 Release request to manager.  The Agent doesn't get disconnected from the Manager
 *	 immediately after calling this function. The PHD Manager sends back a release response
 *	 to the Agent. The Agent responds back with an Abort Message and the Agent moves to DISCONNECTED
 *	 state. The PHD stack calls a callback function (void(* PHDC_APP_CB)(uint8_t)) to the application
 *	 with status of the connection. This function disables all timeout.
 *
 * Conditions:
 *    None.
 *
 * Parameters:
 *	None
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      None
 *
 *****************************************************************************/
void PHDDisConnect(void);


/******************************************************************************
 * Function:
 *      void PHDAppInit(PHDC_APP_CB callback)
 *
 * Summary:
 *      This function is used to initialize the PHD stack.
 *
 * Description:
 *       This function initializes all the application related items.
 *       The input to the function is address of the callback function. This callback function
 *	 which will be called by PHD stack when there is a change in Agent's connection status.
 *
 * Conditions:
 *       None
 *
 * Parameters:
 *	PHDC_APP_CB callback - Pointer to application Call Back Function.
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      None
 *
 *****************************************************************************/
void PHDAppInit(PHDC_APP_CB);

/******************************************************************************
 * Function:
 *      void PHDTimeoutHandler(void)
 *
 * Summary:
 *      This function is used to handle all timeout.
 *
 * Description:
 *       This function handles all timers. This function should be called once in every milli Second.
 *
 * Conditions:
 *       None
 *
 * Parameters:
 *	None
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      If USB is used at the Transport layer then the USB SOF handler can call this function.
 *
 *****************************************************************************/
void PHDTimeoutHandler(void);

/******************************************************************************
 * Function:
 *      void PHDSendMeasuredData(void)
 *
 * Summary:
 *      This function is used to send measurement data to the PHD Manager.
 *
 * Description:
 *       This function sends measurement data to manager. Before calling this function
 *       the caller should fill the Application buffer with the data to send. The Agent
 *	 expects a Confirmation from the Manager for the data sent. This confirmation should
 *	 arrive at the Agent within a specified time of CONFIRM_TIMEOUT. The function starts
 *	 a Timer to see if the Confirmation from the Manager arrives within specified time.
 *	 The timeout is handled by the PHDTimeoutHandler() function.
 *
 * Conditions:
 *       Before calling this function the caller should fill the Application buffer with the data to send.
 *
 * Parameters:
 *	None
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      None
 *
 *****************************************************************************/
void PHDSendMeasuredData(void);

/******************************************************************************
 * Function:
 *      void PHDSendAppBufferPointer(uint8_t * pAppBuffer)
 *
 * Summary:
 *      This function is used to send measurement data to the PHD Manager.
 *
 * Description:
 *       This function passes the application buffer pointer to the PHD stack. The PHD stack
 *       uses this pointer send and receive data through the transport layer.
 *
 * Conditions:
 *
 *
 * Parameters:
 *	uint8_t *pAppBuffer - Pointer to Application Buffer.
 *
 * Return:
 *	None
 *
 * Side Effects:
 *	None
 *
 * Remarks:
 *      None
 *
 *****************************************************************************/
void PHDSendAppBufferPointer(uint8_t * pAppBuffer);

#endif

