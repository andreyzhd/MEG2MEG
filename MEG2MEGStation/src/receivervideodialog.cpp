/*
 * receivervideodialog.cpp
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

#include "receivervideodialog.h"
#include "config.h"
#include "settings.h"

using namespace std;

ReceiverVideoDialog::ReceiverVideoDialog(CycDataBuffer* _cycVideoBuf, QLineEdit* _suffix, QWidget *parent)
    : QDialog(parent)
{
	Settings	settings;

	ui.setupUi(this);
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint| Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
	setWindowTitle(NULL);

	// Set up video recording
	cycVideoBuf = _cycVideoBuf;
	videoFileWriter = new VideoFileWriter(cycVideoBuf, settings.storagePath, settings.siteId, false, _suffix);
    ui.videoWidget->rotate = settings.receiverRotate;
    QObject::connect(cycVideoBuf, SIGNAL(chunkReady(unsigned char*)), ui.videoWidget, SLOT(onDrawFrame(unsigned char*)));

    // Start video running
    videoFileWriter->start();
}


ReceiverVideoDialog::~ReceiverVideoDialog()
{
	delete cycVideoBuf;
	delete videoFileWriter;
}


void ReceiverVideoDialog::stopThreads()
{
	// The piece of code stopping the threads should execute fast enough,
	// otherwise cycVideoBufRaw or cycVideoBufJpeg buffer might overflow. The
	// order of stopping the threads is important.
	videoFileWriter->stop();
}

