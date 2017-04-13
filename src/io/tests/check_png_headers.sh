#!/bin/bash
set -e
set -u
set -x

file=$1
check_cmd="grep"
pngcheck -t $file | $check_cmd "Creation Time"
pngcheck -t $file | $check_cmd "2014-09-22T23:43:55.984Z"
pngcheck -t $file | $check_cmd "dBufferlength: 1376256.000000"
pngcheck -t $file | $check_cmd "dDarkCurrent: " # 4294967296.000000
pngcheck -t $file | $check_cmd "dImageCounter: 0.000000"
pngcheck -t $file | $check_cmd "dInterFrameDelay: 10.000000"
pngcheck -t $file | $check_cmd "dExposureTime: 0.000000"
pngcheck -t $file | $check_cmd "dFixTime: 0.000000"
pngcheck -t $file | $check_cmd "version"
