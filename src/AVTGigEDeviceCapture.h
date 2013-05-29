//
//  AVTGigEDeviceCapture.h
//  BeerMe
//
//  Created by Shawn Roske on 5/6/13.
//
//

#include "cinder/app/AppBasic.h"

#include "CinderOpenCv.h"

#include "RingBuffer.h"

#ifdef _WINDOWS
//#include "StdAfx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#endif

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/times.h>
#include <arpa/inet.h>
#endif

#include "PvApi.h"

#ifdef _WINDOWS
#define _STDCALL __stdcall
#else
#define _STDCALL
#ifndef TRUE
#define TRUE     0
#endif
#endif

#define FRAMESCOUNT 10

typedef struct
{
    unsigned long   UID;
    tPvHandle       Handle;
    tPvFrame        Frames[FRAMESCOUNT];
    bool            Abort;
    
} tCamera;

class AVTGigEDeviceCapture {
public:
	AVTGigEDeviceCapture();
	~AVTGigEDeviceCapture();
	
	void setup();
    void update();
    void shutdown();
    
    cv::Mat currentFrame();
    bool hasNewFrame();
    
    void appendFrame(tPvFrame *pFrame);
    void HandleCameraPlugged(unsigned long UniqueId);
    void HandleCameraUnplugged(unsigned long UniqueId);
    
protected:
    bool EventSetup();
    void EventUnsetup();
    bool CameraSetup();
    void CameraUnsetup();
    bool CameraStart();
    void CameraStop();

    bool mInited;
    bool mHasNewFrame;
    cv::Mat mCurrentFrame;
    
    tCamera GCamera;
    
    RingBuffer *mRingBuffer;

};
