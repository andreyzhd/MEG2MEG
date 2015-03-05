/*
 * sendingsocket.cpp
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

#include "config.h"
#include "sendingsocket.h"
#include "cycdatabuffer.h"
#include "settings.h"

using namespace std;

SendingSocket::SendingSocket()
{
	Settings settings;

	udpSocket = new QUdpSocket();
	udpClientAddr = new QHostAddress(settings.udpRemoteReceiverAddr);
	udpClientPort = settings.udpRemoteReceiverPort;
	socketMutex = new QMutex();
}


SendingSocket::~SendingSocket()
{
	delete socketMutex;
	delete udpClientAddr;
	delete udpSocket;
}


void SendingSocket::sendPacket(unsigned char* _data, char _packetType)
{
	unsigned int	i;
	qint64			rc;
	ChunkAttrib 	chunkAttrib;
	chunkAttrib = *((ChunkAttrib*)(_data-sizeof(ChunkAttrib)));
	char			buf[chunkAttrib.chunkSize + sizeof(ChunkAttrib) + 1];

	// Send the data over UDP. The first byte of the datagram contains type
	// identifier (audio/video).

	// Copy the data. If audio data, send only one channel.
	if(_packetType == UDP_AUDIO_PACKET)
	{
		chunkAttrib.chunkSize /= N_CHANS_SENDER;

		for(i=0; i<chunkAttrib.chunkSize/sizeof(AUDIO_DATA_TYPE); i++)
		{
			((AUDIO_DATA_TYPE*)(buf+1+sizeof(ChunkAttrib)))[i] = ((AUDIO_DATA_TYPE*)_data)[N_CHANS_SENDER*i];
		}
	}
	else
	{
		memcpy(buf+1+sizeof(ChunkAttrib), _data, chunkAttrib.chunkSize);
	}

	buf[0] = _packetType;
	*((ChunkAttrib*)(buf+1)) = chunkAttrib;

	socketMutex->lock();
	rc = udpSocket->writeDatagram(buf, chunkAttrib.chunkSize + sizeof(ChunkAttrib) + 1, *udpClientAddr, udpClientPort);
	socketMutex->unlock();

	if(rc == -1)
	{
		cerr << "Error sending the datagram: " << udpSocket->errorString().toLatin1().data() << endl;
	}
	else if(rc != (chunkAttrib.chunkSize + sizeof(ChunkAttrib) + 1))
	{
		cerr << "Error sending the datagram: only " << rc << " bytes were sent" << endl;
	}
}


void SendingSocket::sendAudioPacket(unsigned char* _data)
{
	sendPacket(_data, UDP_AUDIO_PACKET);
}


void SendingSocket::sendVideoPacket(unsigned char* _data)
{
	sendPacket(_data, UDP_VIDEO_PACKET);
}
