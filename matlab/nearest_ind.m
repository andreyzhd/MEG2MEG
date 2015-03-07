%function j=nearest_ind(A,c)
% returns first (linear) index of array element in A that is closest to c
%
function j=nearest_ind(A,c)
Adiff=abs(A-c);
j=min(find(Adiff==min(Adiff)));



