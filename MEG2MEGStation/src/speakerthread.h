/*
 * speaker.h
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

#ifndef SPEAKERTHREAD_H_
#define SPEAKERTHREAD_H_

// Use the newer ALSA API
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>

#include "stoppablethread.h"
#include "nonblockingbuffer.h"
#include "settings.h"

class SpeakerThread : public StoppableThread
{
public:
	SpeakerThread(NonBlockingBuffer* _buffer);
	virtual ~SpeakerThread();

protected:
	virtual void stoppableRun();

private:
	snd_pcm_t*			sndHandle;
	NonBlockingBuffer*	buffer;
	Settings			settings;
};

#endif /* SPEAKERTHREAD_H_ */
