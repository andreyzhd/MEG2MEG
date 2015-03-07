%function zcind=zerocross(vec);
% Return indices of rising zerocrossings in a vector, i.e. 
% vec(i)>0 & vec(i-1)<=0.
%
function zcind=zerocross(vec);
veca=vec(2:end);
vecc=vec(1:end-1);
zcind=find(veca>0 & vecc<=0)+1;


