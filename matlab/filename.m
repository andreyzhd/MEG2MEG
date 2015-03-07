%function fn=filename(fullpath);
% Return filename, given full path
function fn=filename(fullpath);
  [pathstr,fnbody,ext,versn]=fileparts(fullpath);
  fn=[fnbody ext];
  
  