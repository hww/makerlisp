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

#include "usb_config.h"

#include <stdlib.h>
#include <string.h>

#include "usb.h"
#include "usb_host_hid.h"
#include "usb_host_hid_parser.h"


//#define DEBUG_MODE
#ifdef DEBUG_MODE
    #include "uart2.h"
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Constants
// *****************************************************************************
// *****************************************************************************


//******************************************************************************
//******************************************************************************
// Section: Data Structures
//******************************************************************************
//******************************************************************************


//******************************************************************************
//******************************************************************************
// Section: Macros
//******************************************************************************
//******************************************************************************
#ifndef USB_MALLOC
    #define USB_MALLOC(size) malloc(size)
#endif

#ifndef USB_FREE
    #define USB_FREE(ptr) free(ptr)
#endif

#define USB_FREE_AND_CLEAR(ptr) {USB_FREE(ptr); ptr = NULL;}

//******************************************************************************
//******************************************************************************
// Section: Local Prototypes
//******************************************************************************
//******************************************************************************

static void _USBHostHID_InitDeviceRptInfo(void);
static void _USBHostHID_Parse_Collection(HID_ITEM_INFO* ptrItem);
static void _USBHostHID_Parse_EndCollection(HID_ITEM_INFO* ptrItem);
static USB_HID_RPT_DESC_ERROR _USBHostHID_Parse_ReportType(HID_ITEM_INFO* item);
static void _USBHostHID_ConvertDataToSigned(HID_ITEM_INFO* item);

//******************************************************************************
//******************************************************************************
// Section: HID Host Global Variables
//******************************************************************************
//******************************************************************************

USB_HID_DEVICE_RPT_INFO deviceRptInfo = {0};
USB_HID_ITEM_LIST       itemListPtrs   ={NULL};
uint8_t*                   parsedDataMem = NULL;

