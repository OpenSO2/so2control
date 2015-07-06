set file=%1
set check_cmd="findstr"
pngcheck -t %file% | %check_cmd% /c:"Creation Time"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"2014-09-22T23:43:55.984Z"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dBufferlength: 1376256.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dHistMinInterval: 350.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dHistPercentage: 5.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dDarkCurrent: "
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dImageCounter: 0.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dInterFrameDelay: 10.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dTriggerPulseWidth: 15.000000"
if errorlevel 1 exit 1
REM pngcheck -t %file% | %check_cmd% /c:"dExposureTime: 0.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dFixTime: 0.000000"
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dfilesize: "
if errorlevel 1 exit 1
pngcheck -t %file% | %check_cmd% /c:"dImagesFile: "
if errorlevel 1 exit 1

exit 0