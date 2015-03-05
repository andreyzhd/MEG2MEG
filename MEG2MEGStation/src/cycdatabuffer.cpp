/*
 * cycdatabuffer.cpp
 *
 * Author: Andrey Zhdanov
 * Copyright (C) 2015 Department of Neuroscience and Biomedical Engineering,
 * Aalto University School of Science
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <iostream>
#include <time.h>

#include "config.h"
#include "cycdatabuffer.h"

using namespace std;


CycDataBuffer::CycDataBuffer(int _bufSize, bool _ignoreIsRec, FixedStimuli* _fixedStimuli)
{
	insertPtr = 0;
	getPtr = 0;
	isRec = false;
	bufSize = _bufSize;
    buffSemaphore = new QSemaphore();
    ignoreIsRec = _ignoreIsRec;
    fixedStimuli = _fixedStimuli;

    // Allocate the buffer. Reserve some extra space necessary to handle
    // chunks of varying size.
    dataBuf = (unsigned char*)malloc(bufSize + 2 * (int(bufSize*MAX_CHUNK_SIZE) + sizeof(ChunkAttrib)));
    if (!dataBuf)
    {
    	cerr << "Cannot allocate memory for circular buffer" << endl;
    	abort();
    }
}


CycDataBuffer::~CycDataBuffer()
{
    free(dataBuf);
    delete(buffSemaphore);
}


void CycDataBuffer::insertChunk(unsigned char* _data, ChunkAttrib _attrib)
{
    int            fixedStimFrameSz;
    unsigned char* dataSrc;

	// Check for buffer overflow. CIRC_BUF_MARG is the safety margin against
	// race condition between consumer and producer threads when the buffer
	// is close to full.
	if (buffSemaphore->available() >=  bufSize * (1-CIRC_BUF_MARG))
	{
		cerr << "Circular buffer overflow!" << endl;
		abort();
	}

	// Make sure that the safety margin is at least several (four) times the
	// chunk size. This is necessary to prevent the race condition between
	// consumer and producer threads when the buffer is close to full.
	if(_attrib.chunkSize+sizeof(ChunkAttrib) > bufSize*MAX_CHUNK_SIZE)
	{
		cerr << "The chunk size is too large!" << endl;
		abort();
	}

	// insert the data into the circular buffer
	if(!ignoreIsRec)
	{
		_attrib.isRec = isRec;
	}

    if(fixedStimuli && isRec)
    {   // Check whether we might want to replace the frame with a frame from the fixedStimuli

        dataSrc = fixedStimuli->findFrame(startRecTstamp, _attrib.timestamp, &fixedStimFrameSz);
        if(dataSrc)
        {
            _attrib.chunkSize = fixedStimFrameSz;
        }
        else
        {
            dataSrc = _data;
        }
    }
    else
    {
        dataSrc = _data;
    }

	memcpy(dataBuf + insertPtr, (unsigned char*)(&_attrib), sizeof(ChunkAttrib));
	insertPtr += sizeof(ChunkAttrib);
	buffSemaphore->release(sizeof(ChunkAttrib));

    memcpy(dataBuf + insertPtr, dataSrc, _attrib.chunkSize);
	buffSemaphore->release(_attrib.chunkSize);

    emit chunkReady(dataBuf + insertPtr);

	insertPtr += _attrib.chunkSize;
	if(insertPtr >= bufSize)
	{
		insertPtr = 0;
	}
}


unsigned char* CycDataBuffer::getChunk(ChunkAttrib* _attrib)
{
	unsigned char* res;

	buffSemaphore->acquire(sizeof(ChunkAttrib));
	memcpy((unsigned char*)_attrib, dataBuf + getPtr, sizeof(ChunkAttrib));
	getPtr += sizeof(ChunkAttrib);

	buffSemaphore->acquire(_attrib->chunkSize);
	res = dataBuf + getPtr;

	getPtr += _attrib->chunkSize;
	if(getPtr >= bufSize)
	{
		getPtr = 0;
	}

	return(res);
}


void CycDataBuffer::setIsRec(bool _isRec)
{
    struct timespec	timestamp;

	isRec = _isRec;

    // TODO: The code below does not properly protect startRecTstamp against
    // muti-theaded access

    if (isRec)
    {
        clock_gettime(CLOCK_REALTIME, &timestamp);

        startRecTstamp = timestamp.tv_nsec / 1000000;
        startRecTstamp += timestamp.tv_sec * 1000;
    }
}
