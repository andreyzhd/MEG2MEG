/*
 * config.h
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

#ifndef CONFIG_H_

// Camera configuration
#define VIDEO_DEV_PATH   	"/dev/video0"
#define N_CAMERA_BUFFERS 	1
#define VIDEO_HEIGHT     	480
#define VIDEO_WIDTH      	640

#define SHUTTER_ADDR		0xf0081c
#define SHUTTER_MIN_VAL		1
#define SHUTTER_MAX_VAL		0xfff
#define SHUTTER_OFFSET		0x82000000

#define GAIN_ADDR			0xf00820
#define GAIN_MIN_VAL		0
#define GAIN_MAX_VAL		0x2a8
#define GAIN_OFFSET			0x82000000

#define WHITEBALANCE_ADDR	0xf0080c
#define WHITEBALANCE_OFFSET	0x82000000
#define UV_MIN_VAL			0
#define UV_MAX_VAL			0x238
#define VR_MIN_VAL			0
#define VR_MAX_VAL			0x238
#define UV_REG_SHIFT		0x1000

// Audio configuration
#define N_CHANS_SENDER		2			// stereo
#define N_CHANS_RECEIVER	1			// mono
#define	N_BUF_4_VOL_IND		10			// number of buffers used by volume indicator
#define AUDIO_FORMAT		SND_PCM_FORMAT_S16_LE	// from <alsa/asoundlib.h>
#define	AUDIO_DATA_TYPE		int16_t					// should match AUDIO_FORMAT
#define	MAX_AUDIO_VAL		INT16_MAX				// should match AUDIO_FORMAT

#define AUDIO_FILE_VERSION	3
#define VIDEO_FILE_VERSION	3

#define MAGIC_VIDEO_STR		"ELEKTA_VIDEO_FILE"
#define MAGIC_AUDIO_STR		"ELEKTA_AUDIO_FILE"

#define UDP_AUDIO_PACKET	1
#define UDP_VIDEO_PACKET	2

// Buffer parameters
#define CIRC_BUF_MARG		0.2			// When less than this fraction of the buffer
										// is left free, overflow error is generated.
										// Used as a safety measure against race
										// condition between consumer and producer
										// threads when the circular buffer is close
										// to full.

#define MAX_CHUNK_SIZE		(CIRC_BUF_MARG/10)
										// Maximum chunk size as a fraction of the
										// circular buffer size. Should be at least
										// several times smaller than CIRC_BUF_MARG
										// for the buffer to work properly.

// Buffer sizes
#define CIRC_VIDEO_BUFF_SZ	100000000	// in bytes
#define CIRC_AUDIO_BUFF_SZ	100000000	// in bytes

// Thread priorities
#define CAM_THREAD_PRIORITY	10
#define MIC_THREAD_PRIORITY	15
#define SPK_THREAD_PRIORITY 5

// Used for storing application settings
#define ORG_NAME "Elekta"
#define APP_NAME "VideoRecStation_M2M"

#define SITE_ID_PATH "/.config/Elekta/meg2meg_site_id"   // Relative to the current user's home folder
#define FIXED_STIM_FLDR "/fixed_stim/"   // Relative to the current user's home folder
#define FIXED_STIM_SCRIPT "/fixed_stim/script.txt"   // Relative to the current user's home folder

#define CONFIG_H_


#endif /* CONFIG_H_ */
