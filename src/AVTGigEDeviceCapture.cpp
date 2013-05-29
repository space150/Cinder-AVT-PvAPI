/*
 | ==============================================================================
 | Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
 |
 | Redistribution of this header file, in original or modified form, without
 | prior written consent of AVT is prohibited.
 |
 |==============================================================================
 |
 | EventCallback
 |
 | Based on StreamPnp. Adding PvCameraEventCallbackRegister mechanism,
 | allowing callback to occur on many different camera signals/events. As of
 | firmware 1.42, the following events are available:
 |   AcquisitionStart
 |   AcquisitionEnd
 |   FrameTrigger
 |   ExposureEnd
 |   AcquisitionRecordTrigger
 |   SyncIn1Rise
 |   SyncIn1Fall
 |   SyncIn2Rise
 |   SyncIn2Fall
 |   SyncIn3Rise
 |   SyncIn3Fall
 |   SyncIn4Rise
 |   SyncIn4Fall
 | See Event Controls section of the "Camera and Driver" attributes document
 | online at http://www.alliedvisiontec.com for the latest listing of supported
 | events.
 |
 | Three callback mechanisms at work here:
 | - PvLinkCallbackRegister: Callback when camera first recognized or unplugged
 | - PvCameraEventCallbackRegister: See above
 | - FrameCallback: when frame completes.
 |
 | Program flow:
 | - Main thread registers link callback, waits for Ctrl+C escape
 | - Link callback registers event callbacks, sets camera to 5 FPS, starts streaming.
 | - Event callbacks print the returned event
 | - Frame Callbacks requeue frames for streaming.
 |
 |==============================================================================
 |
 | THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 | WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
 | NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
 | DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 | INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 | LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 | OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF
 | LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 | NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 | EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 |
 |==============================================================================
 */

#include "AVTGigEDeviceCapture.h"

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
struct tms      gTMS;
unsigned long   gT00 = times(&gTMS);

void Sleep(unsigned int time)
{
    struct timespec t,r;
    
    t.tv_sec    = time / 1000;
    t.tv_nsec   = (time % 1000) * 1000000;
    
    while(nanosleep(&t,&r)==-1)
        t = r;
}
#endif

// Event callback.  This is called by PvApi when camera event(s) occur.
void _STDCALL F_CameraEventCallback(void*                   Context,
                                    tPvHandle               Camera,
                                    const tPvCameraEvent*	EventList,
                                    unsigned long			EventListLength)
{
	//multiple events may have occurred for this one callback
	for (unsigned long i = 0; i < EventListLength; i++)
	{
		switch (EventList[i].EventId) {
			case 40000:
				printf("EventAcquisitionStart\n");
				break;
			case 40001:
				printf("EventAcquisitionEnd\n");
				break;
			case 40002:
				//printf("EventFrameTrigger\n");
				break;
			case 40003:
				printf("EventExposureEnd\n");
				break;
			case 40004:
				printf("EventAcquisitionRecordTrigger\n");
				break;
			case 40010:
				printf("EventSyncIn1Rise\n");
				break;
			case 40011:
				printf("EventSyncIn1Fall\n");
				break;
			case 40012:
				printf("EventSyncIn2Rise\n");
				break;
			case 40013:
				printf("EventSyncIn2Fall\n");
				break;
			case 40014:
				printf("EventSyncIn3Rise\n");
				break;
			case 40015:
				printf("EventSyncIn3Fall\n");
				break;
			case 40016:
				printf("EventSyncIn4Rise\n");
				break;
			case 40017:
				printf("EventSyncIn4Fall\n");
				break;
			case 65534:
				printf("EventOverflow error\n");
				break;
			default:
				printf("Event %lu\n",EventList[i].EventId);
				break;
		}
	}
}

