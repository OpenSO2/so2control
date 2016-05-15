Webcam Subsystem
================

The webcam subsystem drives a single webcam attached to the so2 camera
system. Webcam data is useful to get a visual "feel" for the data and
to do (better) spacial correlation.

This subsystem exposes three methods (see `webcam.h`):

* ``int webcam_init`` - initializes the webcam
* ``int webcam_get`` - gets a single webcam image and fills a supplied buffer
* ``int webcam_uninit`` - does clean up and deinitialize the webcam

Currently, only two "cameras" are supported:

* elpcam, any webcam that is supported by opencv
* mock, which is not actually a camera, but a stub returning
  prerecorded data (useful for testing)

To implement a new webcam driver copy the `mock` folder, implement the
three functions (see above), and change CMakeList.txt to include the
correct folder.

This subsystem builds a `webcam-cli` executable that can be used for
testing.
