/*
 * fixedstimuli.cpp
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
#include <fstream>
#include <QDir>

#include "fixedstimuli.h"

using namespace std;

FixedStimuli::FixedStimuli()
{
    ifstream   scriptFile;
    ifstream   frameFile;
    uint64_t   curOnset;
    uint64_t   curOffset;
    string     curFname;

    nFrames = 0;

    scriptFile.open((QDir::homePath() + FIXED_STIM_SCRIPT).toLatin1(), ios::in);
    if(scriptFile.fail())
    {
        cout << 'Cannot read the stimuli script file from ' << FIXED_STIM_SCRIPT << ', no scripted stimuli at this site' << endl;
        return;
    }

    while(true)
    {
        scriptFile >> curOnset;
        scriptFile >> curOffset;
        scriptFile >> curFname;

        if(scriptFile.fail())
        {
            break;
        }

        frameOnset[nFrames] = curOnset;
        frameOffset[nFrames] = curOffset;

        // Read the frame data
        frameFile.open((QDir::homePath() + FIXED_STIM_FLDR + QString::fromUtf8(curFname.c_str())).toLatin1(), ios::in | ios::binary);
        frameFile.seekg(0, ios::end);
        frameSize[nFrames] = frameFile.tellg();
        frameFile.seekg(0, ios::beg);

        frameData[nFrames] = (unsigned char*)malloc(frameSize[nFrames]);
        if (!frameData[nFrames])
        {
            cerr << 'Error allocating memory, aborting' << endl;
            abort();
        }

        frameFile.read((char*)frameData[nFrames], frameSize[nFrames]);
        frameFile.close();

        nFrames++;
        if (nFrames == MAX_SCRIPT_FRAMES)
        {
            cout << 'Maximum number of ' << MAX_SCRIPT_FRAMES << ' is reached, ignoring the rest of the ' << FIXED_STIM_SCRIPT << ' file' << endl;
            break;
        }
    }

    scriptFile.close();
}


FixedStimuli::~FixedStimuli()
{
    int i;

    for (i=0; i<nFrames; i++)
    {
        free(frameData[i]);
    }
}


unsigned char* FixedStimuli::findFrame(uint64_t _startRecTstamp, uint64_t _frameTstamp, int* _size)
{
    // TODO: this implementation is very inefficient. Will cause problems with large number of frames.

    int       i;
    u_int64_t d;

    d = _frameTstamp - _startRecTstamp;

    for(i=0; i<nFrames; i++)
    {
        if ((frameOnset[i] <= d) && (frameOffset[i] >= d))
        {
            *_size = frameSize[i];
            return(frameData[i]);
        }
    }

    // nothing found
    *_size = 0;
    return(NULL);
}
