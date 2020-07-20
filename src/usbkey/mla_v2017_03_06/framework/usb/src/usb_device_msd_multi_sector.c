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
 
/** I N C L U D E S **************************************************/
#include <usb.h>
#include "system.h"
#include "system_config.h"

#include "usb_device_msd.h"

#ifdef USB_USE_MSD

#if MAX_LUN == 0
    #define LUN_INDEX 0
#else
    #define LUN_INDEX gblCBW.bCBWLUN
#endif

#if defined(USE_INTERNAL_FLASH)
    #include "internal_flash.h"
#endif

#if defined(USE_SD_INTERFACE_WITH_SPI)
    #include "sd_spi.h"
#endif

extern LUN_FUNCTIONS LUN[MAX_LUN + 1];
#define LUNMediaInitialize()                LUN[LUN_INDEX].MediaInitialize(LUN[LUN_INDEX].mediaParameters)
#define LUNReadCapacity()                   LUN[LUN_INDEX].ReadCapacity(LUN[LUN_INDEX].mediaParameters)
#define LUNReadSectorSize()                 LUN[LUN_INDEX].ReadSectorSize(LUN[LUN_INDEX].mediaParameters)
#define LUNMediaDetect()                    LUN[LUN_INDEX].MediaDetect(LUN[LUN_INDEX].mediaParameters)
#define LUNSectorWrite(bLBA,pDest,Write0)   LUN[LUN_INDEX].SectorWrite(LUN[LUN_INDEX].mediaParameters, bLBA, pDest, Write0)
#define LUNWriteProtectState()              LUN[LUN_INDEX].WriteProtectState(LUN[LUN_INDEX].mediaParameters)
#define LUNSectorRead(bLBA,pSrc)            LUN[LUN_INDEX].SectorRead(LUN[LUN_INDEX].mediaParameters, bLBA, pSrc)
#define LUNAsyncWriteTasks(pAsyncIO)        LUN[LUN_INDEX].AsyncWriteTasks(LUN[LUN_INDEX].mediaParameters, pAsyncIO)
#define LUNAsyncReadTasks(pAsyncIO)         LUN[LUN_INDEX].AsyncReadTasks(LUN[LUN_INDEX].mediaParameters, pAsyncIO)


//Adjustable user options
#define MSD_FAILED_READ_MAX_ATTEMPTS  (uint8_t)100u    //Used for error case handling
#define MSD_FAILED_WRITE_MAX_ATTEMPTS (uint8_t)100u    //Used for error case handling

/** V A R I A B L E S ************************************************/
#if defined(__18CXX)
    #pragma udata
#endif

//State machine variables
uint8_t MSD_State;			// Takes values MSD_WAIT, MSD_DATA_IN or MSD_DATA_OUT
uint8_t MSDCommandState;
uint8_t MSDReadState;
uint8_t MSDWriteState;
uint8_t MSDRetryAttempt;
//Other variables
USB_MSD_CBW gblCBW;	
uint8_t gblCBWLength;
RequestSenseResponse gblSenseData[MAX_LUN + 1];
uint8_t *ptrNextData;
USB_HANDLE USBMSDOutHandle;
USB_HANDLE USBMSDInHandle;
uint16_t MSBBufferIndex;
uint16_t gblMediaPresent; 
bool SoftDetach[MAX_LUN + 1];
bool MSDHostNoData;
bool MSDCBWValid;

static USB_MSD_TRANSFER_LENGTH TransferLength;
static USB_MSD_LBA LBA;
FILEIO_SD_ASYNC_IO AsyncReadWriteInfo;
uint8_t fetchStatus;

/* 
 * Number of Blocks and Block Length are global because 
 * for every READ_10 and WRITE_10 command need to verify if the last LBA 
 * is less than gblNumBLKS	
 */	
USB_MSD_BLK gblNumBLKS,gblBLKLen;
extern const InquiryResponse inq_resp;

/** P R I V A T E  P R O T O T Y P E S ***************************************/
uint8_t MSDProcessCommand(void);
void MSDProcessCommandMediaAbsent(void);
void MSDProcessCommandMediaPresent(void);
uint8_t MSDReadHandler(void);
uint8_t MSDWriteHandler(void);
void ResetSenseData(void);
uint8_t MSDCheckForErrorCases(uint32_t);
void MSDErrorHandler(uint8_t);
static void MSDComputeDeviceInAndResidue(uint16_t);

/** D E C L A R A T I O N S **************************************************/
#if defined(__18CXX)
    #pragma code
#endif

/** C L A S S  S P E C I F I C  R E Q ****************************************/

/******************************************************************************
  Function:
    void USBMSDInit(void)
    
  Summary:
    This routine initializes the MSD class packet handles, prepares to
    receive a MSD packet, and initializes the MSD state machine. This
    \function should be called once after the device is enumerated.

  Description:
    This routine initializes the MSD class packet handles, prepares to
    receive a MSD packet, and initializes the MSD state machine. This
    \function should be called once after the device is enumerated.
    
    Typical Usage:
    <code>
    void USBCBInitEP(void)
    {
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USBMSDInit();
    }
    </code>
  Conditions:
    The device should already be enumerated with a configuration that
    supports MSD before calling this function.
    
  Paramters: None

  Remarks:
    None                                                                                                          
  ****************************************************************************/	
void USBMSDInit(void)
{
    //Prepare to receive the first CBW
    USBMSDInHandle = 0;    
    USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(uint8_t*)&msd_cbw,MSD_OUT_EP_SIZE);
    //Initialize IN handle to point to first available IN MSD bulk endpoint entry
    USBMSDInHandle = USBGetNextHandle(MSD_DATA_IN_EP, IN_TO_HOST);
    MSD_State = MSD_WAIT;
    MSDCommandState = MSD_COMMAND_WAIT;
    MSDReadState = MSD_READ10_WAIT;
    MSDWriteState = MSD_WRITE10_WAIT;
    MSDHostNoData = false;
    gblNumBLKS.Val = 0;
    gblBLKLen.Val = 0;
    MSDCBWValid = true;

    gblMediaPresent = 0;

    //For each of the possible logical units
    for(gblCBW.bCBWLUN=0;gblCBW.bCBWLUN<(MAX_LUN + 1);gblCBW.bCBWLUN++)
    {
        //clear all of the soft detach variables
        SoftDetach[gblCBW.bCBWLUN] =  false;

        //see if the media is attached
        if(LUNMediaDetect())
        {
            //initialize the media
            if(LUNMediaInitialize())
            {
                //if the media was present and successfully initialized
                //  then mark and indicator that the media is ready
                gblMediaPresent |= ((uint16_t)1<<gblCBW.bCBWLUN);
            }
        }
        ResetSenseData();
    }
}

