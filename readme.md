SO2-Control Software
====================

[![Join the chat at https://gitter.im/johannjacobsohn/so2-camera](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/johannjacobsohn/so2-camera?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

SO2 Camera control software written in C, licenced under MIT.

| Windows | Linux | QA     |
| ------- | ----- | ------ |
| [![Build status on Windows](https://ci.appveyor.com/api/projects/status/wtsnd28pv7ymsabg?svg=true)](https://ci.appveyor.com/project/jjacobsohn/so2-camera) | [![Build Status on Linux](https://drone.io/bitbucket.org/jjacobsohn/so2-camera/status.png)](https://drone.io/bitbucket.org/jjacobsohn/so2-camera/latest)  | [![Coverity Scan Build Status](https://scan.coverity.com/projects/6043/badge.svg)](https://scan.coverity.com/projects/johannjacobsohn-so2-camera) |

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
$ # install dependencies on RHEL, Fedora, Scientific Linux, CentOS etc:
$ sudo yum install cmake opencv-devel gcc gcc-c++
$ # or Debian, Ubuntu, etc:
$ sudo apt-get install cmake libopencv-dev libcv-dev libhighgui-dev gcc gcc-c++
$
$ # prepare
$ cmake . -DMOCK_CAMERA=ON
$
$ # compile
$ make
````
or run `droneio.sh`.

### On Windows:

Download and install [CMake][cmake] and [OpenCV][opencv]. Then run

```
$ cmake . -DMOCK_CAMERA=ON
```

from the command prompt in the project folder. This generates a Visual Studio Workspace,
which can be opened in Visual Studio. Compile and run from there.

Or take a look at `appveyor.yml` and do whats done there.


Usage:
----

This is a purely command line software. After compilation, it can be run with

`$ ./so2-camera` (linux)

or

`$ ./so-camera.exe` (Windows).

There are some (run time) configuration options in
`configurations/SO2Config.conf`.



Supported Framegrabbers/Cameras:
----------------------------------

Currently, only Active Silicon Framegrabbers are supported using the Active
Silicon Phonix SDK. This dependency however is isolated to
`src/camera/phx/camera.c`. Using another Framegrabber/Camera/SDK
thus entails rewriting that file (i.e. `src/camera/mock/camera.c`).
For development, the camera subsystem can be mocked using

```
$ cmake . -DMOCK_CAMERA=ON
```

which circumvents this dependency.


Command line and configuration options, compilation flags
---------------------------------------------------------

There are several command line options that can change the runtime
behaviour of the program:

* `--noprocessing` - only raw images are saved to disk and no further processing is done
* `--png-only` - only png images are saved to disk
* `--noofimages %i` - only %i images are taken after which the programm exits itself.

More configuration options can be set in configation/SO2Config.conf.

Compilation flags can be used to controll the compilation of the program

* `DEBUG` - increases the logging output. Use as `cmake -DDEBUG=ON` or `-DDEBUG=OFF`
* `MOCK_CAMERA` - Replaces the camera code with a mock that always returns a dummy picture. Useful for testing. Use as `cmake -DMOCK_CAMERA=ON` or `-DMOCK_CAMERA=OFF`.
* `MOCK_LOG` - Prevents the creation and the writing to log files, which is useful for unit testing. Use as `cmake -DMOCK_LOG=ON` or `-DMOCK_LOG=OFF`.


Coding style
------------

This project follows the [linux kernel coding style](https://www.kernel.org/doc/Documentation/CodingStyle)
which is enforced using [indent](http://www.gnu.org/software/indent/) and can be triggered by issuing

```
$ make checkstyle
```

[jj]: johann.jacobsohn@uni-hamburg.de
[opencv]: http://opencv.org/
[zlib]: http://www.zlib.net/
[phx]: http://www.activesilicon.com/products_sw.htm#phxsdk
[cmake]: http://www.cmake.org/
