Webcam Subsystem
================

The webcam subsystem drives a single webcam attached to the so2 camera
system. Webcam data is useful to get a visual "feel" for the data and
to do (better) spacial correlation.

This subsystem exposes three methods (see `webcam.h`):

* ``int webcam_init`` - initializes the webcam (which is far more intricate then one would think)
* ``int webcam_get`` - gets a single webcam image and fills a supplied buffer
* ``int webcam_uninit`` - does clean up and deinitialize the webcam

Currently, only two "cameras" are supported:

* ucam 1 (and probably 2), a small UART camera
* mock, which is not actually a camera, but a stub returning
prerecorded data (useful for testing)

To implement a new webcam driver simply copy the `mock` folder,
implement the three functions (see above), and change CMakeList.txt
to include the correct folder.
