#/bin/bash
set -e
set -u
set -x
sudo apt-get update
sudo apt-get install cmake libopencv-dev libcv-dev libhighgui-dev pngcheck
cmake . -DMOCK_LOG=ON -DMOCK_CAMERA=ON
make
env CTEST_OUTPUT_ON_FAILURE=1 make test