/******************************************************************************
 	Function:
 		void USBCheckMSDRequest(void)

 	Summary:
 		This routine handles MSD specific request that happen on EP0.  
        This function should be called from the USBCBCheckOtherReq() call back 
        function whenever implementing an MSD device.

 	Description:
 		This routine handles MSD specific request that happen on EP0.  These
        include, but are not limited to, the standard RESET and GET_MAX_LUN 
 		command requests.  This function should be called from the 
        USBCBCheckOtherReq() call back function whenever using an MSD device.	

        Typical Usage:
        <code>
        void USBCBCheckOtherReq(void)
        {
            //Since the stack didn't handle the request I need to check
            //  my class drivers to see if it is for them
            USBCheckMSDRequest();
        }
        </code>

 	PreCondition:
 		None
 		
 	Parameters:
 		None
 	
 	Return Values:
 		None
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
void USBCheckMSDRequest(void)
{
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD)
    {
        return;
    }
    
    if(SetupPkt.bIntfID != MSD_INTF_ID) 
    {
        return;
    }

    switch(SetupPkt.bRequest)
    {
        case MSD_RESET:
            //First make sure all request parameters are correct:
            //MSD BOT specs require wValue to be == 0x0000 and wLength == 0x0000
            if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 0))
            {
                return; //Return without handling the request (results in STALL)
            }

            //Host would typically issue this after a STALL event on an MSD
            //bulk endpoint.  The MSD reset should re-initialize status
            //so as to prepare for a new CBW.  Any currently ongoing command
            //block should be aborted, but the STALL and DTS states need to be
            //maintained (host will re-initialize these seperately using
            //CLEAR_FEATURE, endpoint halt).
            MSD_State = MSD_WAIT;
            MSDCommandState = MSD_COMMAND_WAIT;
            MSDReadState = MSD_READ10_WAIT;
            MSDWriteState = MSD_WRITE10_WAIT;
            MSDCBWValid = true;
            //Need to re-arm MSD bulk OUT endpoint, if it isn't currently armed,
            //to be able to receive next CBW.  If it is already armed, don't need
            //to do anything, since we can already receive the next CBW (or we are
            //STALLed, and the host will issue clear halt first).
            if(!USBHandleBusy(USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST)))
            {
                USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(uint8_t*)&msd_cbw,MSD_OUT_EP_SIZE);
            }

            //Let USB stack know we took care of handling the EP0 SETUP request.
            //Allow zero byte status stage to proceed normally now.
            USBEP0Transmit(USB_EP0_NO_DATA);
            break;
            
        case GET_MAX_LUN:
            //First make sure all request parameters are correct:
            //MSD BOT specs require wValue to be == 0x0000, and wLengh == 1
            if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 1))
            {
                break;  //Return without handling the request (results in STALL)
            }

            //If the host asks for the maximum number of logical units
            //  then send out a packet with that information
            CtrlTrfData[0] = MAX_LUN;
            USBEP0SendRAMPtr((uint8_t*)&CtrlTrfData[0],1,USB_EP0_INCLUDE_ZERO);
            break;
    }	//end switch(SetupPkt.bRequest)
}

/*********************************************************************************
  Function:
        uint8_t MSDTasks(void)
    
  Summary:
    This function runs the MSD class state machines and all of its
    sub-systems. This function should be called periodically once the
    device is in the configured state in order to keep the MSD state
    machine going.
  Description:
    This function runs the MSD class state machines and all of its
    sub-systems. This function should be called periodically once the
    device is in the configured state in order to keep the MSD state
    machine going.
    
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
                //Keep the MSD state machine going
                MSDTasks();
    
                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    None
  Return Values:
    uint8_t -  the current state of the MSD state machine the valid values are
            defined in MSD.h under the MSDTasks state machine declaration section.
            The possible values are the following\:
            * MSD_WAIT
            * MSD_DATA_IN
            * MSD_DATA_OUT
            * MSD_SEND_CSW
  Remarks:
    None                                                                          
  *********************************************************************************/	
uint8_t MSDTasks(void)
{
    uint8_t i;
    
    //Error check to make sure we have are in the CONFIGURED_STATE, prior to
    //performing MSDTasks().  Some of the MSDTasks require that the device be
    //configured first.
    if(USBGetDeviceState() != CONFIGURED_STATE)
    {
        return MSD_WAIT;
    }
    
    //Note: Both the USB stack code (usb_device.c) and this MSD handler code 
    //have the ability to modify the BDT values for the MSD bulk endpoints.  If the 
    //USB stack operates in USB_INTERRUPT mode (user option in usb_config.h), we
    //should temporily disable USB interrupts, to avoid any possibility of both 
    //the USB stack and this MSD handler from modifying the same BDT entry, or
    //MSD state machine variables (ex: in the case of MSD_RESET) at the same time.
    USBMaskInterrupts();
    
    //Main MSD task dispatcher.  Receives MSD Command Block Wrappers (CBW) and
    //dispatches appropriate lower level handlers to service the requests.
    switch(MSD_State)
    {
        case MSD_WAIT: //idle state, when we are waiting for a command from the host
        {
            //Check if we have received a new command block wrapper (CBW)
            if(!USBHandleBusy(USBMSDOutHandle))
            {
                //If we are in the MSD_WAIT state, and we received an OUT transaction
                //on the MSD OUT endpoint, then we must have just received an MSD
                //Command Block Wrapper (CBW).
                //First copy the the received data to to the gblCBW structure, so
                //that we keep track of the command, but free up the MSD OUT endpoint
                //buffer for fulfilling whatever request may have been received.
                //gblCBW = msd_cbw; //we are doing this, but below method can yeild smaller code size
                for(i = 0; i < MSD_CBW_SIZE; i++)
                {
                    *((uint8_t*)&gblCBW.dCBWSignature + i) = *((uint8_t*)&msd_cbw.dCBWSignature + i);
                }

                //If this CBW is valid?
                if((USBHandleGetLength(USBMSDOutHandle) == MSD_CBW_SIZE) && (gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE))
                {
                    //The CBW was valid, set flag meaning any stalls after this point
                    //should not be "persistent" (as in the case of non-valid CBWs).
                    MSDCBWValid = true;

                    //Is this CBW meaningful?
                    if((gblCBW.bCBWLUN <= MAX_LUN)                                      //Verify the command is addressed to a supported LUN
                        &&(gblCBW.bCBWCBLength <= MSD_MAX_CB_SIZE)                          //Verify the claimed CB length is reasonable/valid
                        &&(gblCBW.bCBWCBLength >= 0x01)                                     //Verify the claimed CB length is reasonable/valid
                        &&((gblCBW.bCBWFlags & MSD_CBWFLAGS_RESERVED_BITS_MASK) == 0x00))   //Verify reserved bits are clear
                    {

                        //The CBW was both valid and meaningful.
                        //Begin preparing a valid Command Status Wrapper (CSW),
                        //in anticipation of completing the request successfully.
                        //If an error detected is later, we will change the status
                        //before sending the CSW.
                        msd_csw.dCSWSignature = MSD_VALID_CSW_SIGNATURE;
                        msd_csw.dCSWTag = gblCBW.dCBWTag;
                        msd_csw.dCSWDataResidue = 0x0;
                        msd_csw.bCSWStatus = MSD_CSW_COMMAND_PASSED;

                        //Since a new CBW just arrived, we should re-init the
                        //lower level state machines to their default states.
                        //Even if the prior operation didn't fully complete
                        //normally, we should abandon the prior operation, when
                        //a new CBW arrives.
                        MSDCommandState = MSD_COMMAND_WAIT;
                        MSDReadState = MSD_READ10_WAIT;
                        MSDWriteState = MSD_WRITE10_WAIT;

                        //Keep track of retry attempts, in case of temporary
                        //failures during read or write of the media.
                        MSDRetryAttempt = 0;

                        //Check the command.  With the exception of the REQUEST_SENSE
                        //command, we should reset the sense key info for each new command block.
                        //Assume the command will get processed successfully (and hence "NO SENSE"
                        //response, which is used for success cases), unless handler code
                        //later on detects some kind of error.  If it does, it should
                        //update the sense keys to reflect the type of error detected,
                        //prior to sending the CSW.
                        if(gblCBW.CBWCB[0] != MSD_REQUEST_SENSE)
                        {
                            gblSenseData[LUN_INDEX].SenseKey=S_NO_SENSE;
                            gblSenseData[LUN_INDEX].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
                            gblSenseData[LUN_INDEX].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
                        }

                        //Isolate the data direction bit.  The direction bit is bit 7 of the bCBWFlags byte.
                        //Then, based on the direction of the data transfer, prepare the MSD state machine
                        //so it knows how to proceed with processing the request.
                        //If bit7 = 0, then direction is OUT from host.  If bit7 = 1, direction is IN to host
                        if (gblCBW.bCBWFlags & MSD_CBW_DIRECTION_BITMASK)
                        {
                            MSD_State = MSD_DATA_IN;
                        }
                        else //else direction must be OUT from host
                        {
                            MSD_State = MSD_DATA_OUT;
                        }

                        //Determine if the host is expecting there to be data transfer or not.
                        //Doing this now will make for quicker error checking later.
                        if(gblCBW.dCBWDataTransferLength != 0)
                        {
                            MSDHostNoData = false;
                        }
                        else
                        {
                            MSDHostNoData = true;
                        }

                        //Copy the received command to the lower level command
                        //state machine, so it knows what to do.
                        MSDCommandState = gblCBW.CBWCB[0];
                    }
                    else
                    {
                        //else the CBW wasn't meaningful.  Section 6.4 of BOT specs v1.0 says,
                        //"The response of a device to a CBW that is not meaningful is not specified."
                        //Lets STALL the bulk endpoints, so as to promote the possibility of recovery.
                        USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
                        USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
                    }
                }//end of: if((USBHandleGetLength(USBMSDOutHandle) == MSD_CBW_SIZE) && (gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE))
                else  //The CBW was not valid.
                {
                    //Section 6.6.1 of the BOT specifications rev. 1.0 says the device shall STALL bulk IN and OUT
                    //endpoints (or should discard OUT data if not stalled), and should stay in this state until a
                    //"Reset Recovery" (MSD Reset + clear endpoint halt commands on EP0, see section 5.3.4)
                    USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
                    USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
                    MSDCBWValid = false;    //Flag so as to enable a "persistent"
                    //stall (cannot be cleared by clear endpoint halt, unless preceded
                    //by an MSD reset).
                }
            }//if(!USBHandleBusy(USBMSDOutHandle))
            break;
        }//end of: case MSD_WAIT:
        case MSD_DATA_IN:
            if(MSDProcessCommand() == MSD_COMMAND_WAIT)
            {
                // Done processing the command, send the status
                MSD_State = MSD_SEND_CSW;
            }
            break;
        case MSD_DATA_OUT:
            if(MSDProcessCommand() == MSD_COMMAND_WAIT)
            {
                /* Finished receiving the data prepare and send the status */
                if ((msd_csw.bCSWStatus == MSD_CSW_COMMAND_PASSED)&&(msd_csw.dCSWDataResidue!=0))
                {
                    msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
                }
                MSD_State = MSD_SEND_CSW;
            }
            break;
        case MSD_SEND_CSW:
            //Check to make sure the bulk IN endpoint is available before sending CSW.
            //The endpoint might still be busy sending the last packet on the IN endpoint.
            if(USBHandleBusy(USBGetNextHandle(MSD_DATA_IN_EP, IN_TO_HOST)) == true)
            {
                break;  //Not available yet.  Just stay in this state and try again later.
            }
            
            //Send the Command Status Wrapper (CSW) packet            
            USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP,(uint8_t*)&msd_csw,MSD_CSW_SIZE);
            //If the bulk OUT endpoint isn't already armed, make sure to do so 
            //now so we can receive the next CBW packet from the host.
            if(!USBHandleBusy(USBMSDOutHandle))
            {
                USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(uint8_t*)&msd_cbw,MSD_OUT_EP_SIZE);
            }
            MSD_State=MSD_WAIT;
            break;
        default:
            //Illegal condition that should not happen, but might occur if the
            //device firmware incorrectly calls MSDTasks() prior to calling
            //USBMSDInit() during the set-configuration portion of enumeration.
            MSD_State=MSD_WAIT;
            msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
            USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
    }//switch(MSD_State)
    
    //Safe to re-enable USB interrupts now.
    USBUnmaskInterrupts();
    
    return MSD_State;
}


