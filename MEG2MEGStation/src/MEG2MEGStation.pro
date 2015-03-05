# Author: Andrey Zhdanov
# Copyright (C) 2015 Department of Neuroscience and Biomedical Engineering,
# Aalto University School of Science
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE = app
TARGET = MEG2MEGStation
QT += core \
    gui \
    widgets \
    network
HEADERS += sendervideodialog.h \
    receivervideodialog.h \
    settings.h \
    filewriter.h \
    videocompressorthread.h \
    stoppablethread.h \
    speakerthread.h \
    nonblockingbuffer.h \
    audiofilewriter.h \
    cycdatabuffer.h \
    microphonethread.h \
    videofilewriter.h \
    config.h \
    camerathread.h \
    videowidget.h \
    maindialog.h \
    sendingsocket.h \
    fixedstimuli.h
SOURCES += sendervideodialog.cpp \
    receivervideodialog.cpp \
    settings.cpp \
    filewriter.cpp \
    videocompressorthread.cpp \
    stoppablethread.cpp \
    speakerthread.cpp \
    nonblockingbuffer.cpp \
    audiofilewriter.cpp \
    cycdatabuffer.cpp \
    microphonethread.cpp \
    videofilewriter.cpp \
    camerathread.cpp \
    videowidget.cpp \
    main.cpp \
    maindialog.cpp \
    sendingsocket.cpp \
    fixedstimuli.cpp
FORMS += sendervideodialog.ui \
    receivervideodialog.ui \
    maindialog.ui
INCLUDEPATH += /usr/include/c++/4.4 \
    /usr/include \
    /usr/local/include
LIBS += -L/usr/local/lib \
    -L/usr/lib \
    -lasound \
    -ldc1394 \
    -ljpeg
RESOURCES += 
DEFINES += __STDC_LIMIT_MACROS
