#/bin/bash
set -e
set -u
set -x
sudo apt-get update
sudo apt-get install cmake zlib1g-dev libopencv-dev libcv-dev libhighgui-dev pngcheck
cmake . -DMOCK_LOG=ON -Dmock=camera
make
env CTEST_OUTPUT_ON_FAILURE=1 make test
