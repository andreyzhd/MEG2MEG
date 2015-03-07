%function [fiff,audsender,audreceiver]=loadmeg_findaud(fn,audiodir,tmax,ch_sel)
%
% Loads fiff and returns a data structure with data, timestamps etc.  
% Can also find corresponding best matching audio files (sender and receiver) for 
% e.g. correction of timestamps. This script does not correct timestamps. 
%
% Input:
% fn       input file
% tmax     max time to read (inf or empty to read all data)
% ch_sel      (optional) cell array containing the list of channel names to load
%             or {} to read everything
% audiodir   audio file directory to scan (empty to ignore audio files)
% 
% Returns structure 'fiff', with fields:
% name    file name
% meta    structure returned by fiff_setup_read_raw
% data    MEG data matrix
% times   times/sample from fiff_read_raw_segment
% tschan  data from timestamp channel
% ts      timestamps per sample
% sfest   estimated sampling freq
% site_id meg2meg site id
%
% If audiodir specified, also returns names of best matching audio files: 
% aud_sender 
% aud_receiver
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

function [fiff,aud_sender,aud_receiver]=loadmeg_findaud(varargin)

% potential trigger channels to scan
% TODO: TRIUX triggers may potentially appear only on STI201?
% STI 014 is needed for legacy systems (BioMag)
TRIG_CHANNELS={'STI 014','STI101'};  
% bits to scan on a combination channel
MAX_BITS=16; 

switch nargin
  case 2
    tmax=inf;
    ch_sel=[];
    use_sel=false;
  case 3
    tmax=varargin{3};
    ch_sel=[];
    use_sel=false;
  case 4
    tmax=varargin{3};
    ch_sel=varargin{4};
    use_sel=true;
  otherwise
    error('Incorrect number of input arguments');
end

fn=varargin{1};
audiodir=varargin{2};

% read uncorrected timestamps
tcorr=[1 0];
offset=0;

fiff.name=fn;
fiff.meta=fiff_setup_read_raw(fn);

if isfinite(tmax) || isempty(tmax)
  last_samp=fiff.meta.first_samp+ceil(tmax*fiff.meta.info.sfreq);
else
  last_samp=fiff.meta.last_samp;
end

% load selected channels
if isempty(ch_sel)
  use_sel=false;
end
if use_sel
    sel_chans = channelns(ch_sel, fiff.meta.info.ch_names);
    [fiff.data,fiff.times]=fiff_read_raw_segment(fiff.meta,fiff.meta.first_samp,last_samp,sel_chans);
    % update fiff.meta to reflect the new number of channels
    fiff.meta.cals = fiff.meta.cals(sel_chans);
    fiff.meta.info.nchan = length(sel_chans);
    fiff.meta.info.chs = fiff.meta.info.chs(sel_chans);
    fiff.meta.info.ch_names = fiff.meta.info.ch_names(sel_chans);
else
    [fiff.data,fiff.times]=fiff_read_raw_segment(fiff.meta,fiff.meta.first_samp,last_samp);
end
fprintf('loadmeg_findaud: read %g MiB of fiff data\n',8*numel(fiff.data)/2^20);

% identify trigger channel
tschanind=[];
k=1;
while isempty(tschanind) && k <= length(TRIG_CHANNELS),
  tschanind=strmatch(TRIG_CHANNELS{k},fiff.meta.info.ch_names);
  k=k+1;
end
  if isempty(tschanind)
    error('loadmeg_findaud: no trigger channel found; please include it if using channel selections');
  end
tschan=fiff.meta.info.ch_names{tschanind};
fprintf('loadmeg_findaud: using trigger channel: %s\n',tschan);

% pick trigger channel
fiff.tschan=fiff.data(tschanind,:);

% check for integer values on trigger channel
tvals=unique(fiff.tschan);
if ~isequal(tvals,round(tvals))
  error('loadmeg_findaud: trigger channel contains non-integer values.');
end

k=1;
nts=0;
while nts==0 && k<=MAX_BITS
  % extract kth bit and scale resulting signal to interval 0-5
  trigch_bit=bitand(fiff.tschan,2^(k-1))/2^(k-1)*5;
  % pass the signal to comp_tstamps_2 and see if it finds any timestamps
  [fiff.ts,nts,fiff.sfest,fiff.site_id]=comp_tstamps_2(trigch_bit,fiff.meta.info.sfreq,tcorr,offset);
  k=k+1;
end
if k==MAX_BITS+1 && nts==0
  error('Did not find any timestamps on trigger channel!');
end
fiff.tschan=trigch_bit;
fprintf('loadmeg_findaud: sampling frequency estimated from timestamps: %g Hz, reported in fiff: %g Hz\n',fiff.sfest,fiff.meta.info.sfreq);
if fiff.site_id==-1
  fprintf('loadmeg_findaud: warning: fiff has an unknown meg2meg site id\n');
end

% if no audio detection, we're done
if isempty(audiodir)
  aud_receiver='';
  aud_sender='';
  return
else
  % locate matching audio files
  if fiff.site_id==-1
    error('loadmeg_findaud: fiff has an unknown site id, cannot automatically determing matching audio files.');
  end
  files=find_matching_aud(audiodir,fiff.ts(1),fiff.ts(end),fiff.site_id);
  if length(files) < 2,
    error('loadmeg_findaud: cannot find two matching audio files in specified directory');
  end
  % else, found matches, and possibly multiple ones
  % try to find to best matches (sender-receiver pair)
  sender_best_overlap=0;
  sender_best=0;
  receiver_best_overlap=0;
  receiver_best=0;
  for k=1:length(files),
    if files(k).is_sender,
      if files(k).overlap > sender_best_overlap,
        sender_best_overlap=files(k).overlap;
        sender_best=k;
      end
    else % receiver
      if files(k).overlap > receiver_best_overlap,
        receiver_best_overlap=files(k).overlap;
        receiver_best=k;
      end
    end          
  end
  if ~(receiver_best && sender_best)
    error('loadmeg_findaud: found matching files, but either sender or receiver is missing.\n');
  end
  aud_sender=files(sender_best).name;
  aud_receiver=files(receiver_best).name;
end

% print names without path
[pathstr,sname,sext]=fileparts(aud_sender);
[pathstr,rname,rext]=fileparts(aud_receiver);
fprintf('loadmeg_findaud: best matches:\nSender file (%g seconds overlap): %s\nReceiver file (%g seconds overlap): %s\n',sender_best_overlap/1e3,[sname sext],receiver_best_overlap/1e3,[rname rext]);



