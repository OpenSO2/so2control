#!/bin/bash
set -e
set -u
set -x

file=$1
check_cmd="grep"
$check_cmd "timestampBefore" $file
$check_cmd "timestampAfter" $file
$check_cmd "2014-09-22T23:43:55.984Z" $file
$check_cmd "dBufferlength 1376256" $file
$check_cmd "dImageCounter 0" $file
$check_cmd "dInterFrameDelay 10" $file
$check_cmd "dExposureTime 0.000000" $file
$check_cmd "dFixTime 0" $file
$check_cmd "version" $file