// setup event channel
// return value: true == success, false == fail
bool AVTGigEDeviceCapture::EventSetup()
{
	unsigned long EventBitmask;
	tPvErr errCode;
	
	// check if events supported with this camera firmware
	if (PvAttrExists(GCamera.Handle,"EventsEnable1") == ePvErrNotFound)
	{
        printf("This camera does not support event notifications.\n");
        return false;
	}
	
	//Clear all events
	//EventsEnable1 is a bitmask of all events. Bits correspond to last two digits of EventId.
	// e.g: Bit 1 is EventAcquisitionStart, Bit 2 is EventAcquisitionEnd, Bit 10 is EventSyncIn1Rise.
    if ((errCode = PvAttrUint32Set(GCamera.Handle,"EventsEnable1",0)) != ePvErrSuccess)
	{
		printf("Set EventsEnable1 err: %u\n", errCode);
		return false;
	}
    
	//Set individual events (could do in one step with EventsEnable1).
	if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventSelector","AcquisitionStart")) != ePvErrSuccess)
	{
		printf("Set EventsSelector err: %u\n", errCode);
		return false;
	}
    if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventNotification","On")) != ePvErrSuccess)
	{
		printf("Set EventsNotification err: %u\n", errCode);
		return false;
	}
    
	if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventSelector","AcquisitionEnd")) != ePvErrSuccess)
	{
		printf("Set EventsSelector err: %u\n", errCode);
		return false;
	}
    if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventNotification","On")) != ePvErrSuccess)
	{
		printf("Set EventsNotification err: %u\n", errCode);
		return false;
	}
    
	if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventSelector","FrameTrigger")) != ePvErrSuccess)
	{
		printf("Set EventsSelector err: %u\n", errCode);
		return false;
	}
    if ((errCode = PvAttrEnumSet(GCamera.Handle,"EventNotification","On")) != ePvErrSuccess)
	{
		printf("Set EventsNotification err: %u\n", errCode);
		return false;
	}
	
	//Get and print bitmask
	PvAttrUint32Get(GCamera.Handle,"EventsEnable1", &EventBitmask);
	printf("Events set. EventsEnable1 bitmask: %lu\n", EventBitmask);
    
    //register callback function
	if ((errCode = PvCameraEventCallbackRegister(GCamera.Handle,F_CameraEventCallback,NULL)) != ePvErrSuccess)
    {
		printf("PvCameraEventCallbackRegister err: %u\n", errCode);
        return false;
    }
	return true;
}

// unsetup event channel
void AVTGigEDeviceCapture::EventUnsetup()
{
    // wait so that the "AcquisitionEnd" [from CameraStop()] can be received on the event channel
    Sleep(1000);
	// clear all events
	PvAttrUint32Set(GCamera.Handle,"EventsEnable1",0);
    // unregister callback function
	PvCameraEventCallbackUnRegister(GCamera.Handle,F_CameraEventCallback);
}

// Frame completed callback executes on seperate driver thread.
// One callback thread per camera. If a frame callback function has not
// completed, and the next frame returns, the next frame's callback function is queued.
// This situation is best avoided (camera running faster than host can process frames).
// Spend as little time in this thread as possible and offload processing
// to other threads or save processing until later.
//
// Note: If a camera is unplugged, this callback will not get called until PvCaptureQueueClear.
// i.e. callback with pFrame->Status = ePvErrUnplugged doesn't happen -- so don't rely
// on this as a test for a missing camera.
void _STDCALL FrameDoneCB(tPvFrame* pFrame)
{
    AVTGigEDeviceCapture *capture = (AVTGigEDeviceCapture *)pFrame->Context[0];
	if ( capture != NULL )
		capture->appendFrame(pFrame);
}