/******************************************************************************
 	Function:
 		uint8_t MSDProcessCommand(void)
 		
 	Description:
 		This funtion processes a command received through the MSD
 		class driver
 		
 	PreCondition:
 		None
 		
 	Paramters:
 		None
 		
 	Return Values:
 		uint8_t - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in MSD.h under the
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
uint8_t MSDProcessCommand(void)
{   
    //Check if the media is either not present, or has been flagged by firmware
    //to pretend to be non-present (ex: SoftDetached).
    if((LUNMediaDetect() == false) || (SoftDetach[gblCBW.bCBWLUN] == true))
    {
        //Clear flag so we know the media need initialization, if it becomes 
        //present in the future.
        gblMediaPresent &= ~((uint16_t)1<<gblCBW.bCBWLUN);
        MSDProcessCommandMediaAbsent();
   	}
    else
    {
        //Check if the media is present and hasn't been already flagged as initialized.
        if((gblMediaPresent & ((uint16_t)1<<gblCBW.bCBWLUN)) == 0)
        {
            //Try to initialize the media
            if(LUNMediaInitialize())
            {
                //The media initialized successfully.  Set flag letting software
                //know that it doesn't need re-initialization again (unless the 
                //media is removable and is subsequently removed and re-inserted). 
                gblMediaPresent |= ((uint16_t)1<<gblCBW.bCBWLUN);

                //The media is present and has initialized successfully.  However,
                //we should still notify the host that the media may have changed,
                //from the host's perspective, since we just initialized it for 
                //the first time.         
                gblSenseData[LUN_INDEX].SenseKey = S_UNIT_ATTENTION;
                gblSenseData[LUN_INDEX].ASC = ASC_NOT_READY_TO_READY_CHANGE;
                gblSenseData[LUN_INDEX].ASCQ = ASCQ_MEDIUM_MAY_HAVE_CHANGED;
                //Signify a soft error to the host, so it knows to check the 
                //sense keys to learn that the media just changed.
                msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED; //No real "error" per se has occurred
                //Process the command now.
                MSDProcessCommandMediaPresent();
            }
            else
            {
                //The media failed to initialize for some reason.
                MSDProcessCommandMediaAbsent();
            }
        }
        else
        {
            //The media was present and was already initialized/ready to process
            //the host's command.
            MSDProcessCommandMediaPresent();
        }
    }

    return MSDCommandState;
}

/******************************************************************************
 	Function:
 		void MSDProcessCommandMediaAbsent(void)
 		
 	Description:
 		This funtion processes a command received through the MSD
 		class driver, when the removable MSD media (ex: MMC/SD card) is not 
 		present, or has been "soft detached" deliberately by the application
 		firmware.
 		
 	PreCondition:
 		The MSD function should have already been initialized (the media isn't
 		required to be initalized however).  Additionally, a valid MSD Command 
 		Block Wrapper (CBW) should have been received and partially parsed 
 		prior to calling this function.
 		
 	Parameters:
 		None
 	
 	Return Values:
 		uint8_t - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in usb_device_msd.h under the 
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
  *****************************************************************************/	
void MSDProcessCommandMediaAbsent(void)
{
    //Check what command we are currently processing, to decide how to handle it.
    switch(MSDCommandState)
    {
        case MSD_REQUEST_SENSE:
            //The host sends this request when it wants to check the status of 
            //the device, and/or identify the reason for the last error that was 
            //reported by the device.
            //Set the sense keys to let the host know that the reason the last
            //command failed was because the media was not present.
            ResetSenseData();
            gblSenseData[LUN_INDEX].SenseKey=S_NOT_READY;
            gblSenseData[LUN_INDEX].ASC=ASC_MEDIUM_NOT_PRESENT;
            gblSenseData[LUN_INDEX].ASCQ=ASCQ_MEDIUM_NOT_PRESENT;

            //After initializing the sense keys above, the subsequent handling 
            //code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;

        case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        case MSD_TEST_UNIT_READY:
            //The host will typically periodically poll the device by sending this
            //request.  Since this is a removable media device, and the media isn't
            //present, we need to indicate an error to let the host know (to 
            //check the sense keys, which will tell it the media isn't present).
            msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
            MSDCommandState = MSD_COMMAND_WAIT;
            break;

        case MSD_INQUIRY:
            //The handling code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;

        case MSD_COMMAND_RESPONSE:
            //The handling code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;

        default:
            //An unsupported command was received.  Since we are uncertain how
            //many bytes we should send/or receive, we should set sense key data
            //and then STALL, to force the host to perform error recovery.
            MSDErrorHandler(MSD_ERROR_UNSUPPORTED_COMMAND);
            break;
    }
}//void MSDProcessCommandMediaAbsent(void)


