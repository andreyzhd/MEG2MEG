/*
 * settings.h
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

#ifndef SETTINGS_H_
#define SETTINGS_H_

//! Application-wide settings preserved across multiple invocations.
/*!
 * This class contains application-wide settings read from disc. To read the
 * settings simply create the instance of this class and read the values from
 * the corresponding public variables of the class. The settings are supposed
 * to be read only; they can be changed by manually editing the text file
 * between program invocations but they should stay constant for the whole
 * lifetime of a single program instance. The class should be completely
 * thread-safe (CURRENTLY IT IS NOT).
 */
class Settings {
	// TODO: make member variables immutable as much as possible
	// TODO: make the class thread-safe
	// TODO: implement singleton pattern?
public:
	Settings();

	// video
	int				jpgQuality;
	bool			color;
	bool			receiverRotate;
	bool			senderRotate;
	bool			highFps;

	// audio
	unsigned int	sampRate;
	unsigned int	framesPerPeriod;
	unsigned int	nSenderPeriods;
	unsigned int	nReceiverPeriods;
	unsigned int	spkBufSz;
	char			inpAudioDev[500];
	char			outAudioDev[500];

	// misc
	char			storagePath[500];
	char			udpRemoteReceiverAddr[500];
	unsigned int	udpRemoteReceiverPort;
	unsigned int	udpLocalReceiverPort;
	char			siteId;
};

#endif /* SETTINGS_H_ */
