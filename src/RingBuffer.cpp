//
//  RingBuffer.cpp
//
//  Created by Shawn Roske on 5/6/13.
//
//

#include "RingBuffer.h"


RingBuffer::RingBuffer( int count, int _bufferSize )
{
	readIndex  = 0;
	writeIndex = 0;
	bufferSize = _bufferSize;
	bufferCount = count;
	
	buffer = new BYTE*[bufferCount];
    
	for ( int i = 0; i < bufferCount; ++i )
		buffer[i] = new BYTE[bufferSize];
}


RingBuffer::~RingBuffer()
{
	for ( int i = 0; i < bufferCount; ++i )
		delete buffer[i];
	delete buffer;
	buffer = NULL;
}

int RingBuffer::size()
{   
	return bufferSize;
}

int RingBuffer::nextIndex( int index )
{
	int nextIndex = 0;
    
	if( index >= (bufferCount-1) )
		nextIndex = 0;
	else
		nextIndex = index + 1;

	return nextIndex;
}

PBYTE RingBuffer::getNextBufferToWrite()
{
	int nextWriteIndex = nextIndex( writeIndex );
	if( nextWriteIndex == readIndex ){
		return NULL;
	}else{
		return buffer[ nextWriteIndex ];
	}
}

void RingBuffer::writeFinished()
{
	writeIndex = nextIndex( writeIndex );
}

PBYTE RingBuffer::getNextBufferToRead()
{
	PBYTE nextBuffer = NULL;
    
	if( readIndex == writeIndex ){
		nextBuffer = NULL;
	}else{
		int nextReadIndex = nextIndex( readIndex );
		nextBuffer = buffer[ nextReadIndex ];
	}
    
	return nextBuffer;
}

void RingBuffer::readFinished()
{   
	readIndex = nextIndex( readIndex );
}

bool RingBuffer::isLastBufferToRead()
{
	return nextIndex( readIndex ) == writeIndex;
}