/******************************************************************************
 	Function:
 		void MSDProcessCommandMediaPresent(void)
 		
 	Description:
 		This funtion processes a command received through the MSD
 		class driver
 		
 	PreCondition:
 		None
 		
 	Paramters:
 		None
 	
 	Return Values:
 		uint8_t - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in MSD.h under the 
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
void MSDProcessCommandMediaPresent(void)
{
    uint8_t i; 
    uint8_t NumBytesInPacket;

    //Check what command we are currently processing, to decide how to handle it.
    switch(MSDCommandState)
    {
        case MSD_READ_10:
            //The host issues a "Read 10" request when it wants to read some number
            //of 10-bit length blocks (512 byte blocks) of data from the media.
            //Since this is a common request and is part of the "critical path"
            //performance wise, we put this at the top of the state machine checks.
            if(MSDReadHandler() == MSD_READ10_WAIT)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
            }
            break;

    	case MSD_WRITE_10:
            //The host issues a "Write 10" request when it wants to write some number
            //of 10-bit length blocks (512 byte blocks) of data to the media.
            //Since this is a common request and is part of the "critical path"
            //performance wise, we put this near the top of the state machine checks.
            if(MSDWriteHandler() == MSD_WRITE10_WAIT)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
            }
            break;
            
    	case MSD_INQUIRY:
    	{
            //The host wants to learn more about our MSD device (spec version,
            //supported abilities, etc.)

            //Error check: If host doesn't want any data, then just advance to CSW phase.
            if(MSDHostNoData == true)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
                break;
            }    
            
            //Get the 16-bit "Allocation Length" (Number of bytes to respond 
            //with.  Note: Value provided in CBWCB is in big endian format)
            TransferLength.byte.HB = gblCBW.CBWCB[3]; //MSB
            TransferLength.byte.LB = gblCBW.CBWCB[4]; //LSB
        	//Check for possible errors.  
            if(MSDCheckForErrorCases(TransferLength.Val) != MSD_ERROR_CASE_NO_ERROR)
            {
                break;
            }

          	//Compute and load proper csw residue and device in number of byte.
            MSDComputeDeviceInAndResidue(sizeof(InquiryResponse));

            //If we get to here, this implies no errors were found and the command is legit.

            //copy the inquiry results from the defined const buffer 
            //  into the USB buffer so that it can be transmitted
            memcpy((void *)&msd_buffer[0], (const void*)&inq_resp, sizeof(InquiryResponse));   //Inquiry response is 36 bytes total
            MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
        }
        case MSD_READ_CAPACITY:
        {
            //The host asked for the total capacity of the device.  The response
            //packet is 8-bytes (32-bits for last LBA implemented, 32-bits for block size).
            USB_MSD_SECTOR_SIZE sectorSize;
            USB_MSD_CAPACITY capacity;

            //get the information from the physical media
            capacity.Val = LUNReadCapacity();
            sectorSize.Val = LUNReadSectorSize();
            
            //Copy the data to the buffer.  Host expects the response in big endian format.
            msd_buffer[0]=capacity.v[3];
            msd_buffer[1]=capacity.v[2];
            msd_buffer[2]=capacity.v[1];
            msd_buffer[3]=capacity.v[0];

            msd_buffer[4]=sectorSize.v[3];
            msd_buffer[5]=sectorSize.v[2];
            msd_buffer[6]=sectorSize.v[1];
            msd_buffer[7]=sectorSize.v[0];

            //Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = 0x08;      //READ_CAPACITY always has an 8-byte response.
            MSDComputeDeviceInAndResidue(0x08);
        
            MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
        }
        case MSD_REQUEST_SENSE:    
            //The host normally sends this request after a CSW completed, where
            //the device indicated some kind of error on the previous transfer.
            //In this case, the host will typically issue this request, so it can
            //learn more details about the cause/source of the error condition.
            
            //Error check: if the host doesn't want any data, just advance to CSW phase.
            if(MSDHostNoData == true)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
                break;
            }    

          	//Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = sizeof(RequestSenseResponse);      //REQUEST_SENSE has an 18-byte response.
            MSDComputeDeviceInAndResidue(sizeof(RequestSenseResponse));
             
            //Copy the requested response data from flash to the USB ram buffer.
            for(i=0;i<sizeof(RequestSenseResponse);i++)
            {
                msd_buffer[i]=gblSenseData[LUN_INDEX]._byte[i];
            }
            MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
            
        case MSD_MODE_SENSE:
            msd_buffer[0]=0x03;
            msd_buffer[1]=0x00;
            msd_buffer[2]=(LUNWriteProtectState()) ? 0x80 : 0x00;
            msd_buffer[3]= 0x00;

            //Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = 0x04;
            MSDComputeDeviceInAndResidue(0x04);
            MSDCommandState = MSD_COMMAND_RESPONSE;
    	    break;

        case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
            gblSenseData[LUN_INDEX].SenseKey=S_ILLEGAL_REQUEST;
            gblSenseData[LUN_INDEX].ASC=ASC_INVALID_COMMAND_OPCODE;
            gblSenseData[LUN_INDEX].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;
            msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
            msd_csw.dCSWDataResidue = 0x00;
            MSDCommandState = MSD_COMMAND_WAIT;
            break;

        case MSD_TEST_UNIT_READY:
            //The host will typically send this command periodically to check if
            //it is ready to be used and to obtain polled notification of changes
            //in status (ex: user removed media from a removable media MSD volume).
            //There is no data stage for this request.  The information we send to
            //the host in response to this request is entirely contained in the CSW.
            
            //First check for possible errors.
            if(MSDCheckForErrorCases(0) != MSD_ERROR_CASE_NO_ERROR)
            {
                break;
            }    
            //The stack sets this condition when the status of the removable media
            //has just changed (ex: the user just plugged in the removable media,
            //in which case we want to notify the host of the changed status, by
            //sending a deliberate "error" notification).  This doesn't mean any 
            //real error has occurred.
            if((gblSenseData[LUN_INDEX].SenseKey==S_UNIT_ATTENTION) && (msd_csw.bCSWStatus==MSD_CSW_COMMAND_FAILED))
            {
                MSDCommandState = MSD_COMMAND_WAIT;
            }
            else
            {
            	ResetSenseData();
            	msd_csw.dCSWDataResidue=0x00;
                MSDCommandState = MSD_COMMAND_WAIT;
            }
            break;

        case MSD_VERIFY:
            //Fall through to STOP_START
            
        case MSD_STOP_START:
            msd_csw.dCSWDataResidue=0x00;
            MSDCommandState = MSD_COMMAND_WAIT;
            break;
            
        case MSD_COMMAND_RESPONSE:
            //This command state didn't originate from the host.  This state was
            //set by the firmware (for one of the other handlers) when it was 
            //finished preparing the data to send to the host, and it is now time
            //to transmit the data over the bulk IN endpoint.
            if(USBHandleBusy(USBMSDInHandle) == false)
            {
                //We still have more bytes needing to be sent.  Compute how many 
                //bytes should be in the next IN packet that we send.
                if(gblCBW.dCBWDataTransferLength >= MSD_IN_EP_SIZE)
                {
                    NumBytesInPacket = MSD_IN_EP_SIZE;
                    gblCBW.dCBWDataTransferLength -= MSD_IN_EP_SIZE;
                }   
                else
                {
                    //This is a short packet and will be our last IN packet sent
                    //in the transfer.
                    NumBytesInPacket = gblCBW.dCBWDataTransferLength;
                    gblCBW.dCBWDataTransferLength = 0;
                } 
                
                //We still have more bytes needing to be sent.  Check if we have
                //already fulfilled the device input expected quantity of bytes.
                //If so, we need to keep sending IN packets, but pad the extra
                //bytes with value = 0x00 (see error case 5 MSD device BOT v1.0 
                //spec handling).
                if(TransferLength.Val >= NumBytesInPacket)
                {
                    //No problem, just send the requested data and keep track of remaining count.
                    TransferLength.Val -= NumBytesInPacket;
                }    
                else
                {
                    //The host is reading more bytes than the device has to send.
                    //In this case, we still need to send the quantity of bytes requested,
                    //but we have to fill the pad bytes with 0x00.  The below for loop
                    //is execution speed inefficient, but performance isn't important 
                    //since this code only executes in the case of a host error 
                    //anyway (Hi > Di).
                    for(i = 0; i < NumBytesInPacket; i++)
                    {
                        if(TransferLength.Val != 0)
                        {
                            TransferLength.Val--;     
                        }    
                        else
                        {
                            msd_buffer[i] = 0x00;
                        }    
                    }    
                }    
                
                //We are now ready to send the packet to the host.                   
                USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP,(uint8_t*)&msd_buffer[0],NumBytesInPacket);
                
                //Check to see if we are done sending all requested bytes of data
                if(gblCBW.dCBWDataTransferLength == 0)
                {
                    //We have sent all the requested bytes.  Go ahead and
                    //advance state so as to send the CSW.
                    MSDCommandState = MSD_COMMAND_WAIT;
                    break;                    
                }                    
            }
            break;
        case MSD_COMMAND_ERROR:
            default:
                //An unsupported command was received.  Since we are uncertain how many
                //bytes we should send/or receive, we should set sense key data and then
                //STALL, to force the host to perform error recovery.
                MSDErrorHandler(MSD_ERROR_UNSUPPORTED_COMMAND);
                break;
	} // end switch	
}//void MSDProcessCommandMediaPresent(void)


/******************************************************************************
 	Function:
 		static void MSDComputeDeviceInAndResidue(uint16_t DiExpected)
 		
 	Description:
 		This is a private function that performs Hi > Di data size checking
 		and handling.  This function also computes the proper CSW data residue
 		and updates the global variable.
 		
 	PreCondition:
 		Should only be called in the context of the 
 		MSDProcessCommandMediaPresent() handler function, after receiving a new
 		command that needs processing.  Before calling this function, make sure
 		the gblCBW.dCBWDataTransferLength and TransferLength.Val variables have
 		been pre-loaded with the expected host and device data size values.
 		
 	Parameters:
 		uint16_t DiExpected - Input: Firmware can specify an addional value that 
 		might be smaller than the TransferLength.Val value.  The function will
 		update TransferLength.Val with the smaller of the original value, or
 		DiExpected.
 		
 	Return Values:
 		None
 		
 	Remarks:
 		None
 
  *****************************************************************************/
