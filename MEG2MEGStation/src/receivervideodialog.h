/*
 * receiverdialog.h
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

#ifndef RECEIVERVIDEODIALOG_H
#define RECEIVERVIDEODIALOG_H

#include <QDialog>
#include "ui_receivervideodialog.h"
#include "cycdatabuffer.h"
#include "videofilewriter.h"



class ReceiverVideoDialog : public QDialog
{
    Q_OBJECT

public:
    ReceiverVideoDialog(CycDataBuffer* _cycVideoBuf, QLineEdit* _suffix, QWidget *parent = 0);
    virtual ~ReceiverVideoDialog();
    void setIsRec(bool _isRec);

public slots:

    //! Stop all the threads associated with the dialog.
    /*!
     * Stop all the threads associated with the dialog so that the dialog can
     * be safely destroyed. This method should always be called before the
     * destructor.
     */
    void stopThreads();

private:
    Ui::ReceiverVideoDialogClass ui;

    CycDataBuffer*			cycVideoBuf;
    VideoFileWriter*		videoFileWriter;
};

#endif // RECEIVERVIDEODIALOG_H
