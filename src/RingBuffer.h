//
//  RingBuffer.h
//
//  Created by Shawn Roske on 5/6/13.
//
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
typedef unsigned char BYTE;
typedef BYTE *PBYTE;
#endif

class RingBuffer {
public:
	RingBuffer( int count, int _bufferSize );
	~RingBuffer();
    
	int size();
    
	PBYTE getNextBufferToWrite();
	void writeFinished();
    
	PBYTE getNextBufferToRead();
	bool isLastBufferToRead();
	void readFinished();
    
private:
	int nextIndex( int index );
	int bufferSize;
	int	bufferCount;
    
	PBYTE* buffer;
	volatile char readIndex;
	volatile char writeIndex;
};

#endif
