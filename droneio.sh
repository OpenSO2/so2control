#/bin/bash
set -e
set -u
set -x
sudo apt-get update
sudo apt-get install cmake libopencv-dev libcv-dev libhighgui-dev pngcheck valgrind
git submodule update --init
git status
git log -n 1
mkdir build
cd build
cmake .. -DMOCK_LOG=ON -DMOCK_CAMERA=ON -DMOCK_FILTERWHEEL=ON
make
ctest --output-on-failure