static void MSDComputeDeviceInAndResidue(uint16_t DiExpected)
{
    //Error check number of bytes to send.  Check for Hi < Di
    if(gblCBW.dCBWDataTransferLength < DiExpected)
    {
        //The host has requested less data than the entire reponse.  We
        //send only the host requested quantity of bytes.
        msd_csw.dCSWDataResidue = 0;
        TransferLength.Val = gblCBW.dCBWDataTransferLength;
    }   	
    else
    {
        //The host requested greater than or equal to the number of bytes expected.
        if(DiExpected < TransferLength.Val)
        {
            TransferLength.Val = DiExpected;
        }    
        msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength - TransferLength.Val;
    }     
}    


/******************************************************************************
 	Function:
 		uint8_t MSDReadHandler(void)
 		
 	Description:
 		This funtion processes a read command received through 
 		the MSD class driver
 		
 	PreCondition:
 		None
 		
 	Parameters:
 		None
 		
 	Return Values:
 		uint8_t - the current state of the MSDReadHandler state
 		machine.  The valid values are defined in MSD.h under the 
 		MSDReadHandler state machine declaration section
 		
 	Remarks:
 		None
 
  *****************************************************************************/
uint8_t MSDReadHandler(void)
{
    //static uint8_t fetchStatus = PRE_FETCH_COMPLETE;
    uint8_t* ptrSave;
    static uint8_t* ptrLastData = (uint8_t*)&msd_buffer[0];
    static uint8_t* ptrNextData = (uint8_t*)&msd_buffer[MSD_IN_EP_SIZE];
    static bool NewDataAlreadyAvailable;
   
    //Call our data fetching poller, unless we already have unprocessed data
    //ready for our retrieval.  In this case, we need to stop calling our
    //MDD_SDSPI_AsyncReadTasks(), until we are ready to consume the new buffer 
    //worth of data and update the AsyncReadWriteInfo structure with a new pointer.
    if((fetchStatus != FILEIO_SD_ASYNC_READ_NEW_PACKET_READY) && (fetchStatus != FILEIO_SD_ASYNC_READ_COMPLETE))
    {
        fetchStatus = LUNAsyncReadTasks(&AsyncReadWriteInfo);
    }    
    
    switch(MSDReadState)
    {
        case MSD_READ10_WAIT:
            //Extract the LBA from the CBW.  Note: Also need to perform endian 
            //swap, since the multi-uint8_t CBW fields are stored big endian, but
            //the Microchip C compilers are little endian.
        	LBA.v[3]=gblCBW.CBWCB[2];
        	LBA.v[2]=gblCBW.CBWCB[3];
        	LBA.v[1]=gblCBW.CBWCB[4];
        	LBA.v[0]=gblCBW.CBWCB[5];
        	
        	TransferLength.byte.HB = gblCBW.CBWCB[7];   //MSB of Transfer Length (in number of blocks, not bytes)
        	TransferLength.byte.LB = gblCBW.CBWCB[8];   //LSB of Transfer Length (in number of blocks, not bytes)

            //Check for possible error cases before proceeding
            if(MSDCheckForErrorCases(TransferLength.Val * (uint32_t)FILEIO_CONFIG_MEDIA_SECTOR_SIZE) != MSD_ERROR_CASE_NO_ERROR)
            {
                break;
            }    

            //Assume success initially, msd_csw.bCSWStatus will get set to 0x01 
            //or 0x02 later if an error is detected during the actual read sequence.        	
        	msd_csw.bCSWStatus=0x0;
        	msd_csw.dCSWDataResidue=gblCBW.dCBWDataTransferLength;
        	
        	//Set up our fetch info data structure.
            AsyncReadWriteInfo.wNumBytes = MSD_IN_EP_SIZE;
            AsyncReadWriteInfo.pBuffer = (uint8_t*)ptrNextData;
            AsyncReadWriteInfo.dwAddress = LBA.Val;
            AsyncReadWriteInfo.dwBytesRemaining = gblCBW.dCBWDataTransferLength;
            AsyncReadWriteInfo.bStateVariable = FILEIO_SD_ASYNC_READ_QUEUED;
            //Advance state machine to begin fetching data
            MSDReadState = MSD_READ10_XMITING_DATA;
            fetchStatus = FILEIO_SD_ASYNC_READ_BUSY;
            NewDataAlreadyAvailable = false;
            //Fall through...
        case MSD_READ10_XMITING_DATA:
            //Check if the MDD_SDSPI_AsyncReadTasks() function has finished sending us
            //a new packet of data yet.  If not, keep calling it until we get some data.
            if(NewDataAlreadyAvailable == false)
            {
             	//Try to fetch a new packet of data
             	if(fetchStatus == FILEIO_SD_ASYNC_READ_NEW_PACKET_READY) //Check if the MDD_SDSPI_AsyncReadTasks() function is about to return on data on the next call
             	{
                    fetchStatus = LUNAsyncReadTasks(&AsyncReadWriteInfo);
                	NewDataAlreadyAvailable = true;
            	}
            	else
            	{
                    fetchStatus = LUNAsyncReadTasks(&AsyncReadWriteInfo);
                }   	
            }    

        	//Check if we are done handling the whole READ10 request.
        	if((fetchStatus == FILEIO_SD_ASYNC_READ_COMPLETE) && (msd_csw.dCSWDataResidue == 0))
        	{
            	MSDReadState = MSD_READ10_AWAITING_COMPLETION;
            	break;
            }  
            else if(fetchStatus == FILEIO_SD_ASYNC_READ_ERROR)
            {
                MSDReadState = MSD_READ10_ERROR;
            }              
            
            //Check if we currently have any new data ready and we are ready to
            //send the data over USB to the host.
            if((!USBHandleBusy(USBMSDInHandle)) && (NewDataAlreadyAvailable == true))
            {
                //Swap the data pointers.
                ptrSave = ptrLastData;
                ptrLastData = ptrNextData;
                ptrNextData = ptrSave;
                //Prepare the USB module to send an IN transaction worth of data to the host.
                USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP,ptrLastData,MSD_IN_EP_SIZE); 
                //Advance state machine now that the fetch operation is returning data.
                AsyncReadWriteInfo.pBuffer = (uint8_t*)ptrNextData; //Swap pointer, so we use the other buffer (the one not currently in use by USB).
                //We are sending a USB packet worth of data.  Decrement counter.
            	msd_csw.dCSWDataResidue-=MSD_IN_EP_SIZE;  
            	//Get the next operation started, so it can occur concurrently 
            	//with the above USB transfer, for maximum transfer speeds.
   	            if(fetchStatus == FILEIO_SD_ASYNC_READ_NEW_PACKET_READY)
   	            {
                    fetchStatus = LUNAsyncReadTasks(&AsyncReadWriteInfo);
                	NewDataAlreadyAvailable = true;
                }	
                else
                {                    
                    fetchStatus = LUNAsyncReadTasks(&AsyncReadWriteInfo);
                	NewDataAlreadyAvailable = false;
                }    
            }  
            break;
        case MSD_READ10_AWAITING_COMPLETION:
            //If the old data isn't completely sent over USB yet, need to stay
            //in this state and return, until the endpoint becomes available agin.
            if(USBHandleBusy(USBMSDInHandle))
            {
                break;
            }
            else
            {
                MSDReadState = MSD_READ10_WAIT;
            }    
            break;
        case MSD_READ10_ERROR:
        default:
            //A read error occurred.  Notify the host.
            msd_csw.bCSWStatus=0x01;  //indicate error
            //Set error status sense keys, so the host can check them later
            //to determine how to proceed.
            gblSenseData[LUN_INDEX].SenseKey=S_MEDIUM_ERROR;
	        gblSenseData[LUN_INDEX].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
	        gblSenseData[LUN_INDEX].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
            //Make sure the IN endpoint is available before advancing the state machine.
            //Host will expect CSW next.
            if(USBHandleBusy(USBMSDInHandle) != 0)
            {
                break;
            }
            //Stall data endpoint and advance state machine
            USBStallEndpoint(MSD_DATA_IN_EP,1); //Will cause host to perform clear endpoint halt, then request CSW
            MSDReadState = MSD_READ10_WAIT;
    }
    
    
    return MSDReadState;
}


