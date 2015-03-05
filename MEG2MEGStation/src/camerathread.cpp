/*
 * camerathread.cpp
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
#include <sched.h>
#include <time.h>
#include <QCoreApplication>
#include <stdlib.h>

#include "camerathread.h"
#include "config.h"

using namespace std;


CameraThread::CameraThread(dc1394camera_t* _camera, CycDataBuffer* _cycBuf, bool _color, bool _highFps)
{
    dc1394error_t 	err;
	struct timespec	timestamp;

    cycBuf = _cycBuf;
    color = _color;
    shouldStop = false;

    camera = _camera;

	// Init the chunk ID counter to unix epoch. This should guarantee
	// uniqueness of each chunk's ID even if the program is restarted.
	clock_gettime(CLOCK_REALTIME, &timestamp);
	curChunkId = timestamp.tv_nsec / 1000000;
	curChunkId += timestamp.tv_sec * 1000;

	preProcBuf = (unsigned char*)malloc(VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1));
	if (!preProcBuf)
	{
        cerr << "Could not allocate memory" << endl;
        abort();
	}

    /*-----------------------------------------------------------------------
     *  setup capture
     *-----------------------------------------------------------------------*/
    err = dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set operation mode" << endl;
        abort();
    }

    err = dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_800);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set iso speed" << endl;
        abort();
    }

    err = dc1394_video_set_mode(camera, (color ? DC1394_VIDEO_MODE_640x480_RGB8 : DC1394_VIDEO_MODE_640x480_MONO8));
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set video mode" << endl;
        abort();
    }

    err = dc1394_video_set_framerate(camera, (_highFps ? DC1394_FRAMERATE_60 : DC1394_FRAMERATE_30));
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not set framerate" << endl;
        abort();
    }

    err = dc1394_capture_setup(camera, N_CAMERA_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not setup camera-" << endl \
             << "make sure that the video mode and framerate are" << endl \
             << "supported by your camera" << endl;
        abort();
    }
}


CameraThread::~CameraThread()
{
	dc1394error_t err;

    err = dc1394_capture_stop(camera);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not stop video capture" << endl;
        abort();
    }

    free(preProcBuf);
}


void CameraThread::stoppableRun()
{
    dc1394error_t			err;
    dc1394video_frame_t*	frame;
    struct sched_param		sch_param;
	struct timespec			timestamp;
	uint64_t				msec;
	ChunkAttrib				chunkAttrib;

    /*-----------------------------------------------------------------------
     *  have the camera start sending us data
     *-----------------------------------------------------------------------*/
    err = dc1394_video_set_transmission(camera, DC1394_ON);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not start camera iso transmission" << endl;
        abort();
    }

    // Set priority
    sch_param.sched_priority = CAM_THREAD_PRIORITY;
    if (sched_setscheduler(0, SCHED_FIFO, &sch_param))
    {
    	cerr << "Cannot set camera thread priority. Continuing nevertheless, but don't blame me if you experience any strange problems." << endl;
    }

    // Start the acquisition loop
    while (!shouldStop)
    {
        err = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
		clock_gettime(CLOCK_REALTIME, &timestamp);

        if (err != DC1394_SUCCESS)
        {
            cerr << "Error dequeuing a frame" << endl;
            abort();
        }

		msec = timestamp.tv_nsec / 1000000;
		msec += timestamp.tv_sec * 1000;

		chunkAttrib.chunkSize = VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1);
		chunkAttrib.timestamp = msec;
		chunkAttrib.id = curChunkId++;

		memcpy(preProcBuf, frame->image, VIDEO_HEIGHT * VIDEO_WIDTH * (color ? 3 : 1));

		// draw the last 6 bits of the frame id onto the image as black and white squares
		//for (int j=0; j<6; j++)
		//{
		//	int lu_corner = (VIDEO_WIDTH * 28*16 + (38-4*j)*16) * (color ? 3 : 1);
		//
		//	for (int i=0; i<16; i++)
		//	{
		//		memset(preProcBuf+lu_corner+VIDEO_WIDTH*i*(color ? 3 : 1), ((chunkAttrib.id >> j) & 1)*255, 16*(color ? 3 : 1));
		//	}
		//}

		cycBuf->insertChunk(preProcBuf, chunkAttrib);

        err = dc1394_capture_enqueue(camera, frame);
        if (err != DC1394_SUCCESS)
        {
            cerr << "Error re-enqueuing a frame" << endl;
            abort();
        }
    }

    /*-----------------------------------------------------------------------
     *  have the camera stop sending us data
     *-----------------------------------------------------------------------*/
    err = dc1394_video_set_transmission(camera, DC1394_OFF);
    if (err != DC1394_SUCCESS)
    {
        cerr << "Could not stop camera iso transmission" << endl;
        abort();
    }
}