/****************************************************************************
  Function:
    USB_HID_RPT_DESC_ERROR _USBHostHID_Parse_Report(uint8_t* hidReportDescriptor
                                 ,uint16_t lengthOfDescriptor, uint16_t pollRate,
                                  uint8_t interfaceNum)

  Description:
    This function is called by usb_host_hid.c after a valid configuration
    device is found. This function parses the report descriptor and stores
    data in data structures. Application can access these data structures
    to understand report format and device capabilities

  Precondition:
    None

  Parameters:
    uint8_t* hidReportDescriptor - Pointer to raw report descriptor
    uint16_t  lengthOfDescriptor  - Length of Report Descriptor
    uint16_t  pollRate            - Poll rate of the report
    uint8_t interfaceNum         - interface number of the respective report
                                descriptor.

  Return Values:
    USB_HID_RPT_DESC_ERROR    - Returns error code(enum) if found while
                                parsing the report descriptor

  Remarks:
    None
***************************************************************************/
USB_HID_RPT_DESC_ERROR _USBHostHID_Parse_Report(uint8_t* hidReportDescriptor , uint16_t lengthOfDescriptor , uint16_t pollRate, uint8_t interfaceNum)
{
   uint16_t  sizeRequired = 0;
   uint16_t  len_to_be_parsed =0;
   uint8_t* currentRptDescPtr = NULL;
   uint8_t* assignMem = NULL;
   /* Main Item Vars */
   HID_COLLECTION *collectionLocal = NULL;
   HID_REPORT *reportLocal = NULL;

   /* Global Item Vars */
   HID_REPORT *lreport = NULL;
   uint8_t lreportIndex = (uint8_t)0;

   /* Local Item Vars */
   HID_DESIGITEM *ldesignatorItem = NULL;
   HID_STRINGITEM *lstringItem = NULL;
   HID_USAGEITEM *lusageItem = NULL;

   /*HID  Error */
   USB_HID_RPT_DESC_ERROR lhidError = HID_ERR;

   HID_ITEM_INFO item;

   uint8_t  i=0;
   uint8_t  dataByte=0 ;
   uint8_t  ldataSize=0;

   if((hidReportDescriptor == NULL) ||(lengthOfDescriptor == 0))
    {
       /* set error flag */
       return(HID_ERR_NullPointer);
    }

    memset( &deviceRptInfo, 0x00, sizeof( USB_HID_DEVICE_RPT_INFO ) );
    _USBHostHID_InitDeviceRptInfo();
   
    deviceRptInfo.interfaceNumber = interfaceNum;  // update interface number for the report
    deviceRptInfo.reportPollingRate = pollRate;

    len_to_be_parsed = lengthOfDescriptor;
    currentRptDescPtr = hidReportDescriptor;

    while(len_to_be_parsed > 0)    /* First parse to calculate the space required for all the items */
    {
       item.ItemDetails.val = *currentRptDescPtr;
       /* Data need not be parsed at this point */
       ldataSize = item.ItemDetails.ItemSize ;
       if(item.ItemDetails.ItemSize == 3) 
           ldataSize = 4; 

       currentRptDescPtr += (ldataSize+1) ; /* point to next item i.e size of item data + 1(item detail) */
       len_to_be_parsed -= (ldataSize+1);   /* remaining bytes = current - (length of data + 1)*/

        switch (item.ItemDetails.ItemType)
            {
                case HIDType_Main:           /* Main Items */
                    switch (item.ItemDetails.ItemTag)
                    {
                        case HIDTag_Collection:
                            deviceRptInfo.collections++;
                            deviceRptInfo.collectionNesting++;
                            if (deviceRptInfo.collectionNesting > deviceRptInfo.maxCollectionNesting)
                                deviceRptInfo.maxCollectionNesting = deviceRptInfo.collectionNesting;
                            break;
                        case HIDTag_EndCollection:
                            if (deviceRptInfo.collectionNesting-- == 0)
                                  lhidError = HID_ERR_UnexpectedEndCollection ;/* Error: UnexpectedEndCollection */
                            break;
                        case HIDTag_Input:
                        case HIDTag_Output:
                        case HIDTag_Feature:
                            deviceRptInfo.reportItems++;
                            break;
                        default :
                    break;
                    }
                    break;
                case HIDType_Global:         /* Global Items */
                    switch (item.ItemDetails.ItemTag)
                    {
                        case HIDTag_ReportID:
                            deviceRptInfo.reports++;
                            break;
                        case HIDTag_Push:
                            deviceRptInfo.globalsNesting++;
                            if (deviceRptInfo.globalsNesting > deviceRptInfo.maxGlobalsNesting)
                                deviceRptInfo.maxGlobalsNesting = deviceRptInfo.globalsNesting;
                            break;
                        case HIDTag_Pop:
                            deviceRptInfo.globalsNesting--;
                            if (deviceRptInfo.globalsNesting > deviceRptInfo.maxGlobalsNesting)
                                lhidError = HID_ERR_UnexpectedPop ;/* Error: global nesting rolled to negative ... */
                            break;
                        default :
                            break;
                    }
                    break;
                case HIDType_Local:          /* Local Item */
                    switch (item.ItemDetails.ItemTag)
                    {
                        case HIDTag_Usage:
                            deviceRptInfo.usages++;
                            break;
                        case HIDTag_UsageMinimum:
                        case HIDTag_UsageMaximum:
                            deviceRptInfo.usageRanges++;
                            break;
                        case HIDTag_StringIndex:
                            deviceRptInfo.strings++;
                            break;
                        case HIDTag_StringMinimum:
                        case HIDTag_StringMaximum:
                            deviceRptInfo.stringRanges++;
                            break;
                        case HIDTag_DesignatorIndex:
                            deviceRptInfo.designators++;
                            break;
                        case HIDTag_DesignatorMinimum:
                        case HIDTag_DesignatorMaximum:
                            deviceRptInfo.designatorRanges++;
                            break;
                        default :
                            break;
                    }
                  break;
             default :
                  break;

        }
    }

    if(lhidError)
    {
        return(lhidError);
    }

    if (deviceRptInfo.collectionNesting != 0) return(HID_ERR_MissingEndCollection) /* HID_RPT_DESC_FORMAT_IMPROPER */;

    if (deviceRptInfo.collections == 1) return(HID_ERR_MissingTopLevelCollection) /* HID_RPT_DESC_FORMAT_IMPROPER */;

    if (deviceRptInfo.reportItems == 0) return(HID_ERR_NoReports)/* HID_RPT_DESC_FORMAT_IMPROPER */;

    if ((deviceRptInfo.usageRanges & 1) == 1) return(HID_ERR_UnmatchedUsageRange)/* HID_RPT_DESC_FORMAT_IMPROPER */;

    if ((deviceRptInfo.stringRanges & 1) == 1) return(HID_ERR_UnmatchedStringRange)/* HID_RPT_DESC_FORMAT_IMPROPER */;

    if ((deviceRptInfo.designatorRanges & 1) == 1) return(HID_ERR_UnmatchedDesignatorRange)/* HID_RPT_DESC_FORMAT_IMPROPER */;


   /* usages , strings & descriptors are in pair */
    deviceRptInfo.usages += (deviceRptInfo.usageRanges/2);
    deviceRptInfo.strings += (deviceRptInfo.stringRanges/2);
    deviceRptInfo.designators += (deviceRptInfo.designatorRanges/2);

   /* Calculate space required */

    sizeRequired = (sizeof(HID_COLLECTION) * deviceRptInfo.collections)
                   + (sizeof(HID_REPORTITEM) * deviceRptInfo.reportItems)
                   + (sizeof(HID_REPORT) * deviceRptInfo.reports)
                   + (sizeof(HID_USAGEITEM) * deviceRptInfo.usages)
                   + (sizeof(HID_STRINGITEM) * deviceRptInfo.strings)
                   + (sizeof(HID_DESIGITEM) * deviceRptInfo.designators)
                   + (sizeof(int) * deviceRptInfo.maxCollectionNesting)
                   + (sizeof(HID_GLOBALS) * deviceRptInfo.maxGlobalsNesting);

    if (parsedDataMem != NULL)
    {
		USB_FREE_AND_CLEAR( parsedDataMem );
    }

    parsedDataMem = (uint8_t*) USB_MALLOC(sizeRequired);
    
#ifdef DEBUG_MODE
    UART2PrintString( "HID: Memory for Report Descriptor: " );
    UART2PutHex( sizeRequired );
#endif

    if (parsedDataMem == NULL) return(HID_ERR_NotEnoughMemory); /* Error: Not enough memory */
    assignMem = (uint8_t*) parsedDataMem;
    
    /* Allocate Space */   
    itemListPtrs.collectionList = (HID_COLLECTION *) assignMem;
    assignMem += (sizeof(HID_COLLECTION) * deviceRptInfo.collections);
    itemListPtrs.reportItemList = (HID_REPORTITEM *) assignMem;
    assignMem += (sizeof(HID_REPORTITEM) * deviceRptInfo.reportItems);
    itemListPtrs.reportList = (HID_REPORT *) assignMem;
    assignMem += (sizeof(HID_REPORT) * deviceRptInfo.reports);
    itemListPtrs.usageItemList = (HID_USAGEITEM *) assignMem;
    assignMem += (sizeof(HID_USAGEITEM) * deviceRptInfo.usages);
    itemListPtrs.stringItemList = (HID_STRINGITEM *) assignMem;
    assignMem += (sizeof(HID_STRINGITEM) * deviceRptInfo.strings);
    itemListPtrs.designatorItemList = (HID_DESIGITEM *) assignMem;
    assignMem += (sizeof(HID_DESIGITEM) * deviceRptInfo.designators);
    itemListPtrs.collectionStack = (uint8_t *) assignMem;
    assignMem += (sizeof(int) * deviceRptInfo.maxCollectionNesting);
    itemListPtrs.globalsStack = (HID_GLOBALS *) assignMem;

    _USBHostHID_InitDeviceRptInfo();

//  Initialize the virtual collection

    collectionLocal = itemListPtrs.collectionList;
    collectionLocal->data = 0;
    collectionLocal->firstChild = 0;
    collectionLocal->firstReportItem = 0;
    collectionLocal->firstUsageItem = 0;
    collectionLocal->nextSibling = 0;
    collectionLocal->parent = 0;
    collectionLocal->reportItems = 0;
    collectionLocal->usageItems = 0;
    collectionLocal->usagePage = 0;

//  Initialize the default report

    reportLocal = itemListPtrs.reportList;
    reportLocal->featureBits = 0;
    reportLocal->inputBits = 0;
    reportLocal->outputBits = 0;
    reportLocal->reportID = 0;

   /* re-init ptr to Rpt Descp & Length of Descp */
   len_to_be_parsed = lengthOfDescriptor;
   currentRptDescPtr = hidReportDescriptor;

#ifdef DEBUG_MODE
    UART2PrintString( "HID-HOST: ... 2nd Parse \n" );
#endif


    while(len_to_be_parsed > 0)     /* Second parse to fill the tables with each item detail */
    {
       item.ItemDetails.val = *currentRptDescPtr;
       item.Data.uItemData = 0;

       ldataSize = item.ItemDetails.ItemSize ;
       if(item.ItemDetails.ItemSize == 3) 
           ldataSize = 4; 

       currentRptDescPtr++; /* ptr points to data */
       for (i = 0; i < ldataSize; i++)
        {
            dataByte = *currentRptDescPtr++;  /* signed data will be taken care in ItemTag it is expected */
            item.Data.uItemData |= ((uint32_t)dataByte << (i*8));
        }
    
       len_to_be_parsed -= (ldataSize+1);   /* remaining bytes = current - (length of current item + 1)*/

       switch(item.ItemDetails.ItemType)
        {
           case HIDType_Main:   /* look for Main Items*/
                switch(item.ItemDetails.ItemTag)
                {
                    case HIDTag_Input :
                    case HIDTag_Output :
                    case HIDTag_Feature :
                                lhidError = _USBHostHID_Parse_ReportType(&item);
                    break;

                    case HIDTag_Collection :
                                _USBHostHID_Parse_Collection(&item);
                    break;

                    case HIDTag_EndCollection :
                                _USBHostHID_Parse_EndCollection(&item);
                    break;
                }
                break;
                
           case HIDType_Global:   /* look for Global Items*/
                switch(item.ItemDetails.ItemTag)
                {
                    case HIDTag_UsagePage :
                         deviceRptInfo.globals.usagePage = item.Data.uItemData;
                         break;

                    case HIDTag_LogicalMinimum : /* convert to signed val */
                         //  Sign extend one value
                         _USBHostHID_ConvertDataToSigned(&item);
                        deviceRptInfo.globals.logicalMinimum = item.Data.sItemData;
                        break;                         

                    case HIDTag_LogicalMaximum :/* convert to signed val */
                         //  Sign extend one value
                         _USBHostHID_ConvertDataToSigned(&item);
                         deviceRptInfo.globals.logicalMaximum = item.Data.uItemData;
                         break;

                    case HIDTag_PhysicalMinimum :/* convert to signed val */
                         //  Sign extend one value
                         _USBHostHID_ConvertDataToSigned(&item);
                         deviceRptInfo.globals.physicalMinimum = item.Data.uItemData;
                        break;

                    case HIDTag_PhysicalMaximum :/* convert to signed val */
                         //  Sign extend one value
                         _USBHostHID_ConvertDataToSigned(&item);
                         deviceRptInfo.globals.physicalMaximum = item.Data.uItemData;
                         break;

                    case HIDTag_UnitExponent :
                         deviceRptInfo.globals.unitExponent = item.Data.uItemData;
                         break;

                    case HIDTag_ReportSize :
                         deviceRptInfo.globals.reportsize = item.Data.uItemData;
                         if (deviceRptInfo.globals.reportsize == 0)
                              lhidError = HID_ERR_ZeroReportSize;
                         break;

                    case HIDTag_ReportID :
                         if (item.Data.uItemData)
                            {
//                               Look for the Report ID in the table
                                                  
                                 lreportIndex = 0;
                                 while ((lreportIndex < deviceRptInfo.reports)
                                        && (itemListPtrs.reportList[lreportIndex].reportID != item.Data.uItemData))
                                        lreportIndex++;
                                 
//                               initialize the entry if it's new and there's room for it
//                               Start with 8 bits for the Report ID
                                 
                                 if (lreportIndex == deviceRptInfo.reports)
                                 {
                                    lreport = &itemListPtrs.reportList[deviceRptInfo.reports++];
                                    lreport->reportID = item.Data.uItemData;
                                    lreport->inputBits = 8;
                                    lreport->outputBits = 8;
                                    lreport->featureBits = 8;
                                 }
                                 
//                               remember which report is being processed
                                 
                                 deviceRptInfo.globals.reportID = item.Data.uItemData;
                                 deviceRptInfo.globals.reportIndex = lreportIndex;
                            }
                            else
                            {
                                 lhidError = HID_ERR_ZeroReportID;
                            }
                         break;

                    case HIDTag_ReportCount :
                         if (item.Data.uItemData)
                         {
                             deviceRptInfo.globals.reportCount = item.Data.uItemData;
                         }
                         else
                         {
                             lhidError = HID_ERR_ZeroReportCount;
                         }
                         break;

                    case HIDTag_Push :
                         itemListPtrs.globalsStack[deviceRptInfo.globalsNesting++] =  deviceRptInfo.globals;
                         break;

                    case HIDTag_Pop :
                         deviceRptInfo.globals = itemListPtrs.globalsStack[--deviceRptInfo.globalsNesting] ;
                    break;
            
                }
                break;

           case HIDType_Local:  /* look for Local Items*/
                switch(item.ItemDetails.ItemTag)
                {
                    case HIDTag_Usage :
                         lusageItem = &itemListPtrs.usageItemList[deviceRptInfo.usageItems++];
                         lusageItem->isRange = false;
                         if (item.ItemDetails.ItemSize == 3) /* 4 data bytes */
                            {
                                lusageItem->usagePage = item.Data.uItemData >> 16;
                                lusageItem->usage = item.Data.uItemData & 0x00FF;
                            }
                         else
                            {
                                lusageItem->usagePage = deviceRptInfo.globals.usagePage;
                                lusageItem->usage = item.Data.uItemData;
                            }
                         break;

                    case HIDTag_UsageMinimum :
                         if(deviceRptInfo.haveUsageMax)
                             {
                                lusageItem = &itemListPtrs.usageItemList[deviceRptInfo.usageItems++];
                                lusageItem->isRange = true;
                                if(item.ItemDetails.ItemSize == 3)
                                 {
                                    lusageItem->usagePage = item.Data.uItemData >> 16;
                                    lusageItem->usageMinimum = item.Data.uItemData & 0x00FFL;
                                 }
                                else
                                 {
                                    lusageItem->usagePage = deviceRptInfo.globals.usagePage;
                                    lusageItem->usageMinimum = item.Data.uItemData;
                                 }

                                 if (lusageItem->usagePage != deviceRptInfo.rangeUsagePage)
                                     lhidError = HID_ERR_BadUsageRangePage; /* Error: BadUsageRangePage */
                                    
                                 lusageItem->usageMaximum = deviceRptInfo.usageMaximum;
                                 
                                 if (lusageItem->usageMaximum < lusageItem->usageMinimum)
                                     lhidError = HID_ERR_BadUsageRange; /* Error: BadUsageRange */
                                 
                                 deviceRptInfo.haveUsageMax = false;
                                 deviceRptInfo.haveUsageMin = false;
                            }
                         else 
                            {
                                if(item.ItemDetails.ItemSize == 3)
                                {
                                    deviceRptInfo.rangeUsagePage = item.Data.uItemData >> 16;
                                    deviceRptInfo.usageMinimum = item.Data.uItemData & 0x00FFL;
                                }
                                else
                                {
                                    deviceRptInfo.rangeUsagePage = deviceRptInfo.globals.usagePage;
                                    deviceRptInfo.usageMinimum = item.Data.uItemData;
                                }
                                
                                deviceRptInfo.haveUsageMin = true;
                            }
                         break;

                    case HIDTag_UsageMaximum :
                         if(deviceRptInfo.haveUsageMin)
                             {
                                lusageItem = &itemListPtrs.usageItemList[deviceRptInfo.usageItems++];
                                lusageItem->isRange = true;
                                if(item.ItemDetails.ItemSize == 3)
                                 {
                                    lusageItem->usagePage = item.Data.uItemData >> 16;
                                    lusageItem->usageMaximum = item.Data.uItemData & 0x00FFL;
                                 }
                                else
                                 {
                                    lusageItem->usagePage = deviceRptInfo.globals.usagePage;
                                    lusageItem->usageMaximum = item.Data.uItemData;
                                 }

                                 if (lusageItem->usagePage != deviceRptInfo.rangeUsagePage)
                                     lhidError = HID_ERR_BadUsageRangePage; /* Error: BadUsageRangePage */
                                    
                                 lusageItem->usageMinimum = deviceRptInfo.usageMinimum;
                                 
                                 if (lusageItem->usageMaximum < lusageItem->usageMinimum)
                                     lhidError = HID_ERR_BadUsageRange; /* Error: BadUsageRange */
                                 
                                 deviceRptInfo.haveUsageMax = false;
                                 deviceRptInfo.haveUsageMin = false;
                            }
                         else 
                            {
                                if(item.ItemDetails.ItemSize == 3)
                                {
                                    deviceRptInfo.rangeUsagePage = item.Data.uItemData >> 16;
                                    deviceRptInfo.usageMaximum = item.Data.uItemData & 0x00FFL;
                                }
                                else
                                {
                                    deviceRptInfo.rangeUsagePage = deviceRptInfo.globals.usagePage;
                                    deviceRptInfo.usageMaximum = item.Data.uItemData;
                                }
                                
                                deviceRptInfo.haveUsageMax = true;
                            }
                         break;

                    case HIDTag_DesignatorIndex :
                         ldesignatorItem = &itemListPtrs.designatorItemList[deviceRptInfo.designatorItems++];
                         ldesignatorItem->isRange = false;
                         ldesignatorItem->index = item.Data.uItemData;

                         break;

                    case HIDTag_DesignatorMinimum :
                         if(deviceRptInfo.haveDesignatorMax)
                         {
                             ldesignatorItem = &itemListPtrs.designatorItemList[deviceRptInfo.designatorItems++];
                             ldesignatorItem->isRange = true;
                             ldesignatorItem->minimum = item.Data.uItemData;
                             ldesignatorItem->maximum = deviceRptInfo.designatorMaximum;
                             deviceRptInfo.haveDesignatorMin = false;
                             deviceRptInfo.haveDesignatorMax = false;
                         }
                         else
                         {
                             deviceRptInfo.designatorMinimum = item.Data.uItemData;
                             deviceRptInfo.haveDesignatorMin = true;
                         }
                         break;

                    case HIDTag_DesignatorMaximum :
                         if(deviceRptInfo.haveDesignatorMin)
                         {
                             ldesignatorItem = &itemListPtrs.designatorItemList[deviceRptInfo.designatorItems++];
                             ldesignatorItem->isRange = true;
                             ldesignatorItem->maximum = item.Data.uItemData;
                             ldesignatorItem->minimum = deviceRptInfo.designatorMinimum;
                             deviceRptInfo.haveDesignatorMin = false;
                             deviceRptInfo.haveDesignatorMax = false;
                         }
                         else
                         {
                             deviceRptInfo.designatorMaximum = item.Data.uItemData;
                             deviceRptInfo.haveDesignatorMax = true;
                         }
                         break;

                    case HIDTag_StringIndex :
                         lstringItem = &itemListPtrs.stringItemList[deviceRptInfo.stringItems++];
                         lstringItem->isRange = false;
                         lstringItem->index = item.Data.uItemData;
                         break;

                    case HIDTag_StringMinimum :
                         if (deviceRptInfo.haveStringMax) {
                             lstringItem = &itemListPtrs.stringItemList[deviceRptInfo.stringItems++];
                             lstringItem->isRange = true;
                             lstringItem->minimum = item.Data.uItemData;
                             lstringItem->maximum = deviceRptInfo.stringMaximum;
                             deviceRptInfo.haveStringMin = false;
                             deviceRptInfo.haveStringMax = false;
                         }
                         else {
                             deviceRptInfo.stringMinimum = item.Data.uItemData;
                             deviceRptInfo.haveStringMin = true;
                         }
                         break;

                    case HIDTag_StringMaximum :
                         if (deviceRptInfo.haveStringMin) {
                             lstringItem = &itemListPtrs.stringItemList[deviceRptInfo.stringItems++];
                             lstringItem->isRange = true;
                             lstringItem->maximum = item.Data.uItemData;
                             lstringItem->minimum = deviceRptInfo.stringMinimum;
                             deviceRptInfo.haveStringMin = false;
                             deviceRptInfo.haveStringMax = false;
                         }
                         else {
                             deviceRptInfo.stringMaximum = item.Data.uItemData;
                             deviceRptInfo.haveStringMax = true;
                         }
                         break;
                    break;

                    case HIDTag_SetDelimiter :
                    break;

                }
                
                break;

           default:
                break;
        }
        /* during 2nd parse if any anomaly is found in report format abort parsing and return */
        if(lhidError)
        {
            return(lhidError);
        }
    }

//  Update the virtual collection

//    itemListPtrs.collectionList[0].reportItems = deviceRptInfo.reportItems;

//  Remove reports that have just the report id

    for (i=1; i<deviceRptInfo.reports; i++) {
        if (itemListPtrs.reportList[i].inputBits == 8) itemListPtrs.reportList[i].inputBits = 0;
        if (itemListPtrs.reportList[i].outputBits == 8) itemListPtrs.reportList[i].outputBits = 0;
        if (itemListPtrs.reportList[i].featureBits == 8) itemListPtrs.reportList[i].featureBits = 0;
    }

    return(lhidError);
}