/******************************************************************************
 	Function:
 		uint8_t MSDWriteHandler(void)
 		
 	Description:
 		This funtion processes a write command received through 
 		the MSD class driver
 		
 	PreCondition:
 		None
 		
 	Parameters:
 		None
 		
 	Return Values:
 		uint8_t - the current state of the MSDWriteHandler state
 		machine.  The valid values are defined in MSD.h under the 
 		MSDWriteHandler state machine declaration section
 		
 	Remarks:
 		None
 
 *****************************************************************************/
uint8_t MSDWriteHandler(void)
{
    static uint8_t LastWriteStatus;
    static unsigned short int BufferredBytes;
    static uint8_t* pNextData;
    static uint8_t* pUSBReceive;
    static USB_HANDLE MSDOutHandle1;
    static USB_HANDLE MSDOutHandle2;
    static USB_HANDLE NextHandleToCheck;
    static uint32_t PacketRequestCounter;
    static uint32_t PacketsRemainingToBeReceived;


    switch(MSDWriteState)
    {
        case MSD_WRITE10_WAIT:
            /* Read the LBA, TransferLength fields from Command Block
               NOTE: CB is Big-Endian */
        	LBA.v[3]=gblCBW.CBWCB[2];
        	LBA.v[2]=gblCBW.CBWCB[3];
        	LBA.v[1]=gblCBW.CBWCB[4];
        	LBA.v[0]=gblCBW.CBWCB[5];
        	TransferLength.v[1]=gblCBW.CBWCB[7];    //TransferLength is in units of LBAs to transfer.
        	TransferLength.v[0]=gblCBW.CBWCB[8];
            msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength;

            //Do some error case checking.
            if(MSDCheckForErrorCases(TransferLength.Val * (uint32_t)FILEIO_CONFIG_MEDIA_SECTOR_SIZE) != MSD_ERROR_CASE_NO_ERROR)
            {
                //An error was detected.  The MSDCheckForErrorCases() function will
                //have taken care of setting the proper states to report the error to the host.
                break;
            } 
        
      		//Check if the media is write protected before deciding what
      		//to do with the data.
      		if(LUNWriteProtectState()) 
            {
                //The media appears to be write protected.
          	    //Let host know error occurred.  The bCSWStatus flag is also used by
          	    //the write handler, to know not to even attempt the write sequence.
          	    msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;    

                //Set sense keys so the host knows what caused the error.
          	    gblSenseData[LUN_INDEX].SenseKey=S_DATA_PROTECT;
          	    gblSenseData[LUN_INDEX].ASC=ASC_WRITE_PROTECTED;
          	    gblSenseData[LUN_INDEX].ASCQ=ASCQ_WRITE_PROTECTED;

                //Stall the OUT endpoint, so as to promptly inform the host
                //that the data cannot be accepted, due to write protected media.
          		USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          		MSDWriteState = MSD_WRITE10_WAIT;
          	    return MSDWriteState;
          	}
        	
        	//Initialize our ASYNC_IO structure, so the MDD_SDSPI_AsyncWriteTasks()
        	//API will know what to do.
        	AsyncReadWriteInfo.bStateVariable = FILEIO_SD_ASYNC_WRITE_QUEUED;
        	AsyncReadWriteInfo.dwAddress = LBA.Val;
        	AsyncReadWriteInfo.dwBytesRemaining = gblCBW.dCBWDataTransferLength;
        	AsyncReadWriteInfo.pBuffer = (uint8_t*)&msd_buffer[0];
        	AsyncReadWriteInfo.wNumBytes = MSD_OUT_EP_SIZE;
        	LastWriteStatus = FILEIO_SD_ASYNC_WRITE_BUSY;
   	
        	
            //Initialize other parameters
            PacketRequestCounter = gblCBW.dCBWDataTransferLength / MSD_OUT_EP_SIZE;
            PacketsRemainingToBeReceived = PacketRequestCounter;
            BufferredBytes = 0;
            pNextData = (uint8_t*)&msd_buffer[0];
            pUSBReceive = (uint8_t*)&msd_buffer[0];
            
            //Arm both MSD bulk OUT endpoints (even and odd) to begin receiving
            //the data to write to the media, from the host.
            if((!USBHandleBusy(USBMSDOutHandle)) && (!USBHandleBusy(USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST))))
            {
                MSDOutHandle1 = USBRxOnePacket(MSD_DATA_OUT_EP, pUSBReceive, MSD_OUT_EP_SIZE);
                pUSBReceive += MSD_OUT_EP_SIZE;
                MSDOutHandle2 = USBRxOnePacket(MSD_DATA_OUT_EP, pUSBReceive, MSD_OUT_EP_SIZE);
                pUSBReceive += MSD_OUT_EP_SIZE;

                PacketRequestCounter -= 2u;
            }
            else
            {
                //Something is wrong.  The endpoints should have been free at 
                //this point in the code.
                msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
                USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          		MSDWriteState = MSD_WRITE10_WAIT;
          		break;                
            }        

            NextHandleToCheck = MSDOutHandle1;
            USBMSDOutHandle = NextHandleToCheck;
    
        	
        	MSDWriteState = MSD_WRITE10_RX_SECTOR;
            //Fall through to MSD_WRITE10_RX_SECTOR
        case MSD_WRITE10_RX_SECTOR:
        {
      	    //Check if we have pending data to receive, and have actually received it.
      	    //Also make sure we have some room in our receive buffer RAM before
      	    //processing the newest data and re-arming the EP to receive even more
      	    //data.
      	    if((PacketsRemainingToBeReceived != 0) && (!USBHandleBusy(NextHandleToCheck)) && (BufferredBytes < (2u * MSD_OUT_EP_SIZE )))
      	    {
          	    //We just finished receiving a packet worth of data.  Make sure
          	    //it was the expected size.
          		if(USBHandleGetLength(NextHandleToCheck) != MSD_OUT_EP_SIZE)
          		{
              		//The host sent us an unexpected short packet.  This is
              		//presumably an error, possibly a phase error (ex: host 
              		//send a CBW, instead of a data packet), since bulk 
              		//endpoint sizes are required to be a power of 2, which 
              		//happens to be exact integer divider of the write block
              		//size (512).
              		msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
              		USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
              		MSDWriteState = MSD_WRITE10_WAIT;
              		break;
                }  		
          		BufferredBytes += MSD_OUT_EP_SIZE;
          		//Reduce number of future packets we expect to receive for this
          		//Write10 request.
          		PacketsRemainingToBeReceived--;              	    
          	       
          	    if(NextHandleToCheck == MSDOutHandle1)
          	    {
          	        NextHandleToCheck = MSDOutHandle2;      	                  	    
          	    }
          	    else
          	    {
              	    NextHandleToCheck = MSDOutHandle1;      	                  	    
              	}        
              	
              	//Re-arm the endpoint now, if we still need more data from the
              	//host to finish the Write10 request.
              	if(PacketRequestCounter != 0)
              	{
                  	USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP, pUSBReceive, MSD_OUT_EP_SIZE);
                    //Decrement remaining packets to be requested, so we know when
                    //to stop requesting more data.
                    PacketRequestCounter--;
    
              		//Increment next receive location pointer, but check for wraparound
              		pUSBReceive += MSD_OUT_EP_SIZE;
              		if(pUSBReceive >= (uint8_t*)&msd_buffer[4u * (uint16_t)MSD_OUT_EP_SIZE])
              		{
                  		pUSBReceive = (uint8_t*)&msd_buffer[0];
                    }   
                } 
          	} 
 

          	if(LastWriteStatus == FILEIO_SD_ASYNC_WRITE_BUSY)
          	{
              	//Just keep calling the function until it is ready to receive new
              	//data to write to the media.
              	if(msd_csw.bCSWStatus == 0x00)
              	{
                    LastWriteStatus = LUNAsyncWriteTasks(&AsyncReadWriteInfo);
              	}    
            } 
      	    
      	    if(LastWriteStatus == FILEIO_SD_ASYNC_WRITE_ERROR)
      	    {
          	    //Some unexpected error occurred.  Notify the host.
          		msd_csw.bCSWStatus = 0x01;
          		USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          		MSDWriteState = MSD_WRITE10_WAIT;
          	    break;
          	}    
      	    if(LastWriteStatus == FILEIO_SD_ASYNC_WRITE_COMPLETE)
      	    {
          	    //The media is now fully done with all packets of the write request.
          	    MSDWriteState = MSD_WRITE10_WAIT;
                USBMSDOutHandle = USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          	    break;
          	}    
  
            //Check if the media write state machine wants data, and we have 
            //data ready to send it.
            if((LastWriteStatus == FILEIO_SD_ASYNC_WRITE_SEND_PACKET) && (BufferredBytes > 0))
            {
                //Send the next packet worth of data.
                AsyncReadWriteInfo.pBuffer = pNextData;
                if(msd_csw.bCSWStatus == 0x00)
                {
              		msd_csw.dCSWDataResidue -= MSD_OUT_EP_SIZE;
                    LastWriteStatus = LUNAsyncWriteTasks(&AsyncReadWriteInfo);
                }    

                //Update pointer to point to the next data location, so we will
                //be ready for the next iteration of this code.  Make sure
                //to check for pointer wraparound.
                pNextData += MSD_OUT_EP_SIZE;
                if(pNextData >= (uint8_t*)&msd_buffer[4u * (uint16_t)MSD_OUT_EP_SIZE])
                {
                    pNextData = (uint8_t*)&msd_buffer[0];
                }    
                
                //We removed data from the buffer.  
                BufferredBytes -= MSD_OUT_EP_SIZE;
                //Check for underflow.  This shouldn't happen, and would only
                //occur in some kind of error case.
                if((signed short int)BufferredBytes < 0)
                {
                    BufferredBytes = 0;
                }    
            }    

            break;

        }//case MSD_WRITE10_RX_SECTOR:
        default:
            //Illegal condition which should not occur.  If for some reason it
            //does, try to let the host know know an error has occurred.
            msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;    //Phase Error
			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
            MSDWriteState = MSD_WRITE10_WAIT;            
    }
    
    return MSDWriteState;
}