void AVTGigEDeviceCapture::appendFrame(tPvFrame *pFrame)
{
	// update the current frame pixels if we have a successfull callback
	if ( pFrame->Status == ePvErrSuccess )
	{   
		// get buffer to write the image
		PBYTE cameraWriteBuffer = mRingBuffer->getNextBufferToWrite();
		if ( cameraWriteBuffer )
		{
			//printf( "AVTGigEDeviceCapture::appendFrame() - Loading image buffer into ring buffer\n" );
            
			// copy the new frame in the buffer, and notify it
			memcpy( cameraWriteBuffer, pFrame->ImageBuffer, mRingBuffer->size() );
            
			// Notify ring buffer that write is finished
			mRingBuffer->writeFinished();
		}
		else
		{
			printf( "AVTGigEDeviceCapture::appendFrame() - didn't get new buffer to write in the ring buffer!!!\n" );
		}
	}
	else if ( pFrame->Status == ePvErrDataLost || pFrame->Status == ePvErrDataMissing )
	{
		printf( "AVTGigEDeviceCapture::appendFrame() - frame data missing, are we dropping frames?\n" );
	}
	else
	{
		printf( "AVTGigEDeviceCapture::appendFrame() - failed to grab frame! status: %i\n", pFrame->Status );
	}
    
    // if the frame was completed (or if data were missing/lost) we re-enqueue it
	if( pFrame->Status == ePvErrSuccess  || pFrame->Status == ePvErrDataLost || pFrame->Status == ePvErrDataMissing )
		PvCaptureQueueFrame(GCamera.Handle,pFrame,FrameDoneCB);
}

// open camera, allocate memory
// return value: true == success, false == fail
bool AVTGigEDeviceCapture::CameraSetup()
{
    tPvErr errCode;
	bool failed = false;
	unsigned long FrameSize = 0;
    
	// open camera
	if ((errCode = PvCameraOpen(GCamera.UID,ePvAccessMaster,&(GCamera.Handle))) != ePvErrSuccess)
	{
		if (errCode == ePvErrAccessDenied)
			printf("PvCameraOpen returned ePvErrAccessDenied:\nCamera already open, or not properly closed.\n");
		else
			printf("PvCameraOpen err: %u\n", errCode);
		return false;
	}
    
	// Calculate frame buffer size
    if((errCode = PvAttrUint32Get(GCamera.Handle, "TotalBytesPerFrame", &FrameSize)) != ePvErrSuccess)
	{
		printf("CameraSetup: Get TotalBytesPerFrame err: %u\n", errCode);
		return false;
	}
    
    mRingBuffer = new RingBuffer( FRAMESCOUNT, FrameSize );
    
	// allocate the frame buffers
    for(int i=0;i<FRAMESCOUNT && !failed;i++)
    {
        GCamera.Frames[i].ImageBuffer = new char[FrameSize];
        if(GCamera.Frames[i].ImageBuffer)
        {
			GCamera.Frames[i].ImageBufferSize = FrameSize;
		}
        else
		{
			printf("CameraSetup: Failed to allocate buffers.\n");
			failed = true;
		}        
        GCamera.Frames[i].Context[0] = (void*)this;
    }
    
	return !failed;
}

// close camera, free memory.
void AVTGigEDeviceCapture::CameraUnsetup()
{
    tPvErr errCode;
	
    if((errCode = PvCameraClose(GCamera.Handle)) != ePvErrSuccess)
	{
		printf("CameraUnSetup: PvCameraClose err: %u\n", errCode);
	}
	else
	{
		printf("Camera closed.\n");
	}
	// delete image buffers
    for(int i=0;i<FRAMESCOUNT;i++)
        delete [] (char*)GCamera.Frames[i].ImageBuffer;
    
    GCamera.Handle = NULL;
    
    delete mRingBuffer;
	mRingBuffer = NULL;
}