/****************************************************************************
  Function:
    static void _USBHostHID_InitDeviceRptInfo(void)

  Description:
    This function is called by _USBHostHID_Parse_Report() to Initialize
    report information to default value before every parse.  Note that not
    all values are reset.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
static void _USBHostHID_InitDeviceRptInfo(void)
{
    deviceRptInfo.collectionNesting = 0;
    deviceRptInfo.collections = 1;
    deviceRptInfo.designatorRanges = 0;
    deviceRptInfo.designators = 0;
    deviceRptInfo.globalsNesting = 0;
    deviceRptInfo.maxCollectionNesting = 0;
    deviceRptInfo.maxGlobalsNesting = 0;
    deviceRptInfo.reportItems = 0;
    deviceRptInfo.reports = 1;
    deviceRptInfo.stringRanges = 0;
    deviceRptInfo.strings = 0;
    deviceRptInfo.usages = 0;
    deviceRptInfo.usageRanges = 0;
    deviceRptInfo.usageItems = 0;
    
    deviceRptInfo.haveDesignatorMax = false;
    deviceRptInfo.haveDesignatorMin = false;
    deviceRptInfo.haveStringMax = false;
    deviceRptInfo.haveStringMin = false;
    deviceRptInfo.haveUsageMax = false;
    deviceRptInfo.haveUsageMin = false;

    deviceRptInfo.globals.logicalMaximum = 0;
    deviceRptInfo.globals.logicalMinimum = 0;
    deviceRptInfo.globals.physicalMaximum = 0;
    deviceRptInfo.globals.physicalMinimum = 0;
    deviceRptInfo.globals.reportCount = 0;
    deviceRptInfo.globals.reportID = 0;
    deviceRptInfo.globals.reportIndex = 0;
    deviceRptInfo.globals.reportsize = 0;
    deviceRptInfo.globals.unit = 0;
    deviceRptInfo.globals.unitExponent = 0;
    deviceRptInfo.globals.usagePage = 0;
}


/****************************************************************************
  Function:
    static void _USBHostHID_Parse_Collection(HID_ITEM_INFO* ptrItem)

  Description:
    This function is called by _USBHostHID_Parse_Report() to parse 
    collection item.

  Precondition:
    None

  Parameters:
    HID_ITEM_INFO* ptrItem - pointer to item structure containg raw
                             information from the report
  Return Values:
    None

  Remarks:
    None
***************************************************************************/
static void _USBHostHID_Parse_Collection(HID_ITEM_INFO* ptrItem)
{
    HID_COLLECTION *lcollection;
    uint16_t i;

//  Initialize the new Collection Structure

    i = deviceRptInfo.collections++;
    lcollection = &itemListPtrs.collectionList[i];
    lcollection->data = ptrItem->Data.uItemData;
    lcollection->firstUsageItem = deviceRptInfo.firstUsageItem;
    lcollection->usageItems = deviceRptInfo.usageItems - deviceRptInfo.firstUsageItem;
    deviceRptInfo.firstUsageItem = deviceRptInfo.usageItems;
    lcollection->nextSibling = deviceRptInfo.sibling;
    deviceRptInfo.sibling = 0;
    lcollection->firstChild = 0;
    lcollection->usagePage = deviceRptInfo.globals.usagePage;
    lcollection->firstReportItem = deviceRptInfo.reportItems;

//  Set up the relationship with the Parent Collection

    lcollection->parent = deviceRptInfo.parent;
    itemListPtrs.collectionList[deviceRptInfo.parent].firstChild = i;

//  Save the Parent Collection Information on the stack
    itemListPtrs.collectionStack[deviceRptInfo.collectionNesting++] = deviceRptInfo.parent;
    deviceRptInfo.parent = i;
}

