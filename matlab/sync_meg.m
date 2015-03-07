%function [fiffinfo,data]=sync_meg(...);
%
% Data synchronization script for the meg2meg project.
%
% Takes two meg2meg fiff files fn1 and fn2 from a simultaneous recording,
% synchronizes and cuts them to equal length, and writes the result to a
% .mat-file as a single matrix containing the channels from both sites.
%
% Correction of meg2meg PC clocks across sites is based on audio file
% timestamps (NTP-type method that estimates network delay and clock
% discrepancy). Audio files matching the specified fiff files are
% automatically found, given a directory. Directories auddir1 and auddir2
% must contain the audio files (.aud) corresponding to fn1 and fn2 (or all
% audio files can be located in the same directory, in which case specify
% the same directory twice).
%
% Previously, PC clocks were synchronized to GPS receivers. For such
% recordings, no clock correction is needed; specify empty auddir1 and
% auddir2 for no clock adjustments. Note that this will lead to erroneous
% results, if the clocks were not synchronized during the recording. If the
% PPS signal from a GPS receiver is recorded on a trigger channel, it can
% always be used to verify the result: after synchronization, the PPS
% signals from the two sites should stay aligned throughout the recording,
% within about 0-2 ms.
%
% After possible clock correction, the data are synchronized by upsampling
% the data from the other site, followed by linear interpolation into a
% common time grid.
%
% Some arguments can be omitted. Possible calling conventions:
%
% Read only specified channels until given time and sync, write output file
% (or give ch_sel1={} and ch_sel2={} to read all channels)
%[fiffinfo,data]=sync_meg(fn1,fn2,auddir1,auddir2,outfile,ch_sel1,ch_sel2,tmax);
%
% Read specified channels only and sync, write output (or give
% outfile='' for no output)
%[fiffinfo,data]=sync_meg(fn1,fn2,auddir1,auddir2,outfile,ch_sel1,ch_sel2);
%
% Read all data and sync, write output
%[fiffinfo,data]=sync_meg(fn1,fn2,auddir1,auddir2,outfile);
%
% Read all data and sync, don't write output
%[fiffinfo,data]=sync_meg(fn1,fn2,auddir1,auddir2);
%
% Input arguments: 
%
% fn1, fn2             fiff filenames for each site
% auddir1, auddir2     directories containing .aud files for each site
% tmax                 (optional) read until time tmax (s); use inf or [] to read all.
% outfile              (optional) name of .mat file for the results. If empty string,
%                      no output file is created.
% ch_sel1, ch_sel2     (optional) cell arrays of channel names to be loaded for each site:
%                      e.g. {'MEG 0121','STI 005'}
%
% Output file:
%
% data                 matrix with all MEG data aligned sample by sample, 
%                      containing channel from fn1 and fn2, in that order
%
% fiffinfo             struct with the following fields:
%
% name1,name2          names of original fiff files
% info1,info2          original info structures
% chs1,chs2            channel names in the data matrix 
%                      (contains first chs1 and then chs2)
% sfreq                common sampling frequency
% ts                   recomputed per-sample timestamps
%
%
% Requires in Matlab path:
%
% MNE toolbox
% Signal processing toolbox
% meg2meg Matlab scripts
%
% Needs Matlab 7.3 or newer
%

%--------------------------------------------------------------------------
%   Copyright (C) 2015 Department of Neuroscience and Biomedical Engineering,
%   Aalto University School of Science
%
%   This program is free software: you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation, version 3.
%
%   This program is distributed in the hope that it will be useful,
%   but WITHOUT ANY WARRANTY; without even the implied warranty of
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%   GNU General Public License for more details.
%
%   You should have received a copy of the GNU General Public License
%   along with this program.  If not, see <http://www.gnu.org/licenses/>.
%--------------------------------------------------------------------------

function [fiffinfo,data]=sync_meg(varargin);

% Give warning if recording start times differ by more than this
OFFSET_WARNING=120;  % seconds 
% TODO: Acceptable error for synchronization (ms). If the expected time
% discrepancy at end of recording (computed as (fs1-fs2)*N) is smaller
% than this, synchronization is not performed.
MAX_SYNC_ERR=3;  % ms
% Upsampling factor before linear interpolation. 5 is safe for all
% circumstances (gives 40 dB SNR in worst case).
USF=5;

