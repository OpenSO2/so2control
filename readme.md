SO2-Control Software
====================

SO2 Camera control software written in C, licenced under MIT.

** Collaborators: **

- [Johann Jacobsohn][jj] (Universität Hamburg)
- Morten Harms (Universität Hamburg)

** Dependencies: **

- [OpenCV][opencv]
- [zLib][zlib]
- [Active Silicon SDK](phx)

All Dependencies are optional.

For a technical documentation see [the `readme.md` in `src`](src/readme.md).


Building:
---------

### On Linux:

From a terminal, run:

````
$ # install dependencies
$ sudo yum/apt-get install cmake zlib-devel opencv-devel gcc
$
$ # prepare
$ cmake .
$
$ # compile
$ make
````

### On Windows:

Download and install [CMake][cmake], [zLib][zlib] and [OpenCV][opencv]. Then run

```
$ cmake .
```

from the command prompt in the project folder. This generates a Visual Studio Workspace,
which can be opened in Visual Studio. Compile and run from there.


Usage:
----

This is a purely command line software. After compilation, it can be run with

`$ ./so2-camera` (linux)

or

`$ ./so-camera.exe` (Windows).

There are some (run time) configuration options in
configurations/SO2Config.conf.



Supported Framegrabbers/Cameras:
----------------------------------

Currently, only Active Silicon Framegrabbers are supported using the Active
Silicon Phonix SDK. This dependency however is isolated to
`src/camera/phx/camera.c`. Using another Framegrabber/Camera/SDK
thus entails rewriting that file (i.e. `src/camera/mock/camera.c`).
For development, the camera subsystem can be mocked using

```
$ cmake . -Dmock=camera
```

which circumvents this dependency.

[jj]: johann.jacobsohn@uni-hamburg.de
[opencv]: http://opencv.org/
[zlib]: http://www.zlib.net/
[phx]: http://www.activesilicon.com/products_sw.htm#phxsdk
[cmake]: http://www.cmake.org/