/****************************************************************************
  Function:
    static void _USBHostHID_Parse_EndCollection(HID_ITEM_INFO* ptrItem)

  Description:
    This function is called by _USBHostHID_Parse_Report() to parse end of
    collection item.

  Precondition:
    None

  Parameters:
    HID_ITEM_INFO* ptrItem - pointer to item structure containg raw
                             information from the report
  Return Values:
    None

  Remarks:
    None
***************************************************************************/
static void _USBHostHID_Parse_EndCollection(HID_ITEM_INFO* ptrItem)
{
    HID_COLLECTION *lcollection;
    uint8_t i;

//  Remember the number of reportItem MainItems in this Collection

    lcollection = &itemListPtrs.collectionList[deviceRptInfo.parent];
    lcollection->reportItems = deviceRptInfo.reportItems - lcollection->firstReportItem;

//  Restore the Parent Collection Data

    i = itemListPtrs.collectionStack[--deviceRptInfo.collectionNesting];
    deviceRptInfo.sibling = deviceRptInfo.parent;
    deviceRptInfo.parent = i;
}

/****************************************************************************
  Function:
    static USB_HID_RPT_DESC_ERROR _USBHostHID_Parse_ReportType(HID_ITEM_INFO* item)

  Description:
    This function is called by _USBHostHID_Parse_Report() to parse
    input, output & report item.

  Precondition:
    None

  Parameters:
    HID_ITEM_INFO* ptrItem - pointer to item structure containg raw
                             information from the report

  Return Values:
    USB_HID_RPT_DESC_ERROR - Returns error code if any error is encountered
                             in report descriptor.

  Remarks:
    None
***************************************************************************/
static USB_HID_RPT_DESC_ERROR _USBHostHID_Parse_ReportType(HID_ITEM_INFO* item)
{
    HID_REPORTITEM *lreportItem = NULL;
    HID_REPORT *lreport = NULL;
    uint16_t bits = 0;

    if(item == NULL)
        return(HID_ERR_NullPointer);
   
//  Reality Check on the Report Main Item

    if (deviceRptInfo.globals.logicalMinimum >= ((int32_t)1<<deviceRptInfo.globals.reportsize)) return(HID_ERR_BadLogicalMin) ;
    if (deviceRptInfo.globals.logicalMaximum >= ((int32_t)1<<deviceRptInfo.globals.reportsize))return(HID_ERR_BadLogicalMax);
    // The barcode scanner has this issue.  We'll ignore it.
	// if (deviceRptInfo.globals.logicalMinimum > deviceRptInfo.globals.logicalMaximum)return(HID_ERR_BadLogical); 
    if (deviceRptInfo.haveUsageMin || deviceRptInfo.haveUsageMax)return(HID_ERR_UnmatchedUsageRange);
    if (deviceRptInfo.haveStringMin || deviceRptInfo.haveStringMax)return(HID_ERR_UnmatchedStringRange);
    if (deviceRptInfo.haveDesignatorMin || deviceRptInfo.haveDesignatorMax)return(HID_ERR_UnmatchedDesignatorRange);

//  Initialize the new Report Item structure

    lreportItem = &itemListPtrs.reportItemList[deviceRptInfo.reportItems++];
    lreportItem->dataModes = item->Data.uItemData;
    lreportItem->globals = deviceRptInfo.globals;
    lreportItem->parent = deviceRptInfo.parent;
    lreportItem->firstUsageItem = deviceRptInfo.firstUsageItem;
    deviceRptInfo.firstUsageItem = deviceRptInfo.usageItems;
    lreportItem->usageItems = deviceRptInfo.usageItems - lreportItem->firstUsageItem;
    lreportItem->firstStringItem = deviceRptInfo.firstStringItem;
    deviceRptInfo.firstStringItem = deviceRptInfo.stringItems;
    lreportItem->stringItems = deviceRptInfo.stringItems - lreportItem->firstStringItem;
    lreportItem->firstDesignatorItem = deviceRptInfo.firstDesignatorItem;
    deviceRptInfo.firstDesignatorItem = deviceRptInfo.designatorItems;
    lreportItem->designatorItems = deviceRptInfo.designatorItems - lreportItem->firstDesignatorItem;

//  Update the Report by the size of this item

    lreport = &itemListPtrs.reportList[deviceRptInfo.globals.reportIndex];
    bits = deviceRptInfo.globals.reportsize * deviceRptInfo.globals.reportCount;
    switch (item->ItemDetails.ItemTag) 
    {
        case HIDTag_Feature:
            lreportItem->reportType = hidReportFeature;
            lreportItem->startBit = lreport->featureBits;
            lreport->featureBits += bits;
            break;
        case HIDTag_Output:
            lreportItem->reportType = hidReportOutput;
            lreportItem->startBit = lreport->outputBits;
            lreport->outputBits += bits;
            break;
        case HIDTag_Input:
            lreportItem->reportType = hidReportInput;
            lreportItem->startBit = lreport->inputBits;
            lreport->inputBits += bits;
            break;
        default:
            lreportItem->reportType = hidReportUnknown;
            break;
    }

    return HID_ERR;
}

