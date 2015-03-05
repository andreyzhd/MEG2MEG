/*
 * audiofilewriter.cpp
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
#include <stdlib.h>

#include "config.h"
#include "audiofilewriter.h"
#include "settings.h"

using namespace std;

AudioFileWriter::AudioFileWriter(CycDataBuffer* _cycBuf, const char* _path, int _siteId, bool _isSender, int _nchans, QLineEdit* _suffix)
	:	FileWriter(_cycBuf, _path, "aud", _siteId, _isSender, _suffix)
{
	Settings	settings;
	uint32_t	nchans = _nchans;
	uint32_t	srate = settings.sampRate;
	uint32_t	ver = AUDIO_FILE_VERSION;

	// Create header
	bufLen = strlen(MAGIC_AUDIO_STR) + 3 * sizeof(uint32_t) + 2;
	buf = (unsigned char*)malloc(bufLen);

	if(!buf)
	{
		cerr << "Error allocating memory!" << endl;
		abort();
	}

	memcpy(buf, MAGIC_AUDIO_STR, strlen(MAGIC_AUDIO_STR));									// string identifying the file type
	memcpy(buf + strlen(MAGIC_AUDIO_STR), &ver, sizeof(uint32_t));							// version of file format
	memset(buf + strlen(MAGIC_AUDIO_STR) + sizeof(uint32_t), _siteId, 1);					// site ID
	memset(buf + strlen(MAGIC_AUDIO_STR) + sizeof(uint32_t) + 1, (_isSender? 1 : 0), 1);	// sender / receiver flag
	memcpy(buf + strlen(MAGIC_AUDIO_STR) + sizeof(uint32_t) + 2, &srate, sizeof(uint32_t));		// sampling rate
	memcpy(buf + strlen(MAGIC_AUDIO_STR) + 2*sizeof(uint32_t) + 2, &nchans, sizeof(uint32_t));	// number of channels
}


AudioFileWriter::~AudioFileWriter()
{
	free(buf);
}


unsigned char* AudioFileWriter::getHeader(int* _len)
{
	*_len = bufLen;
	return(buf);
}