/******************************************************************************
 	Function:
 		void ResetSenseData(void)
 		
 	Description:
 		This routine resets the Sense Data, initializing the
 		structure RequestSenseResponse gblSenseData.
 		
 	PreCondition:
 		None 
 		
 	Parameters:
 		None
 		
 	Return Values:
 		None
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
void ResetSenseData(void) 
{
    gblSenseData[LUN_INDEX].ResponseCode=S_CURRENT;
    gblSenseData[LUN_INDEX].VALID=0;			// no data in the information field
    gblSenseData[LUN_INDEX].Obsolete=0x0;
    gblSenseData[LUN_INDEX].SenseKey=S_NO_SENSE;
    //gblSenseData.Resv;
    gblSenseData[LUN_INDEX].ILI=0;
    gblSenseData[LUN_INDEX].EOM=0;
    gblSenseData[LUN_INDEX].FILEMARK=0;
    gblSenseData[LUN_INDEX].InformationB0=0x00;
    gblSenseData[LUN_INDEX].InformationB1=0x00;
    gblSenseData[LUN_INDEX].InformationB2=0x00;
    gblSenseData[LUN_INDEX].InformationB3=0x00;
    gblSenseData[LUN_INDEX].AddSenseLen=0x0a;	// n-7 (n=17 (0..17))
    gblSenseData[LUN_INDEX].CmdSpecificInfo.Val=0x0;
    gblSenseData[LUN_INDEX].ASC=0x0;
    gblSenseData[LUN_INDEX].ASCQ=0x0;
    gblSenseData[LUN_INDEX].FRUC=0x0;
    gblSenseData[LUN_INDEX].SenseKeySpecific[0]=0x0;
    gblSenseData[LUN_INDEX].SenseKeySpecific[1]=0x0;
    gblSenseData[LUN_INDEX].SenseKeySpecific[2]=0x0;
}



/******************************************************************************
 	Function:
 		uint8_t MSDCheckForErrorCases(uint32_t DeviceBytes)
 		
 	Description:
 	   This function can be called to check for various error cases, primarily 
 	   the "Thirteen Cases" errors described in the MSD BOT v1.0 specs.  If an
 	   error is detected, the function internally calls the MSDErrorHandler()
 	   handler function, to take care of appropriately responding to the host, 
 	   based on the error condition.
 	PreCondition:
 	    None
 	     		
 	Parameters:
 		uint32_t DeviceBytes - Input: This is the total number of bytes the MSD 
 		            device firmware is expecting in the MSD transfer.  
 	Return Values:
 		uint8_t - Returns a byte containing the error code.  The possible error
 		    cases that can be detected and reported are:
            MSD_ERROR_CASE_NO_ERROR - None of the "Thirteen cases" errors were detected
            MSD_ERROR_CASE_2 	            
            MSD_ERROR_CASE_3 	            
            MSD_ERROR_CASE_4 	            
            MSD_ERROR_CASE_5 	            
            MSD_ERROR_CASE_7 	            
            MSD_ERROR_CASE_8 	            
            MSD_ERROR_CASE_9 	            
            MSD_ERROR_CASE_11               
            MSD_ERROR_CASE_10               
            MSD_ERROR_CASE_13               
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
uint8_t MSDCheckForErrorCases(uint32_t DeviceBytes)
{
    uint8_t MSDErrorCase;
    bool HostMoreDataThanDevice;
    bool DeviceNoData;
  
    //Check if device is expecting no data (Dn)
    if(DeviceBytes == 0)
    {
        DeviceNoData = true;
    }    
    else
    {
        DeviceNoData = false;
    }     
    
    //First check for the three good/non-error cases
    
    //Check for good case: Hn = Dn (Case 1)
    if((MSDHostNoData == true) && (DeviceNoData == true))
    {
        return MSD_ERROR_CASE_NO_ERROR;
    }    

    //Check for good cases where the data sizes between host and device match
    if(gblCBW.dCBWDataTransferLength == DeviceBytes)
    {
        //Check for good case: Hi = Di (Case 6)
        if(MSD_State == MSD_DATA_IN)
        {
            //Make sure Hi = Di, instead of Hi = Do
            if(MSDCommandState != MSD_WRITE_10)
            {
                return MSD_ERROR_CASE_NO_ERROR;
            }    
        }
        else //if(MSD_State == MSD_DATA_OUT)  
        {
            //Check for good case: Ho = Do (Case 12)
            //Make sure Ho = Do, instead of Ho = Di
            if(MSDCommandState == MSD_WRITE_10)
            {
                return MSD_ERROR_CASE_NO_ERROR;
            }             
        }      
    }    

    //If we get to here, this implies some kind of error is occuring.  Do some
    //checks to find out which error occurred, so we know how to handle it.

    //Check if the host is expecting to transfer more bytes than the device. (Hx > Dx)
    if(gblCBW.dCBWDataTransferLength > DeviceBytes)
    {
        HostMoreDataThanDevice = true;
    }   
    else
    {
        HostMoreDataThanDevice = false;
    } 
 
    //Check host's expected data direction
	if(MSD_State == MSD_DATA_OUT)
	{
    	//First check for Ho <> Di (Case 10)
    	if((MSDCommandState != MSD_WRITE_10) && (DeviceNoData == false))
    	    MSDErrorCase = MSD_ERROR_CASE_10;
   	   	//Check for Hn < Do  (Case 3)
    	else if(MSDHostNoData == true)  
    	    MSDErrorCase = MSD_ERROR_CASE_3;
    	//Check for Ho > Dn  (Case 9)
    	else if(DeviceNoData == true)
    	    MSDErrorCase = MSD_ERROR_CASE_9;
    	//Check for Ho > Do  (Case 11)
    	else if(HostMoreDataThanDevice == true)
    	    MSDErrorCase = MSD_ERROR_CASE_11;
    	//Check for Ho < Do  (Case 13)
    	else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
    	    MSDErrorCase = MSD_ERROR_CASE_13;
    }    
    else //else the MSD_State must be == MSD_DATA_IN
    {
    	//First check for Hi <> Do (Case 8)
    	if(MSDCommandState == MSD_WRITE_10)
    	    MSDErrorCase = MSD_ERROR_CASE_8;    	
    	//Check for Hn < Di  (Case 2)
    	else if(MSDHostNoData == true)  
    	    MSDErrorCase = MSD_ERROR_CASE_2;
    	//Check for Hi > Dn  (Case 4)
    	else if(DeviceNoData == true)
    	    MSDErrorCase = MSD_ERROR_CASE_4;
    	//Check for Hi > Di  (Case 5)
    	else if(HostMoreDataThanDevice == true)
    	    MSDErrorCase = MSD_ERROR_CASE_5;
        //Check for Hi < Di  (Case 7)
    	else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
    	    MSDErrorCase = MSD_ERROR_CASE_7;
    }        
    //Now call the MSDErrorHandler(), based on the error that was detected.
    MSDErrorHandler(MSDErrorCase);
    return MSDErrorCase;
}    


/******************************************************************************
 	Function:
 		void MSDErrorHandler(uint8_t ErrorCase)
 		
 	Description:
 	    Once an error condition has been detected, this function can be called
 	    to set the proper states and perform the proper tasks needed to let the
 	    host know about the error.
 	PreCondition:
 		Firmware should have already determined an error occurred, and it should
 		know what the error code was before calling this handler.
 		
 	Parameters:
 		uint8_t ErrorCase - Input: This is the error code that the firmware 
 		                    detected.  This error code will determine how the
 		                    handler will behave (ex: what status to send to host,
 		                    what endpoint(s) should be stalled, etc.).
 		                    The implemented error case possibilities are (suffix
 		                    numbers correspond to the "Thirteen cases" numbers 
 		                    described in the MSD BOT specs v1.0):
 		                    
                            MSD_ERROR_CASE_2 	            
                            MSD_ERROR_CASE_3 	            
                            MSD_ERROR_CASE_4 	            
                            MSD_ERROR_CASE_5 	            
                            MSD_ERROR_CASE_7 	            
                            MSD_ERROR_CASE_8 	            
                            MSD_ERROR_CASE_9 	            
                            MSD_ERROR_CASE_11               
                            MSD_ERROR_CASE_10               
                            MSD_ERROR_CASE_13               
                            MSD_ERROR_UNSUPPORTED_COMMAND   

 	Return Values:
 		None
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
void MSDErrorHandler(uint8_t ErrorCase)
{
    uint8_t OldMSD_State;
    
    //Both MSD bulk IN and OUT endpoints should not be busy when these error cases are detected
    //If for some reason this isn't true, then we should preserve the state machines states for now.
    if((USBHandleBusy(USBMSDInHandle)) || (USBHandleBusy(USBMSDOutHandle)))
    {
    	return;	
    }

    //Save the old state before we change it.  The old state is needed to determine
    //the proper handling behavior in the case of receiving unsupported commands.
    OldMSD_State = MSD_State;

    //Reset main state machines back to idle values.
    MSDCommandState = MSD_COMMAND_WAIT;
    MSDReadState = MSD_READ10_WAIT;
    MSDWriteState = MSD_WRITE10_WAIT;
    //After the conventional 13 test cases failures, the host still expects a valid CSW packet
    msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength; //Indicate the unconsumed/unsent data
    msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;    //Gets changed later to phase error for errors that user phase error
    MSD_State = MSD_SEND_CSW;

    //Now do other error related handling tasks, which depend on the specific 
    //error	type that was detected.
    switch(ErrorCase)
    {
        case MSD_ERROR_CASE_2://Also CASE_3
            msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
            break;

        case MSD_ERROR_CASE_4://Also CASE_5
            USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
            break;

        case MSD_ERROR_CASE_7://Also CASE_8
            msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
            USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
            break;

        case MSD_ERROR_CASE_9://Also CASE_11
            USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST); //Stall the bulk OUT endpoint
            break;

        case MSD_ERROR_CASE_10://Also CASE_13
            msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
            USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
            break;

        case MSD_ERROR_UNSUPPORTED_COMMAND:
            ResetSenseData();
            gblSenseData[LUN_INDEX].SenseKey=S_ILLEGAL_REQUEST;
            gblSenseData[LUN_INDEX].ASC=ASC_INVALID_COMMAND_OPCODE;
            gblSenseData[LUN_INDEX].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;
            
            if((OldMSD_State == MSD_DATA_OUT) && (gblCBW.dCBWDataTransferLength != 0))
            {
                USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
            }
            else
            {
                USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
            }
            break;
        default:	//Shouldn't get hit, don't call MSDErrorHandler() if there is no error
            break;
    }//switch(ErrorCase)
}	



//-----------------------------------------------------------------------------------------
#endif //end of #ifdef USB_USE_MSD
//End of file usb_device_msd_multi_sector.c
