/*
 * settings.cpp
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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <QSettings>
#include <QDir>

#include "settings.h"
#include "config.h"

using namespace std;

Settings::Settings()
{
	QSettings settings(ORG_NAME, APP_NAME);

	//---------------------------------------------------------------------
	// Read the site ID
	//

    // TODO add support for 2-digit site ID, better checking
	ifstream siteIdFile;

    siteIdFile.open((QDir::homePath() + SITE_ID_PATH).toLatin1(), ios::in | ios::binary);
    if(siteIdFile.fail())
    {
        cerr << 'Error reading the site ID from ' << SITE_ID_PATH << ', aborting' << endl;
        abort();
    }

	siteIdFile.read(&siteId, 1);
	if(siteIdFile.fail())
	{
		cerr << 'Error reading the site ID from ' << SITE_ID_PATH << ', aborting' << endl;
		abort();
	}

	siteIdFile.close();

	siteId -= 48;	// translate an ascii code of a digit to a number
	if((siteId < 0) || (siteId > 3))
	{
		cerr << 'Invalid site ID: ' << siteId << ', should be between 0 and 3' << endl;
		abort();
	}


	//---------------------------------------------------------------------
	// Video settings
	//

	// JPEG quality
	if(!settings.contains("video/jpeg_quality"))
	{
		settings.setValue("video/jpeg_quality", 80);
		jpgQuality = 80;
	}
	else
	{
		jpgQuality = settings.value("video/jpeg_quality").toInt();
	}

	// Use color mode
	if(!settings.contains("video/color"))
	{
		settings.setValue("video/color", true);
		color = true;
	}
	else
	{
		color = settings.value("video/color").toBool();
	}

	// Rotate sender window
	if(!settings.contains("video/sender_rotate"))
	{
		settings.setValue("video/sender_rotate", false);
		senderRotate = false;
	}
	else
	{
		senderRotate = settings.value("video/sender_rotate").toBool();
	}

	// Rotate receiver window
	if(!settings.contains("video/receiver_rotate"))
	{
		settings.setValue("video/receiver_rotate", false);
		receiverRotate = false;
	}
	else
	{
		receiverRotate = settings.value("video/receiver_rotate").toBool();
	}

	// Use high (60 FPS) frame rate
	if(!settings.contains("video/high_fps"))
	{
		settings.setValue("video/high_fps", false);
		highFps = true;
	}
	else
	{
		highFps = settings.value("video/high_fps").toBool();
	}


	//---------------------------------------------------------------------
	// Audio settings
	//

	// Sampling rate
	if(!settings.contains("audio/sampling_rate"))
	{
		settings.setValue("audio/sampling_rate", 48000);
		sampRate = 48000;
	}
	else
	{
		sampRate = settings.value("audio/sampling_rate").toInt();
	}

	// Frames per period
	if(!settings.contains("audio/frames_per_period"))
	{
		settings.setValue("audio/frames_per_period", 96);
		framesPerPeriod = 96;
	}
	else
	{
		framesPerPeriod = settings.value("audio/frames_per_period").toInt();
	}

	// Number of periods - server
	if(!settings.contains("audio/num_sender_periods"))
	{
		settings.setValue("audio/num_sender_periods", 2);
		nSenderPeriods = 10;
	}
	else
	{
		nSenderPeriods = settings.value("audio/num_sender_periods").toInt();
	}

	// Number of periods - client
	if(!settings.contains("audio/num_receiver_periods"))
	{
		settings.setValue("audio/num_receiver_periods", 4);
		nReceiverPeriods = 10;
	}
	else
	{
		nReceiverPeriods = settings.value("audio/num_receiver_periods").toInt();
	}

	// Speaker buffer size (in frames)
	if(!settings.contains("audio/speaker_buffer_size"))
	{
		settings.setValue("audio/speaker_buffer_size", 4);
		spkBufSz = 4;
	}
	else
	{
		spkBufSz = settings.value("audio/speaker_buffer_size").toInt();
	}

	// Input audio device
	if(!settings.contains("audio/input_audio_device"))
	{
		settings.setValue("audio/input_audio_device", "default");
		sprintf(inpAudioDev, "default");
	}
	else
	{
		sprintf(inpAudioDev, settings.value("audio/input_audio_device").toString().toLocal8Bit().data());
	}

	// Output audio device
	if(!settings.contains("audio/output_audio_device"))
	{
		settings.setValue("audio/output_audio_device", "default");
		sprintf(outAudioDev, "default");
	}
	else
	{
		sprintf(outAudioDev, settings.value("audio/output_audio_device").toString().toLocal8Bit().data());
	}


	//---------------------------------------------------------------------
	// Misc settings
	//

	// Data storage folder
	if(!settings.contains("misc/data_storage_path"))
	{
		settings.setValue("misc/data_storage_path", "/videodat_M2M");
		sprintf(storagePath, "/videodat_M2M");
	}
	else
	{
		sprintf(storagePath, settings.value("misc/data_storage_path").toString().toLocal8Bit().data());
	}

	// UDP client address
	if(!settings.contains("network/udp_remote_receiver_address"))
	{
		settings.setValue("network/udp_remote_receiver_address", "127.0.0.1");
		sprintf(udpRemoteReceiverAddr, "127.0.0.1");
	}
	else
	{
		sprintf(udpRemoteReceiverAddr, settings.value("network/udp_remote_receiver_address").toString().toLocal8Bit().data());
	}

	// Remote UDP client port
	if(!settings.contains("network/udp_remote_receiver_port"))
	{
		settings.setValue("network/udp_remote_receiver_port", 2222);
		udpRemoteReceiverPort = 2222;
	}
	else
	{
		udpRemoteReceiverPort = settings.value("network/udp_remote_receiver_port").toInt();
	}

	// Local UDP client port
	if(!settings.contains("network/udp_local_receiver_port"))
	{
		settings.setValue("network/udp_local_receiver_port", 2222);
		udpLocalReceiverPort = 2222;
	}
	else
	{
		udpLocalReceiverPort = settings.value("network/udp_local_receiver_port").toInt();
	}
}