switch nargin
  case 4
    outfile='';
    use_sel=false;
    tmax=inf;
  case 5
    use_sel=false;
    outfile=varargin{5};
    tmax=inf;
  case 7
    use_sel=true;
    outfile=varargin{5};
    ch_sel1=varargin{6};
    ch_sel2=varargin{7};
    tmax=inf;
  case 8
    use_sel=true;
    outfile=varargin{5};
    ch_sel1=varargin{6};
    ch_sel2=varargin{7};
    tmax=varargin{8};
  otherwise
    error('sync_meg: wrong number of input arguments');
end

% these must always be supplied
fn1=varargin{1};
fn2=varargin{2};
auddir1=varargin{3};
auddir2=varargin{4};
    
if ~exist('fiff_open')
  error('sync_meg: please check that the MNE toolbox is in your Matlab path.');
end
if ~exist('comp_tstamps_2')
  error('sync_meg: please check that meg2meg Matlab functions are in your Matlab path.');
end

if ~isempty(auddir1)
  if ~exist(auddir1,'dir') | ~exist(auddir2,'dir')
    error('sync_meg: one of the specified audio directories does not exist');
  end
end

% load meg data and find matching audio files
if use_sel
  [fiff1,sender1,receiver1]=loadmeg_findaud(fn1,auddir1,tmax,ch_sel1);
  [fiff2,sender2,receiver2]=loadmeg_findaud(fn2,auddir2,tmax,ch_sel2);
else
  [fiff1,sender1,receiver1]=loadmeg_findaud(fn1,auddir1,tmax);
  [fiff2,sender2,receiver2]=loadmeg_findaud(fn2,auddir2,tmax);
end

if fiff1.site_id~=-1
  if fiff1.site_id==fiff2.site_id
    error('sync_meg: the fiff files seem to be from the same site, how come?');
  end
end

% Using audio files, calculate the linear time correction coefficients
% between the two sites. 
if ~isempty(auddir1)
  % check for site id
  if fiff1.site_id==-1 | fiff2.site_id==-1
    error('sync_meg: cannot identify sites from the fiff files. This is needed for audio-based sync.');
  end
  [tcorr,offset]=comp_time_corr(sender1,sender2,receiver1,receiver2);
  fprintf('Clock correction: tcorr %f %f offset %f\n',tcorr,offset);
  % Use the above coefficients to recompute per-sample timestamps for
  % site 2; this puts the sites into a common time frame of
  % reference. Sample timestamps will still disagree, due to different
  % sampling clocks.
  [fiff2.ts,nts,fiff2.sfest]=comp_tstamps_2(fiff2.tschan,fiff2.meta.info.sfreq,tcorr,offset);
else
  fprintf('\nsync_meg: WARNING: no audio specified; will not compute correction for PC clocks\n');
  fprintf('Please be sure that the clocks were kept synchronized during the recording (e.g. by NTP+GPS).\n\n');
end

fiff1.os=fiff1.ts(1);  % offset (absolute start of data)
fiff1.endt=fiff1.ts(end);  % end of data
fiff2.os=fiff2.ts(1);
fiff2.endt=fiff2.ts(end);

fprintf('sync_meg: start to align files.\n');
osdiff=abs(fiff1.os-fiff2.os)/1e3;
fprintf('Start times differ by %g s\n', osdiff);
if osdiff>OFFSET_WARNING
  fprintf('WARNING: these recordings seem to have a large offset difference.\n');
end
if max([fiff1.os fiff2.os])>min([fiff1.endt fiff2.endt])
  error('sync_meg: fiff files have no time overlap. Cannot sync. Probably you have the wrong files.');
end

% eliminate common offset in timestamps
osmin=min(fiff1.os,fiff2.os);
fiff1.ts=fiff1.ts-osmin;
fiff2.ts=fiff2.ts-osmin;

% arrange things so that fiff2 has the higher sampling frequency
% (it will be resampled)
is_swapped = (fiff1.meta.info.sfreq > fiff2.meta.info.sfreq);
if is_swapped
  fifftmp=fiff1;
  fiff1=fiff2;
  fiff2=fifftmp;
  clear fifftmp;
end

% Check for difference in sampling freqs.  TODO: If the expected drift
% is small enough, synchronization can be skipped (align & cut only)
sfdiff=(fiff2.sfest-fiff1.sfest)/fiff2.sfest;
fprintf('Sampling freqs differ by %g ppm\n',1e6*sfdiff);