// setup and start streaming
// return value: true == success, false == fail
bool AVTGigEDeviceCapture::CameraStart()
{
    tPvErr errCode;
	bool failed = false;
    
    // NOTE: This call sets camera PacketSize to largest sized test packet, up to 8228, that doesn't fail
	// on network card. Some MS VISTA network card drivers become unresponsive if test packet fails.
	// Use PvUint32Set(handle, "PacketSize", MaxAllowablePacketSize) instead. See network card properties
	// for max allowable PacketSize/MTU/JumboFrameSize.
	if((errCode = PvCaptureAdjustPacketSize(GCamera.Handle,8228)) != ePvErrSuccess)
	{
		printf("CameraStart: PvCaptureAdjustPacketSize err: %u\n", errCode);
		return false;
	}
    
    // start driver capture stream
	if((errCode = PvCaptureStart(GCamera.Handle)) != ePvErrSuccess)
	{
		printf("CameraStart: PvCaptureStart err: %u\n", errCode);
		return false;
	}
    
    // queue frames with FrameDoneCB callback function. Each frame can use a unique callback function
	// or, as in this case, the same callback function.
	for(int i=0;i<FRAMESCOUNT && !failed;i++)
	{
		if((errCode = PvCaptureQueueFrame(GCamera.Handle,&(GCamera.Frames[i]),FrameDoneCB)) != ePvErrSuccess)
		{
			printf("CameraStart: PvCaptureQueueFrame err: %u\n", errCode);
			failed = true;
		}
	}
    
	if (failed)
		return false;
    
	// set the camera to 30 FPS, continuous mode, and start camera receiving triggers
	if((PvAttrFloat32Set(GCamera.Handle, "FrameRate", 30) != ePvErrSuccess) ||
       (PvAttrEnumSet(GCamera.Handle, "FrameStartTriggerMode", "FixedRate") != ePvErrSuccess) ||
       (PvAttrEnumSet(GCamera.Handle, "AcquisitionMode", "Continuous") != ePvErrSuccess) ||
       (PvCommandRun(GCamera.Handle, "AcquisitionStart") != ePvErrSuccess))
	{
		printf("CameraStart: failed to set camera attributes\n");
		return false;
	}
    
	return true;
}


// stop streaming
void AVTGigEDeviceCapture::CameraStop()
{
    tPvErr errCode;
	
	//stop camera receiving triggers
	if ((errCode = PvCommandRun(GCamera.Handle,"AcquisitionStop")) != ePvErrSuccess)
		printf("AcquisitionStop command err: %u\n", errCode);
	else
		printf("AcquisitionStop success.\n");
    
	//PvCaptureQueueClear aborts any actively written frame with Frame.Status = ePvErrDataMissing
	//Further queued frames returned with Frame.Status = ePvErrCancelled
	
	//Add delay between AcquisitionStop and PvCaptureQueueClear
	//to give actively written frame time to complete
	Sleep(200);
	
	printf("Calling PvCaptureQueueClear...\n");
	if ((errCode = PvCaptureQueueClear(GCamera.Handle)) != ePvErrSuccess)
		printf("PvCaptureQueueClear err: %u\n", errCode);
	else
		printf("...Queue cleared.\n");
    
	//stop driver stream
	if ((errCode = PvCaptureEnd(GCamera.Handle)) != ePvErrSuccess)
		printf("PvCaptureEnd err: %u\n", errCode);
	else
		printf("Driver stream stopped.\n");
}

//Setup camera events and stream.
void AVTGigEDeviceCapture::HandleCameraPlugged(unsigned long UniqueId)
{
    if(!GCamera.UID && !GCamera.Abort)
    {
        GCamera.UID = UniqueId;
        
        if(CameraSetup())
        {
            printf("Camera %lu opened\n",UniqueId);
            
            // setup event channel
            if(true)//EventSetup())
			{
				// start streaming from the camera
				if (!CameraStart())
					GCamera.Abort = true;
                
                mInited = true;
			}
			else
				GCamera.Abort = true;
        }
        else
		{
            //failure. signal main thread to abort
			GCamera.Abort = true;
		}
    }
}

void AVTGigEDeviceCapture::HandleCameraUnplugged(unsigned long UniqueId)
{
    if(GCamera.UID == UniqueId)
    {
		//signal main thread to abort
		GCamera.Abort = true;
    }
}

// callback function called on seperate thread when a registered camera event received
void _STDCALL CameraLinkCallback(void* Context,
                                 tPvInterface Interface,
                                 tPvLinkEvent Event,
                                 unsigned long UniqueId)
{
    AVTGigEDeviceCapture *capture = (AVTGigEDeviceCapture *)Context;
    switch(Event)
    {
        case ePvLinkAdd:
        {
			printf("LinkEvent: camera %lu recognized\n",UniqueId);
            capture->HandleCameraPlugged(UniqueId);
            break;
        }
        case ePvLinkRemove:
        {
			printf("LinkEvent: camera %lu unplugged\n",UniqueId);
            capture->HandleCameraUnplugged(UniqueId);
            
            break;
        }
        default:
            break;
    }
}

