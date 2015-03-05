/*
 * fixedstimuli.h
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

#ifndef FIXEDSTIMULI_H
#define FIXEDSTIMULI_H

#include <stdint.h>
#include "config.h"

#define MAX_SCRIPT_FRAMES 20000  // The maximum number of different pictures that we can show

//! Fixed stimuli sequence.
/*!
 * This class contains the fixed stimuli sequence data. To make it thread-safe,
 * the class is only "read-only": the class data structures should not be
 * modified outside of constructor.
 */
class FixedStimuli
{
public:
    FixedStimuli();
    virtual ~FixedStimuli();
    unsigned char* findFrame(uint64_t _startRecTstamp, uint64_t _frameTstamp, int* _size);

private:
    int            nFrames;
    uint64_t       frameOnset[MAX_SCRIPT_FRAMES];
    uint64_t       frameOffset[MAX_SCRIPT_FRAMES];
    unsigned char* frameData[MAX_SCRIPT_FRAMES];
    int            frameSize[MAX_SCRIPT_FRAMES];
};

#endif // FIXEDSTIMULI_H
