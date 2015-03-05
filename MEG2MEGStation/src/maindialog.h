/*
 * maindialog.h
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

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <stdint.h>
#include <QMainWindow>

#include "config.h"
#include "ui_maindialog.h"
#include "camerathread.h"
#include "microphonethread.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "audiofilewriter.h"
#include "speakerthread.h"
#include "videocompressorthread.h"
#include "sendervideodialog.h"
#include "settings.h"
#include "sendingsocket.h"
#include "receivervideodialog.h"
#include "fixedstimuli.h"

class MainDialog : public QMainWindow
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = 0);
    ~MainDialog();

public slots:
    void onStartRec();
    void onStopRec();
    void onAudioUpdate(unsigned char* _data);
    void onUdpPacketArrived();

private:
    void initVideo();

    Ui::MainDialogClass ui;
	Settings 			settings;
    FixedStimuli*        fixedStimuli;


    //---------------------------------------------------------------------
	// Server stuff
	//
    dc1394camera_t*		camera1;
	SenderVideoDialog*	senderVideoDialog;

    MicrophoneThread*	microphoneThread;
    CycDataBuffer*		senderAudioBuf;
    AudioFileWriter*	senderAudioFileWriter;
	SendingSocket*		sendingSocket;

    // Data structures for volume indicator. volMaxvals is a cyclic buffer
    // that stores maximal values for the last N_BUF_4_VOL_IND periods for
    // all channels in an interleaved fashion.
	AUDIO_DATA_TYPE		volSenderMaxvals[N_CHANS_SENDER * N_BUF_4_VOL_IND];
	int					volSenderIndNext;


	//---------------------------------------------------------------------
	// Client stuff
	//
	void updateReceiverAudioBars(unsigned char* _data);

	ReceiverVideoDialog*	receiverVideoDialog;

    CycDataBuffer*			receiverAudioBuf;
    CycDataBuffer*			receiverVideoBuf;
    AudioFileWriter*		receiverAudioFileWriter;
    NonBlockingBuffer*		speakerBuffer;
	SpeakerThread*			speakerThread;
	QUdpSocket*				udpSocket;

	AUDIO_DATA_TYPE			volReceiverMaxvals[N_CHANS_RECEIVER * N_BUF_4_VOL_IND];
	int						volReceiverIndNext;

    volatile bool           isRec;
    volatile uint64_t       startRecTstamp;
};

#endif // MAINDIALOG_H
