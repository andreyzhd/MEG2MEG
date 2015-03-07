% returns (linear) index of array element in incr. sorted array A that is
% closest to c but < c
%
function j=nearest_smaller_ind(A,c)
j=max(find(A-c<0));






