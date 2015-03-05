/*
 * cycdatabuffer.h
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

#ifndef CYCDATABUFFER_H_
#define CYCDATABUFFER_H_

#include <stdint.h>
#include <QObject>
#include <QSemaphore>

#include "fixedstimuli.h"

//! Attributes associated with each data chunk.
typedef struct
{
	int			chunkSize;
	uint64_t	timestamp;
	uint64_t	id;
	bool		isRec;
} ChunkAttrib;


//! Cyclic buffer capable of holding data chunks of variable size.
/*!
 * Cyclic buffer for synchronizing producer/consumer threads. Supports single
 * producer, single primary consumer and multiple secondary consumers. Producer
 * and primary consumer access the buffer through insertChunk() and getChunk()
 * respectively. No additional synchronization between the producer and the
 * primary consumer is needed - the buffer takes care of it.
 *
 * Secondary consumers are only notified of every new chunk inserted trough
 * chunkReady() signal. The buffer provides no hard guaranty that the data will
 * not be overwritten before secondary consumers access it, but if the buffer
 * is large enough in comparison to the chunk insertion rate, the secondary
 * consumers are fast enough and the are general system load is low enough,
 * this should not typically happen.
 *
 * Neither primary nor secondary consumers should modify the content of the
 * data chunks.
 *
 * The class assumes that in general the primary consumer is considerably
 * faster than the producer, so that buffer is nearly empty most of the time.
 */
class CycDataBuffer : public QObject
{
	Q_OBJECT

public:
	/*!
	 * Buffer size is in bytes. Due to implementation details (semaphore counts
	 * bytes rather than chunks) buffer size is limited by the size of "int" on
	 * your platform. If _ignoreIsRec is true, does not change the isRec field
	 * of the ChunkAttrib, otherwise sets them according to the buffer's state
	 * set by setIsRec.
	 */
    CycDataBuffer(int _bufSize, bool _ignoreIsRec, FixedStimuli* _fixedStimuli=NULL);
	virtual ~CycDataBuffer();
	void insertChunk(unsigned char* _data, ChunkAttrib _attrib);

	/*!
	 * Acquire a chunk and return a pointer to it. The chunk is implicitly
	 * released next time getChunk is called.
	 */
	unsigned char* getChunk(ChunkAttrib* _attrib);
	void setIsRec(bool _isRec);

signals:
	/*!
	 * This signal is raised when a new chunk of data has been copied to the
	 * buffer. _data points to the chunk's data. The corresponding ChunkAttrib
	 * structure is placed immediately before _data.
	 */
    void chunkReady(unsigned char* _data);

private:
    volatile bool	  isRec;
    volatile uint64_t startRecTstamp;

    QSemaphore*		  buffSemaphore;	// counts number of bytes available for reading
    unsigned char*	  dataBuf;
    FixedStimuli*     fixedStimuli;

    int				  insertPtr;
    int				  getPtr;
    int				  bufSize;
    bool			  ignoreIsRec;
};

#endif /* CYCDATABUFFER_H_ */
