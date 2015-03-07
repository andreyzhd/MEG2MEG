%function chns=channelns(chnames,chlist);
%
% Return channel numbers, given a cell array of desired channel
% names and a channel namelist.
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

function chns=channelns(chnames,chlist);
if ~iscell(chnames)
  error('channelns: specify desired channels as a cell array');
end
chns=zeros(length(chnames),1);
k=1;
for i=1:length(chnames),   % desired channels
  for j=1:length(chlist),  % all channels
    if strcmp(chnames(i),chlist(j)) == 1
      chns(k)=j;
      k=k+1;
    end
  end
end
if min(chns)==0  % zeros correspond to nonexistent channels
    error('channelns: requested channel does not exist');
end
