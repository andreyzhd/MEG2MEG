/*
 * sendingsocket.h
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

#ifndef SENDINGSOCKET_H_
#define SENDINGSOCKET_H_

#include <QUdpSocket>
#include <QMutex>

class SendingSocket : public QObject
{
	Q_OBJECT

public:
	SendingSocket();
	virtual ~SendingSocket();

public slots:
    void sendAudioPacket(unsigned char* _data);
    void sendVideoPacket(unsigned char* _data);

private:
	QUdpSocket*	  	udpSocket;
	QHostAddress* 	udpClientAddr;
	int				udpClientPort;
	QMutex*			socketMutex;

	void sendPacket(unsigned char* _data, char _packetType);
};

#endif /* SENDINGSOCKET_H_ */
