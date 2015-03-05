/*
 * senderdialog.h
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

#ifndef SENDERVIDEODIALOG_H
#define SENDERVIDEODIALOG_H

#include <QDialog>
#include "ui_sendervideodialog.h"
#include "camerathread.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"
#include "videocompressorthread.h"
#include "sendingsocket.h"
#include "fixedstimuli.h"


class SenderVideoDialog : public QDialog
{
    Q_OBJECT

public:
    SenderVideoDialog(dc1394camera_t* _camera, int _cameraId, SendingSocket* _sendingSocket, QLineEdit* _suffix, FixedStimuli* _fixedStimuli, QWidget *parent = 0);
    virtual ~SenderVideoDialog();
    void setIsRec(bool _isRec);

public slots:
    void onShutterChanged(int _newVal);
    void onGainChanged(int _newVal);
    void onUVChanged(int _newVal);
    void onVRChanged(int _newVal);
    void onNewFrame(unsigned char* _jpegBuf);

    //! Stop all the threads associated with the dialog.
    /*!
     * Stop all the threads associated with the dialog so that the dialog can
     * be safely destroyed. This method should always be called before the
     * destructor.
     */
    void stopThreads();

private:
    Ui::SenderVideoDialogClass ui;

    dc1394camera_t*			camera;
    CameraThread*       	cameraThread;
    CycDataBuffer*			cycVideoBufRaw;
    CycDataBuffer*			cycVideoBufJpeg;
    VideoFileWriter*		videoFileWriter;
    VideoCompressorThread*	videoCompressorThread;
    SendingSocket*			sendingSocket;

    // This variables are used for showing the FPS
    uint64_t                prevFrameTstamp=0;
    int                     frameCnt=0;
};

#endif // SENDERVIDEODIALOG_H
