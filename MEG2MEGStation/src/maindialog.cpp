/*
 * maindialog.cpp
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
#include "maindialog.h"

using namespace std;


MainDialog::MainDialog(QWidget *parent)
    : QMainWindow(parent)
{

	ui.setupUi(this);
	setWindowFlags(Qt::WindowTitleHint);
    fixedStimuli = new FixedStimuli();
    isRec = false;

	//---------------------------------------------------------------------
	// Sender stuff
	//

	// Set up video recording
	initVideo();

    // Set up audio recording
    senderAudioBuf = new CycDataBuffer(CIRC_AUDIO_BUFF_SZ, false);
    microphoneThread = new MicrophoneThread(senderAudioBuf);
    senderAudioFileWriter = new AudioFileWriter(senderAudioBuf, settings.storagePath, settings.siteId, true, N_CHANS_SENDER, ui.suffixEdit);
    sendingSocket = new SendingSocket();
    QObject::connect(senderAudioBuf, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onAudioUpdate(unsigned char*)));
    QObject::connect(senderAudioBuf, SIGNAL(chunkReady(unsigned char*)), sendingSocket, SLOT(sendAudioPacket(unsigned char*)));

	// Initialize volume indicator history
	memset(volSenderMaxvals, 0, N_CHANS_SENDER * N_BUF_4_VOL_IND * sizeof(AUDIO_DATA_TYPE));
	volSenderIndNext = 0;

    ui.senderLevelLeft->setMaximum(MAX_AUDIO_VAL);
    ui.senderLevelRight->setMaximum(MAX_AUDIO_VAL);

    // Start audio running
    senderAudioFileWriter->start();
    microphoneThread->start();

    if(camera1)
    {
        senderVideoDialog = new SenderVideoDialog(camera1, 1, sendingSocket, ui.suffixEdit, fixedStimuli);
    	senderVideoDialog->show();
    }

    //---------------------------------------------------------------------
    // Receiver stuff
    //

    // Set up audio recording
    receiverAudioBuf = new CycDataBuffer(CIRC_AUDIO_BUFF_SZ, true);
    receiverAudioFileWriter = new AudioFileWriter(receiverAudioBuf, settings.storagePath, settings.siteId, false, N_CHANS_RECEIVER, ui.suffixEdit);

    // Initialize volume indicator history
	memset(volReceiverMaxvals, 0, N_CHANS_RECEIVER * N_BUF_4_VOL_IND * sizeof(AUDIO_DATA_TYPE));
	volReceiverIndNext = 0;

	// Initialize speaker
	speakerBuffer = new NonBlockingBuffer(settings.spkBufSz, settings.framesPerPeriod*N_CHANS_RECEIVER*sizeof(AUDIO_DATA_TYPE));
	speakerThread = new SpeakerThread(speakerBuffer);

    ui.receiverLevelLeft->setMaximum(MAX_AUDIO_VAL);

    // Start audio running
    receiverAudioFileWriter->start();
    speakerThread->start();

    receiverVideoBuf = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, true);
	receiverVideoDialog = new ReceiverVideoDialog(receiverVideoBuf, ui.suffixEdit);
	receiverVideoDialog->show();

    // Start listening to the network
	udpSocket = new QUdpSocket();
    QObject::connect(udpSocket, SIGNAL(readyRead()), this, SLOT(onUdpPacketArrived()));
	udpSocket->bind(settings.udpLocalReceiverPort);
}


MainDialog::~MainDialog()
{
	// TODO: Implement proper destructor
}


void MainDialog::onStartRec()
{
    struct timespec	timestamp;

    ui.stopButton->setEnabled(true);
    ui.startButton->setEnabled(false);
    ui.suffixEdit->setEnabled(false);

    isRec = true;

    // TODO: The code below does not properly protect startRecTstamp against
    // muti-theaded access

    clock_gettime(CLOCK_REALTIME, &timestamp);
    startRecTstamp = timestamp.tv_nsec / 1000000;
    startRecTstamp += timestamp.tv_sec * 1000;

    if(camera1)
    {
    	senderVideoDialog->setIsRec(true);
    }

    senderAudioBuf->setIsRec(true);
}


void MainDialog::onStopRec()
{
    ui.stopButton->setEnabled(false);
    ui.startButton->setEnabled(true);
    ui.suffixEdit->setEnabled(true);

    isRec = false;

    if(camera1)
    {
    	senderVideoDialog->setIsRec(false);
    }

    senderAudioBuf->setIsRec(false);
}


void MainDialog::onAudioUpdate(unsigned char* _data)
{
	unsigned int 	i=0;
	unsigned int	j;
	AUDIO_DATA_TYPE	maxvals[N_CHANS_SENDER]={0};
	AUDIO_DATA_TYPE	curval;

	// Update the history
	memset(&(volSenderMaxvals[volSenderIndNext]), 0, N_CHANS_SENDER * sizeof(AUDIO_DATA_TYPE));
	while(i < settings.framesPerPeriod * N_CHANS_SENDER)
	{
		for(j=0; j<N_CHANS_SENDER; j++)
		{
			curval = abs(((AUDIO_DATA_TYPE*)_data)[i++]);
			volSenderMaxvals[volSenderIndNext + j] = (volSenderMaxvals[volSenderIndNext + j] >= curval) ? volSenderMaxvals[volSenderIndNext + j] : curval;
		}
	}

	volSenderIndNext += N_CHANS_SENDER;
	volSenderIndNext %= (N_CHANS_SENDER * N_BUF_4_VOL_IND);

	// Compute maxima for all channels
	i = 0;
	while(i < N_CHANS_SENDER * N_BUF_4_VOL_IND)
	{
		for(j=0; j<N_CHANS_SENDER; j++)
		{
			curval = volSenderMaxvals[i++];
			maxvals[j] = (maxvals[j] >= curval) ? maxvals[j] : curval;
		}
	}

	// Update only two level bars
	ui.senderLevelLeft->setValue(maxvals[0]);
	ui.senderLevelRight->setValue(maxvals[1]);
}


void MainDialog::initVideo()
{
    dc1394_t*				dc1394Context;
    dc1394camera_list_t*	camList;
    dc1394error_t			err;

    dc1394Context = dc1394_new();
    if(!dc1394Context)
    {
        cerr << "Cannot initialize!" << endl;
        abort();
    }

    err = dc1394_camera_enumerate(dc1394Context, &camList);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Failed to enumerate cameras" << endl;
        abort();
    }

    camera1 = NULL;

    if (camList->num == 0)
    {
        cerr << "No cameras found" << endl;
        return;
    }

    // use the first camera in the list
    camera1 = dc1394_camera_new(dc1394Context, camList->ids[0].guid);
    if (!camera1)
    {
        cerr << "Failed to initialize camera with guid " << camList->ids[0].guid << endl;
        abort();
    }
    cout << "Using camera with GUID " << camera1->guid << endl;

    dc1394_camera_free_list(camList);
}


void MainDialog::onUdpPacketArrived()
{
	QByteArray		datagram;
    QHostAddress	sender;
    quint16			senderPort;
    ChunkAttrib		chunkAttrib;
    unsigned char*  dataSrc;
    int             fixedStimFrameSz;

	struct timespec	timestamp;
	uint64_t		msec;


    while (udpSocket->hasPendingDatagrams())
    {
        clock_gettime(CLOCK_REALTIME, &timestamp);
		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        switch(datagram.data()[0])
        {
        case UDP_AUDIO_PACKET:
        	chunkAttrib = *((ChunkAttrib*)(((unsigned char*)datagram.data())+1));
        	chunkAttrib.timestamp = msec;
            receiverAudioBuf->insertChunk(((unsigned char*)datagram.data())+sizeof(ChunkAttrib)+1, chunkAttrib);
            speakerBuffer->insertChunk(((unsigned char*)datagram.data())+sizeof(ChunkAttrib)+1);
            updateReceiverAudioBars(((unsigned char*)datagram.data())+sizeof(ChunkAttrib)+1);
        	break;

        case UDP_VIDEO_PACKET:
        	chunkAttrib = *((ChunkAttrib*)(((unsigned char*)datagram.data())+1));
        	chunkAttrib.timestamp = msec;

            if(isRec)
            {   // Check whether we might want to replace the frame with a frame from the fixedStimuli

                dataSrc = fixedStimuli->findFrame(startRecTstamp, chunkAttrib.timestamp, &fixedStimFrameSz);
                if(dataSrc)
                {
                    chunkAttrib.chunkSize = fixedStimFrameSz;
                }
                else
                {
                    dataSrc = ((unsigned char*)datagram.data())+sizeof(ChunkAttrib)+1;
                }
                }
            else
            {
                dataSrc = ((unsigned char*)datagram.data())+sizeof(ChunkAttrib)+1;
            }

            receiverVideoBuf->insertChunk(dataSrc, chunkAttrib);
        	break;

        default:
        	cerr << "Unknown UDP datagram type, dropping" << endl;
        }
    }
}


void MainDialog::updateReceiverAudioBars(unsigned char* _data)
{

	// TODO: unify with the similar code for the server
	unsigned int 	i=0;
	unsigned int	j;
	AUDIO_DATA_TYPE	maxvals[N_CHANS_RECEIVER]={0};
	AUDIO_DATA_TYPE	curval;

	// Update the history
	memset(&(volReceiverMaxvals[volReceiverIndNext]), 0, N_CHANS_RECEIVER * sizeof(AUDIO_DATA_TYPE));
	while(i < settings.framesPerPeriod * N_CHANS_RECEIVER)
	{
		for(j=0; j<N_CHANS_RECEIVER; j++)
		{
			curval = abs(((AUDIO_DATA_TYPE*)_data)[i++]);
			volReceiverMaxvals[volReceiverIndNext + j] = (volReceiverMaxvals[volReceiverIndNext + j] >= curval) ? volReceiverMaxvals[volReceiverIndNext + j] : curval;
		}
	}

	volReceiverIndNext += N_CHANS_RECEIVER;
	volReceiverIndNext %= (N_CHANS_RECEIVER * N_BUF_4_VOL_IND);

	// Compute maxima for all channels
	i = 0;
	while(i < N_CHANS_RECEIVER * N_BUF_4_VOL_IND)
	{
		for(j=0; j<N_CHANS_RECEIVER; j++)
		{
			curval = volReceiverMaxvals[i++];
			maxvals[j] = (maxvals[j] >= curval) ? maxvals[j] : curval;
		}
	}

	// Update only one level bar
	ui.receiverLevelLeft->setValue(maxvals[0]);
}