AVTGigEDeviceCapture::AVTGigEDeviceCapture()
{
    mInited = false;
    mCurrentFrame = cv::Mat();
    mHasNewFrame = false;
}

AVTGigEDeviceCapture::~AVTGigEDeviceCapture()
{
    // nothing
}

void AVTGigEDeviceCapture::setup()
{
    tPvErr errCode;
	
	// initialize the PvAPI
	if((errCode = PvInitialize()) != ePvErrSuccess)
    {
		printf("PvInitialize err: %u\n", errCode);
    }
	else
	{
        //IMPORTANT: Initialize camera structure. See tPvFrame in PvApi.h for more info.
		memset(&GCamera,0,sizeof(tCamera));
        
		printf("Waiting for camera discovery...\n");
        
        // register camera plugged in callback
        if((errCode = PvLinkCallbackRegister(CameraLinkCallback,ePvLinkAdd,this)) != ePvErrSuccess)
			printf("PvLinkCallbackRegister err: %u\n", errCode);
        
        // register camera unplugged callback
        if((errCode = PvLinkCallbackRegister(CameraLinkCallback,ePvLinkRemove,this)) != ePvErrSuccess)
			printf("PvLinkCallbackRegister err: %u\n", errCode);
		
		// All camera setup, event setup, streaming, handled in ePvLinkAdd callback
    }
}

void AVTGigEDeviceCapture::shutdown()
{
    if ( mInited )
    {
        tPvErr errCode;
        
        CameraStop();
//		EventUnsetup();
        CameraUnsetup();
        
        if((errCode = PvLinkCallbackUnRegister(CameraLinkCallback,ePvLinkAdd)) != ePvErrSuccess)
			printf("PvLinkCallbackUnRegister err: %u\n", errCode);
        if((errCode = PvLinkCallbackUnRegister(CameraLinkCallback,ePvLinkRemove)) != ePvErrSuccess)
			printf("PvLinkCallbackUnRegister err: %u\n", errCode);
        
        // uninitialize the API
        PvUnInitialize();
    }
}

void AVTGigEDeviceCapture::update()
{
    if ( !mInited ) return;
    
	// Check if we have a new frame
	PBYTE cameraReadBuffer = mRingBuffer->getNextBufferToRead();
	int bufferEntryCount = 1;
    
	// Get all the new frames
	if( cameraReadBuffer != NULL)
    {
		while( cameraReadBuffer != NULL )
		{
			// only pluck the last available buffer and convert it
			if ( mRingBuffer->isLastBufferToRead() )
			{
				//printf("AVTGigEDeviceCapture::update() - copying frame from ring buffer, skipped %i entries\n", bufferEntryCount-1 );
                
                // mono8
                mCurrentFrame = cv::Mat( 493, 656, CV_8UC1, cameraReadBuffer);
                
                // rgb24
                //cv::Mat rgbFrame = cv::Mat( 493, 656, CV_8UC3, cameraReadBuffer);;
                //cv::cvtColor(rgbFrame, mCurrentFrame, CV_RGB2BGR);
			}
            
			// Notify read finished to the ring buffer (so it can be loaded with new frames)
			mRingBuffer->readFinished();
            
			// Check if there are more new frames-
			cameraReadBuffer = mRingBuffer->getNextBufferToRead();
            
			bufferEntryCount++;
		}
        
		mHasNewFrame = true;
	}
	else if ( cameraReadBuffer == NULL )
	{
		mHasNewFrame = false;
	}
}

cv::Mat AVTGigEDeviceCapture::currentFrame()
{
    return mCurrentFrame;
}

bool AVTGigEDeviceCapture::hasNewFrame()
{
    return mHasNewFrame;
}
