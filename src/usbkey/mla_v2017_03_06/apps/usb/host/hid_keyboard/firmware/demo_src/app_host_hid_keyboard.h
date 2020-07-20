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

#include "usb_host_hid.h"

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
void APP_HostHIDKeyboardInitialize();

/*********************************************************************
* Function: void APP_HostHIDKeyboardTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized via
*   the APP_HostHIDKeyboardInitialize() function
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_HostHIDKeyboardTasks();

/*********************************************************************
* Function: bool APP_HostHIDKeyboardReportParser(void);
*
* Overview: Parses the report descriptor to determine if the report
*           matches what this demo will support
*
* PreCondition: None
*
* Input: None
*
* Output: bool - true if the demo supports this device, false otherwise.
*
********************************************************************/
bool APP_HostHIDKeyboardReportParser(void);