/****************************************************************************
  Function:
    static void _USBHostHID_ConvertDataToSigned(HID_ITEM_INFO* item)

  Description:
    This function is called by _USBHostHID_Parse_Report() convert data
    to signed whenever required

  Precondition:
    None

  Parameters:
    HID_ITEM_INFO* ptrItem - pointer to item structure containg raw
                             information from the report

  Return Values:
    None

  Remarks:
    None
***************************************************************************/
static void _USBHostHID_ConvertDataToSigned(HID_ITEM_INFO* item)
{
    uint8_t    dataByte=0;
    uint8_t    index;
 
    index = item->ItemDetails.ItemSize;

    if(index)
    {
       if(item->ItemDetails.ItemSize == 3) 
           index = 4;

       dataByte = item->Data.bItemData[index-1];
       if ((dataByte & 0x80) != 0)
       {
           while (index < sizeof(int32_t))
                item->Data.sItemData |= (0xFF << ((index++)*8)); /* extend one */
       }
    }
}


/****************************************************************************
  Function:
    bool USBHostHID_HasUsage(HID_REPORTITEM *reportItem, uint16_t usagePage,
                                          uint16_t usage, uint16_t *pindex, uint8_t* count)

  Description:
    This function is used to locate the usage in a report descriptor.
    Function will look into the data structures created by the HID parser
    and return the appropriate location.

  Precondition:
    None

  Parameters:
    HID_REPORTITEM *reportItem - Report item index to be searched
    uint16_t usagePage             - Application needs to pass the usagePage as
                                 the search criteria for the usage
    uint16_t usage                 - Application needs to pass the usageto be
                                 searched
    uint16_t *pindex               - returns index to the usage item requested.
    uint8_t* count                - returns the remaining number of reports

  Return Values:
    bool                       - false - If requested usage is not found
                                 true  - if requested usage is found
  Remarks:
    None
***************************************************************************/
bool USBHostHID_HasUsage(HID_REPORTITEM *reportItem,uint16_t usagePage, uint16_t usage,uint16_t *pindex, uint8_t* count)
{
    HID_USAGEITEM *hidUsageItem;
    uint16_t usageIndex;
    int16_t usages;
    bool onPage;
    uint8_t usageItem;
    uint8_t countsLeft;
    uint8_t i;

//  Disallow Null Pointers

    if ((reportItem == NULL)|(pindex == NULL))
        return false;

//  Look through the Usage Items for this Usage

    usageItem = reportItem->firstUsageItem;
    usageIndex = 0;
    for (i=0; i<reportItem->usageItems; i++) {
//     Each Usage Item is either a Usage or a Usage Range

        hidUsageItem = &itemListPtrs.usageItemList[usageItem++];
        onPage = ((usagePage == 0) || (usagePage == hidUsageItem->usagePage));
        if (hidUsageItem->isRange) 
            {
//          For Usage Ranges
//            If the index is in the range
//              then return the Usage
//            Otherwise adjust the index by the size of the range

            if ((usage >= hidUsageItem->usageMinimum)&& (usage <= hidUsageItem->usageMaximum))
                {
                   if (pindex != NULL)
                      *pindex = usageIndex + (usage - hidUsageItem->usageMinimum);

//              If this usage is the last one for this reportItem
//                then it gets all of the remaining ReportCount

                    if (count != NULL)
                        {
                            if (((i+1) == reportItem->usageItems)&& (usage == hidUsageItem->usageMaximum))
                            {
                                countsLeft = reportItem->globals.reportCount - usageIndex;
                                if (countsLeft > 1)
                                    *count = countsLeft;
                                else
                                    *count = 1;
                            }
                            else
                                *count = 1;
                        }
                     if(onPage)
                         return true;
                }
                usages = hidUsageItem->usageMaximum - hidUsageItem->usageMinimum + 1;
                if (usages < 0) usages = -usages;
                usageIndex += usages;
            }
        else 
            {
//          For Usages
//          If the index is zero
//            then return this Usage
//          Otherwise one less to index through

            if (usage == hidUsageItem->usage)
                {
                    if (pindex != NULL)
                        *pindex = usageIndex;
                    if (count != NULL)
                    {
                        if ((i+1) == reportItem->usageItems)
                        {
                            countsLeft = reportItem->globals.reportCount - usageIndex;
                            if (countsLeft > 1)
                                *count = countsLeft;
                            else
                                *count = 1;
                        }
                        else
                            *count = 1;
                  }
                    if (onPage)
                        return true;
                }
            usageIndex++;
        }
    }
    return false;
}

