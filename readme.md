SO2-Control Software
====================

SO2 Camera control software written in C, licensed under [MIT](LICENSE.md).

| Windows | Linux | QA     |
| ------- | ----- | ------ |
| [![Build status on Windows](https://ci.appveyor.com/api/projects/status/wtsnd28pv7ymsabg?svg=true)](https://ci.appveyor.com/project/jjacobsohn/so2-camera) | [![Build Status on Linux](https://travis-ci.org/OpenSO2/so2control.svg?branch=master)](https://travis-ci.org/johannjacobsohn/so2-camera)  | [![Coverity Scan Build Status](https://scan.coverity.com/projects/6043/badge.svg)](https://scan.coverity.com/projects/johannjacobsohn-so2-camera) |


This is a command line utility to drive the hardware typically employed
in SO2 cameras. This includes dual view UV cameras, filter wheel,
spectrometer, spectrometer shutter and webcam in addition to general
housekeeping such as logging, config parsing etc. It does not provide
as graphical interface or capabilities for further processing. These
task are left to auxiliary software packages such as SO2live and
SO2Eval.

This software is optimized for performance and adaptability. To support
different hardware devices (types of UV cameras, webcams etc)
device-specific code is hidden behind a simple interface for each
device. The specific implemented can be switched at compile time. For
testing this includes a mock implementation for each device.

For a technical documentation see [the `readme.md` in `src`](src/readme.md).


Getting Started:
---------

### On Linux:

From a terminal, run:

````
$ # install dependencies on RHEL, Fedora, Scientific Linux, CentOS etc:
$ sudo yum install cmake opencv-devel gcc gcc-c++
$
$ # or Debian, Ubuntu, etc:
$ sudo apt-get install cmake libopencv-dev libcv-dev libhighgui-dev gcc gcc-c++
$
$ # prepare (mock/simulate all devices)
$ cmake . -DMOCK_CAMERA=ON -DMOCK_FILTERWHEEL=ON -DMOCK_SPECTROMETER_SHUTTER=ON -DMOCK_SPECTROMETER=ON -DMOCK_WEBCAM=ON -DMOCK_SPECTROMETER=ON
$
$ # compile
$ make
$
$ # test run
$ ./so2-camera --help
````
or look at `.travis.yml` and do whats done there.

### On Windows:

Download and install [CMake][cmake] and [OpenCV][opencv]. Then run

```
$ cmake . -DMOCK_CAMERA=ON -DMOCK_FILTERWHEEL=ON -DMOCK_SPECTROMETER_SHUTTER=ON -DMOCK_SPECTROMETER=ON -DMOCK_WEBCAM=ON -DMOCK_SPECTROMETER=ON
```

from the command prompt in the project folder. This generates a Visual
Studio Workspace, which can be opened in Visual Studio. Compile and run
from there.

Or take a look at `appveyor.yml` and do whats done there.

**Beware:** Windows support is incomplete and not well-tested.

Usage:
----

After compilation, it can be run with

`$ ./so2-camera` (linux)

or

`$ ./so-camera.exe` (Windows).


Command line and configuration options, compilation flags
---------------------------------------------------------

Run time behavior can be configured through a config file or as
command line options (command line options take precedence).

Configuration values are taken from three sources (in overriding order):

- internal presets (see `configurations.c`)
- from a config file
- from the command line arguments, eg. --noofimages N

These include:

```
   --noprocessing                Skip processing as much as possible and only save raw images
   --png-only                    Skip saving of raw files
   --debug                       Print debug output
   --noofimages n                Only save n UV image sets and exit
   --configfile /path/file.conf  Load config file from path. If not set config files are searched for at the usual places
   --imagepath /path/outfolder   Save images and logs in path
   --port portno                 Set port for liveview. Default: 7009
   --disableWebcam               Disable processing and saving of webcam images
   --disableSpectroscopy         Disable processing and saving of spectra
```

To see all command line options run

```
$ ./so2-camera --help
```

The config file named `so2-camera.conf` is searched for at the usual
places (see `configurations.c` for details) or at the path configured
through the `--configfile` command line option. A sample configuration
file can be found `configurations/so2-camera.conf`.

The config file is parsed as simple key=value syntax, similar to INI
style. Lines starting with '#' are ignored, as are unknown keys.


Coding style
------------

This project loosely follows the [linux kernel coding style](https://www.kernel.org/doc/Documentation/CodingStyle) (ignoring the line length limit)
which is checked using [indent](http://www.gnu.org/software/indent/) and can be triggered by issuing

```
$ cmake .
$ make checkstyle
```

Authors:
--------

- [Johann Jacobsohn][jj] (Universität Hamburg)
- Morten Harms (Universität Hamburg)


License
-------

This project is licensed under the MIT License - see the LICENSE.md file for details

[jj]: johann.jacobsohn@uni-hamburg.de
[opencv]: http://opencv.org/
[zlib]: http://www.zlib.net/
[phx]: http://www.activesilicon.com/products_sw.htm#phxsdk
[cmake]: http://www.cmake.org/
