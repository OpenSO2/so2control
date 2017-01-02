cinst -y OpenCV
git submodule update --init
set PATH=%PATH%;c:\OpenCV249\opencv\build\x86\vc11\bin
cmake -DCMAKE_BUILD_TYPE=Release -DMOCK_CAMERA=ON -DOpenCV_DIR=c:\OpenCV249\opencv\build .
cmake --build . --config Release
ctest --output-on-failure
