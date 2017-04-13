set file=%1
set check_cmd="findstr"
%check_cmd% /c:"timestampBefore" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"2014-09-22T23:43:55.984Z" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"dBufferlength 1376256" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"dDarkCurrent " %file%
if errorlevel 1 exit 1
%check_cmd% /c:"dImageCounter 0" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"dInterFrameDelay 10" %file%
if errorlevel 1 exit 1
REM %check_cmd% /c:"dExposureTime 0.000000" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"dFixTime 0" %file%
if errorlevel 1 exit 1
%check_cmd% /c:"version" %file%
if errorlevel 1 exit 1

exit 0
