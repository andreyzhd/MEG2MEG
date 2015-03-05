/*
 * sendervideodialog.cpp
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

#include "sendervideodialog.h"
#include "config.h"
#include "settings.h"

using namespace std;

SenderVideoDialog::SenderVideoDialog(dc1394camera_t* _camera, int _cameraId, SendingSocket* _sendingSocket, QLineEdit* _suffix, FixedStimuli* _fixedStimuli, QWidget *parent)
    : QDialog(parent)
{
	char		winCaption[500];
	Settings	settings;

	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
	sprintf(winCaption, "Camera %i", _cameraId);
	setWindowTitle(winCaption);
	camera = _camera;
	sendingSocket = _sendingSocket;

	// Set up video recording
	cycVideoBufRaw = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, false);
    cycVideoBufJpeg = new CycDataBuffer(CIRC_VIDEO_BUFF_SZ, false, _fixedStimuli);
    cameraThread = new CameraThread(camera, cycVideoBufRaw, settings.color, settings.highFps);
	videoFileWriter = new VideoFileWriter(cycVideoBufJpeg, settings.storagePath, settings.siteId, true, _suffix);
	videoCompressorThread = new VideoCompressorThread(cycVideoBufRaw, cycVideoBufJpeg, settings.color, settings.jpgQuality);
    ui.videoWidget->rotate = settings.senderRotate;

    QObject::connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), ui.videoWidget, SLOT(onDrawFrame(unsigned char*)));
	QObject::connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), sendingSocket, SLOT(sendVideoPacket(unsigned char*)));
    QObject::connect(cycVideoBufJpeg, SIGNAL(chunkReady(unsigned char*)), this, SLOT(onNewFrame(unsigned char*)));

	// Setup gain/shutter sliders
    ui.shutterSlider->setMinimum(SHUTTER_MIN_VAL);
    ui.shutterSlider->setMaximum(SHUTTER_MAX_VAL);

    ui.gainSlider->setMinimum(GAIN_MIN_VAL);
    ui.gainSlider->setMaximum(GAIN_MAX_VAL);

    ui.uvSlider->setMinimum(UV_MIN_VAL);
    ui.uvSlider->setMaximum(UV_MAX_VAL);
    ui.uvSlider->setEnabled(settings.color);

    ui.vrSlider->setMinimum(VR_MIN_VAL);
    ui.vrSlider->setMaximum(VR_MAX_VAL);
    ui.vrSlider->setEnabled(settings.color);

    ui.uvLabel->setEnabled(settings.color);
    ui.vrLabel->setEnabled(settings.color);
    ui.wbLabel->setEnabled(settings.color);

    // Start video running
    videoFileWriter->start();
    videoCompressorThread->start();
    cameraThread->start();
}


SenderVideoDialog::~SenderVideoDialog()
{
	delete cycVideoBufRaw;
	delete cycVideoBufJpeg;
	delete cameraThread;
	delete videoFileWriter;
	delete videoCompressorThread;
}


void SenderVideoDialog::stopThreads()
{
	// The piece of code stopping the threads should execute fast enough,
	// otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
	// order of stopping the threads is important.
	videoFileWriter->stop();
	videoCompressorThread->stop();
	cameraThread->stop();
}


void SenderVideoDialog::onShutterChanged(int _newVal)
{
	dc1394error_t	err;

	if (!cameraThread)
	{
		return;
	}

	err = dc1394_set_register(camera, SHUTTER_ADDR, _newVal + SHUTTER_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set shutter register" << endl;
        //abort();
    }
}


void SenderVideoDialog::onGainChanged(int _newVal)
{
	dc1394error_t	err;

	if (!cameraThread)
	{
		return;
	}

	err = dc1394_set_register(camera, GAIN_ADDR, _newVal + GAIN_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set gain register" << endl;
        //abort();
    }
}


void SenderVideoDialog::onUVChanged(int _newVal)
{
	dc1394error_t	err;

	if (!cameraThread)
	{
		return;
	}

	// Since UV and VR live in the same register, we need to take care of both
	err = dc1394_set_register(camera, WHITEBALANCE_ADDR, _newVal * UV_REG_SHIFT + ui.vrSlider->value() + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set white balance register" << endl;
        //abort();
    }
}


void SenderVideoDialog::onVRChanged(int _newVal)
{
	dc1394error_t	err;

	if (!cameraThread)
	{
		return;
	}

	// Since UV and VR live in the same register, we need to take care of both
	err = dc1394_set_register(camera, WHITEBALANCE_ADDR, _newVal + UV_REG_SHIFT * ui.uvSlider->value() + WHITEBALANCE_OFFSET);

    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set white balance register" << endl;
        //abort();
    }
}


void SenderVideoDialog::onNewFrame(unsigned char* _jpegBuf)
{
    ChunkAttrib chunkAttrib;
    float       fps;
    char        fpsLabelBuff[100];

    chunkAttrib = *((ChunkAttrib*)(_jpegBuf-sizeof(ChunkAttrib)));

    if (prevFrameTstamp)
    {
        fps = 1 / (float(chunkAttrib.timestamp - prevFrameTstamp) / 1000);
        sprintf(fpsLabelBuff, "FPS: %02.01f", fps);

        // Update FPS every 10 frames to make it smoother
        if(frameCnt == 10)
        {
            ui.fpsLabel->setText(fpsLabelBuff);
            frameCnt = 0;
        }
    }

    prevFrameTstamp = chunkAttrib.timestamp;
    frameCnt++;
}


void SenderVideoDialog::setIsRec(bool _isRec)
{
	cycVideoBufJpeg->setIsRec(_isRec);
}