% match signal starting points
fprintf('Matching signal start times...\n');
% time where we start the synced data
startT=max(fiff1.ts(1),fiff2.ts(1));  
fiff1_startind=nearest_ind(fiff1.ts,startT);
fiff2_startind=nearest_ind(fiff2.ts,startT);
tdiff=fiff1.ts(fiff1_startind)-fiff2.ts(fiff2_startind);
%fprintf('Initial timing discrepancy: %g ms\n',min(tdiff));
% cut signals to approximately equal length
% fiff2 is interpolated to fiff1 time grid,
% so fiff1 time grid must be contained inside fiff2 grid
endT=min(fiff1.ts(end),fiff2.ts(end));
fiff2_endind=nearest_ind(fiff2.ts,endT);
fiff1_endind=nearest_smaller_ind(fiff1.ts,fiff2.ts(fiff2_endind));
fiff1.datacut=fiff1.data(:,fiff1_startind:fiff1_endind);
fiff2.datacut=fiff2.data(:,fiff2_startind:fiff2_endind);
fiff1.tscut=fiff1.ts(:,fiff1_startind:fiff1_endind);
fiff2.tscut=fiff2.ts(:,fiff2_startind:fiff2_endind);
% drop uncut data to save memory
clear('fiff1.data');
clear('fiff2.data');

% interpolate fiff2 to fiff1 time grid
fprintf('Range for %s: ',filename(fiff1.name));
fprintf('%g s-%g s\n',fiff1.tscut(1)/1e3,fiff1.tscut(end)/1e3);
fprintf('Range for %s (to be interpolated): ',filename(fiff2.name));
fprintf('%g-%g s\n',fiff2.tscut(1)/1e3,fiff2.tscut(end)/1e3);
fprintf('Interpolating total of %d channels...\n',fiff2.meta.info.nchan);
for k=1:fiff2.meta.info.nchan,
  if ~mod(k,40),
    fprintf('Interpolating channel %d/%d...\n',k,fiff2.meta.info.nchan);
  end
  % digital trigger channels are processed with nearest-neighbor
  % interpolation, to avoid distorting the pulses
  if ~isempty(strfind(fiff2.meta.info.ch_names{1},'STI'))
    fiff2.data_interp(k,:)=interp1(fiff2.tscut,fiff2.datacut(k,:),fiff1.tscut,'nearest');
  else
  % upsample (interpolate) before interp1 to avoid distortion
  dataos=resample(fiff2.datacut(k,:),USF,1);
  % compute corresponding new time grid
  tsos=fiff2.tscut(1):1000/(USF*fiff2.sfest):fiff2.tscut(end)+1;
  tsos=tsos(1:length(dataos));
  % only linear ip - causes distortion for high freqs
  %fiff2.data_interp(k,:)=interp1(fiff2.tscut,fiff2.datacut(k,:),fiff1.tscut);
  % upsample + linear ip
  fiff2.data_interp(k,:)=interp1(tsos,dataos,fiff1.tscut);
  end
end
fiff2.datacut=fiff2.data_interp;

% compute original timestamps (unix epoch) for the cut data
ts=osmin+startT+(0:1000/fiff1.sfest:(length(fiff1.tscut)-1)*1000/fiff1.sfest);

% write matrix with all MEG channels + relevant meta info
fiffinfo.sfreq=fiff1.sfest;

% swap the datasets back to the original order if necessary
if is_swapped
  fifftmp=fiff1;
  fiff1=fiff2;
  fiff2=fifftmp;
  clear fifftmp;
end

% write result .mat file
if strcmp(outfile, '')
    fprintf('Empty output file name, will not create an output file.\n');
else
    fprintf('Saving data in %s...\n',outfile);
end
fiffinfo.name1=fiff1.name;
fiffinfo.name2=fiff2.name;
fiffinfo.info1=fiff1.meta;
fiffinfo.info2=fiff2.meta;
fiffinfo.chs1=fiff1.meta.info.chs;
fiffinfo.chs2=fiff2.meta.info.chs;
% generate list of all channels
Nch1=length(fiffinfo.chs1);
Nch2=length(fiffinfo.chs2);
fiffinfo.chnames=cell(Nch1+Nch2,1);
for k=1:Nch1,
  fiffinfo.chnames{k}=fiffinfo.chs1(k).ch_name;
end
for k=1:Nch2,
  fiffinfo.chnames{Nch1+k}=fiffinfo.chs2(k).ch_name;
end
fiffinfo.ts=ts;
data=[fiff1.datacut; fiff2.datacut];
% 7.3 format required for large arrays
if ~strcmp(outfile, '')
    save(outfile,'fiffinfo','data','-v7.3'); 
end

% great success (hopefully)
fprintf('sync_meg: done.\n');




