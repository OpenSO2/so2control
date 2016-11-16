SO2 Camera: Camera subsystem
============================

This implements the actual camera/framegrabber interface and
mostly is a framegrabber SDK wrapper to allow for the underlying SDK
to be changed. This can also be used to mock the hardware functions
for unit/integration testing.

The following functions are provided by this file:

- `camera_init` - inits and configures the camera/framegrabber
- `camera_get` - aquires one image/frame from the camera/framegrabber
- `camera_setExposure` -
- `camera_uninit` - stops (uninits) the camera/framegrabber and does neccesarry clean up
- `camera_abort` - aborts a triggered capture

Currently, two implementations are included:

- `phx` - supports Active Silicon framegrabbers using the sdk
- `mock` - mocks the camera subsystem for development and testing

To add support for other framegrabbers/SDKs, copy mock/camera.c and
implement the functions therein.
