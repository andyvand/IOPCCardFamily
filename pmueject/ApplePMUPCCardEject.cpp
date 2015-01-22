/*
 * Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "ApplePMUPCCardEject.h"

#ifdef PCMCIA_DEBUG
#define DEBUG(args...)	IOLog(args)
#else
#define DEBUG(args...)
#endif

#define super IOPCCardEjectController
OSDefineMetaClassAndStructors(ApplePMUPCCardEject, IOPCCardEjectController)

#define kPMUpcmcia	0x04   		// pcmcia (buttons and timeout-eject)

bool
ApplePMUPCCardEject::start(IOService * provider)
{
    if (!super::start(provider)) return false;
	
    bridge = OSDynamicCast(IOPCIDevice, provider);
    if (!bridge) return false;

    // mac os 9 uses the interrupt pin reg in config space to determine which socket
    // the card is really in.  since we only run on the newer machinces, we can just
    // look it up in the device tree.

    OSData * socketData = OSDynamicCast(OSData, bridge->getProperty("AAPL,pmu-socket-number"));
    if (!socketData) return false;
    
    pmuSocket = *(UInt32 *)socketData->getBytesNoCopy();
    if (!pmuSocket) return false;

    // Wait for the PMU to show up:
    pmuDriver = waitForService(serviceMatching("ApplePMU"));
    if (!pmuDriver) return false;

    // Register for the eject button interrupts and card ejection timeout
    if (pmuDriver->callPlatformFunction("registerForPMUInterrupts", true, (void*)kPMUpcmcia,
					(void*)handleInterrupt, (void*)this, NULL) != kIOReturnSuccess) {
	return false;
    }

    DEBUG("ApplePMUPCCardEject::start for pmu socket %d was successful\n", pmuSocket);
    
    return true;
}

void
ApplePMUPCCardEject::stop(IOService * provider)
{
    DEBUG("ApplePMUPCCardEject::stop, pmu socket %d\n", pmuSocket);

    if (pmuDriver) {
        pmuDriver->callPlatformFunction("deRegisterClient", true, (void*)this, (void*)kPMUpcmcia, NULL, NULL);
    }
    
    super::stop(provider);
}

//========================================================================================================
//========================================================================================================
//========================================================================================================

// We need this to callPlatformFunction when sending to sendMiscCommand
typedef struct SendMiscCommandParameterBlock {
    int 	command;
    IOByteCount sLength;
    UInt8 *	sBuffer;
    IOByteCount *rLength;
    UInt8 * 	rBuffer;
} SendMiscCommandParameterBlock;
typedef SendMiscCommandParameterBlock *SendMiscCommandParameterBlockPtr;

IOReturn
ApplePMUPCCardEject::localSendMiscCommand(int command, IOByteCount sLength, UInt8 *sBuffer)
{
    if (!pmuDriver) return kIOReturnError;

    SendMiscCommandParameterBlock prmBlock = {command, sLength, sBuffer, 0, 0};

    return pmuDriver->callPlatformFunction("sendMiscCommand", true, (void*)&prmBlock, NULL, NULL, NULL);   
}

#define kPMUDoPCMCIAEject 0x4C        	// eject PCMCIA card(s)

IOReturn
ApplePMUPCCardEject::ejectCard()
{
    DEBUG("ApplePMUPCCardEject::ejectCard sending command\n");

    IOReturn rc = localSendMiscCommand(kPMUDoPCMCIAEject, 1, &pmuSocket);
    if (rc != kIOReturnSuccess) return rc;

    return super::ejectCard();
}

//========================================================================================================
//========================================================================================================
//========================================================================================================

//	
//	Handles interrupts generated by the PMGR micro for users pushing the card eject
//	buttons, and for eject timeouts after an eject operation has been initiated.
//
//	The interrupt data is contained in a buffer passed to this interrupt handler.
//	The buffer contains the following info:
//
//	byte 0:	flags for all PMGR interrupt sources
//		 1:	bit 0=1:	button interrupt
//			    1=1:	timeout interrupt
//		 2:	bit field for sockets, i.e., if bit 0=1, socket 0 is affected, etc.

#define	kEjectRequestPMgrOp		1
#define	kEjectTimeoutPMgrOp		2
#define	kEjectInterruptTypeMask	(kEjectRequestPMgrOp | kEjectTimeoutPMgrOp)

/* static */ void
ApplePMUPCCardEject::handleInterrupt(IOService *client, UInt8 interruptMask, UInt32 length, UInt8 *buffer)
{
    // Check if we are the right client for this interrupt
    if (interruptMask != kPMUpcmcia) return;

    ApplePMUPCCardEject *myThis = OSDynamicCast(ApplePMUPCCardEject, client);
    if (!myThis) return;

    // check to see if this is our socket
    if (!buffer || (length < 2)) return;
    if (buffer[1] != myThis->pmuSocket) return;

    DEBUG("ApplePMUPCCardEject::handleInterrupt mask = 0x%x, pmu socket = %d\n", interruptMask, myThis->pmuSocket);
    DEBUG("ApplePMUPCCardEject::handleInterrupt length = %d, buffer = %x %x\n", (int)length, buffer[0], buffer[1]);
    
    switch (buffer[0] & kEjectInterruptTypeMask) {

    case kEjectRequestPMgrOp:
	DEBUG("ApplePMUPCCardEject::handleInterrupt EJECTION_REQUEST\n");
	(void)myThis->requestCardEjection();

	break;

    case kEjectTimeoutPMgrOp:
	DEBUG("ApplePMUPCCardEject::handleInterrupt EJECTION_FAILED\n");

	break;

    default:
	DEBUG("ApplePMUPCCardEject::handleInterrupt command completed cmd = 0x%x\n", buffer[0]);

	break;
    }
}