#ifdef DEBUG_MODE
void USBHID_ReportDecriptor_Dump(void)
{
    HID_COLLECTION *lcollection;
    HID_REPORT *lreport;
    HID_REPORTITEM *lreportItem;
    HID_USAGEITEM *lusageItem;
    uint16_t j;
    uint8_t i;


    UART2PrintString("\r\n ======================= Report Descriptor Dump ======================= \n\n");

    UART2PrintString("\r\nCollections:  ");
    UART2PutHex( deviceRptInfo.collections );

    UART2PrintString("\r\nMax Nesting:  ");
    UART2PutHex( deviceRptInfo.maxCollectionNesting );

    UART2PrintString("\r\nReports:  ");
    UART2PutHex( deviceRptInfo.reports );

    UART2PrintString("\r\nReportItems:  ");
    UART2PutHex( deviceRptInfo.reportItems );

    UART2PrintString("\r\nUsageItems:   ");
    UART2PutHex( deviceRptInfo.usageItems );

        for (i=0;i<deviceRptInfo.collections;i++)
        {
            UART2PrintString("\n\r------------------------\n");
            UART2PrintString("\r\nCollection      : ");UART2PutHex(i);
            lcollection = &itemListPtrs.collectionList[i];
            UART2PrintString("\r\n    Data            : ");
            UART2PutHex( lcollection->data);
            UART2PrintString("\r\n    Usage Page      : ");
            UART2PutHex( lcollection->usagePage);
            UART2PrintString("\r\n    1st Usage Item  : ");
            UART2PutHex( lcollection->firstUsageItem);
            UART2PrintString("\r\n    # Usage Items   : ");
            UART2PutHex( lcollection->usageItems);
            UART2PrintString("\r\n    1st Report Item : ");
            UART2PutHex( lcollection->firstReportItem);
            UART2PrintString("\r\n    # Report Items  : ");
            UART2PutHex( lcollection->reportItems);
            UART2PrintString("\r\n    Parent          : ");
            UART2PutHex( lcollection->parent);
            UART2PrintString("\r\n    1st Child       : ");
            UART2PutHex( lcollection->firstChild);
            UART2PrintString("\r\n    Next Sibling    : ");
            UART2PutHex( lcollection->nextSibling);
        }


    for (i=0; i<deviceRptInfo.reports; i++)
        {
        UART2PrintString("\n------------------------\n");
        UART2PrintString("\r\nReport   : ");UART2PutHex(i);
        lreport = &itemListPtrs.reportList[i];
        UART2PrintString("\r\nReportID : ");UART2PutHex(lreport->reportID);
        if (lreport->inputBits > 0)
            UART2PrintString("\r\nInbits   : ");UART2PutHex(lreport->inputBits);
        if (lreport->outputBits > 0)
            UART2PrintString("\r\nOutbits  : ");UART2PutHex(lreport->outputBits);
        if (lreport->featureBits > 0)
            UART2PrintString("\r\nFeatbits : ");UART2PutHex(lreport->featureBits);
        }
    for (i=0; i<deviceRptInfo.reportItems; i++) {
//        getchar();
        UART2PrintString("\n------------------------\n");
        UART2PrintString("\r\nReportItem  : ");UART2PutHex(i);
        lreportItem = &itemListPtrs.reportItemList[i];
        UART2PrintString("\r\nReportType  : ");UART2PutHex(lreportItem->reportType);
//    HIDGlobals            globals;
        UART2PrintString("\r\nStart Bit   : ");UART2PutHex(lreportItem->startBit);
        UART2PrintString("\r\nBits        : ");UART2PutHex(lreportItem->globals.reportsize);
        UART2PrintString("\r\nParent      : ");UART2PutHex(lreportItem->parent);
        UART2PrintString("\r\nDataModes   : ");UART2PutHex(lreportItem->dataModes);
        UART2PrintString("\r\n1st Usage   : ");UART2PutHex(lreportItem->firstUsageItem);
        UART2PrintString("\r\nUsage Items : ");UART2PutHex(lreportItem->usageItems);
        UART2PrintString("\r\n      Globals   ");
        UART2PrintString("\r\nusagePage   : ");UART2PutHex(lreportItem->globals.usagePage);
        UART2PrintString("\r\nreportsize   : ");UART2PutHex(lreportItem->globals.reportsize);
        UART2PrintString("\r\nreportID   : ");UART2PutHex(lreportItem->globals.reportID);
        UART2PrintString("\r\nreportCount   : ");UART2PutHex(lreportItem->globals.reportCount);
    
    }

        UART2PrintString("\n------------------------\n");
        UART2PrintString("\r\nUsageItem  : ");UART2PutHex(i);
    for (i=0; i<deviceRptInfo.usageItems; i++)
        {

        if (itemListPtrs.usageItemList[i].isRange)
            {
                UART2PrintString("\r\nUsages     Minimum      Maximum   \n\r");
                UART2PrintString("\t\t");
                j = itemListPtrs.usageItemList[i].usageMaximum;
                UART2PutHex(j); 
                UART2PrintString("\t");
                j = itemListPtrs.usageItemList[i].usageMinimum;
                UART2PutHex(j);
            }
        else
            UART2PrintString("\r\nUsage: ");UART2PutHex(itemListPtrs.usageItemList[i].usage);
        }
}
#endif

