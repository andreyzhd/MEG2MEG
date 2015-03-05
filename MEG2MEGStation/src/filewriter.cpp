/*
 * filewriter.cpp
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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "filewriter.h"

using namespace std;

FileWriter::FileWriter(CycDataBuffer* _cycBuf, const char* _path, const char* _ext, int _siteId, bool _isSender, QLineEdit* _suffix)
{
	cycBuf = _cycBuf;
	siteId = _siteId;
	isSender = _isSender;
	suffix = _suffix;

	path = (char*)malloc(strlen(_path)+1);
	if(!path)
	{
		cerr << "Cannot allocate memory!" << endl;
		abort();
	}

	ext = (char*)malloc(strlen(_ext)+1);
	if(!ext)
	{
		cerr << "Cannot allocate memory!" << endl;
		abort();
	}

	strcpy(path, _path);
	strcpy(ext, _ext);
}


FileWriter::~FileWriter()
{
	free(ext);
	free(path);
}


void FileWriter::stoppableRun()
{
	unsigned char*	databuf;
	bool			prevIsRec=false;
	ofstream		outData;
	char			nameBuf[500];
	time_t			timeNow;
    struct tm*		timeNowParsed;
    ChunkAttrib		chunkAttrib;
    uint32_t		chunkSz;

    unsigned char*	header;
    int				headerLen;

	while (true)
	{
		databuf = cycBuf->getChunk(&chunkAttrib);
		if (chunkAttrib.isRec)
		{
			if (!prevIsRec)
			{
				timeNow = time(NULL);
				timeNowParsed = localtime(&timeNow);
				// TODO: replace sprintf with C++ strings
				sprintf(nameBuf, "%s/%04i-%02i-%02i--%02i-%02i-%02i--%s--site_%01i_%s.%s",
						path,
						timeNowParsed->tm_year+1900,
						timeNowParsed->tm_mon+1,
						timeNowParsed->tm_mday,
						timeNowParsed->tm_hour,
						timeNowParsed->tm_min,
						timeNowParsed->tm_sec,
						suffix->text().toLatin1().data(),
						siteId,
						(isSender ? "sender" : "receiver"),
						ext);
				outData.open(nameBuf, ios_base::out | ios_base::binary | ios_base::trunc);
				if(outData.fail())
				{
					// TODO: Add more elaborate error checking
					cerr << "Error opening the file " << nameBuf << endl;
					abort();
				}
				header = getHeader(&headerLen);
				outData.write((const char*)header, headerLen);
			}

			chunkSz = chunkAttrib.chunkSize;
			outData.write((const char*)(&(chunkAttrib.timestamp)), sizeof(uint64_t));
			outData.write((const char*)(&(chunkAttrib.id)), sizeof(uint64_t));
			outData.write((const char*)(&chunkSz), sizeof(uint32_t));
			outData.write((const char*)databuf, chunkAttrib.chunkSize);
		}
		else
		{
			if (prevIsRec)
			{
				outData.close();
			}
		}

		prevIsRec = chunkAttrib.isRec;

		if(shouldStop)
		{
			if(prevIsRec)
			{
				outData.close();
			}
			return;
		}
	}
}
