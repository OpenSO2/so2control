SO2 Camera: Subsystems
======================

The SO2 camera system is organized into subsystem. This is done so
that each subsystem can be tested, mocked and over-/rewritten
independently.

Each component/subsystem abstracts the underlying
hardware/SDK/problem space in a way that encapsulates the actual
implementation, which improves test- and readability.

- `camera subsystem` - drives the dual view camera system
- `log` - provides convenient logging methods
- `spectrometer` - not yet implemented
- `filterwheel` - not yet implemented
- `processing` - not yet implemented
- `webcam` - not yet implemented
- `gpstime` - not yet implemented
- `io` - not yet implemented

Each subsystem has its own readme with further documentation.



Approximate program flow:
-------------------------

```
---------------------------------------------------------------------------
|                             camera_config                               |
---------------------------------------------------------------------------
                                                                       ↧
---------------------------------------------------------------------------
|                             configurations                              |
---------------------------------------------------------------------------
                                                                       ↧
---------------------------------------------------------------------------
|                            setExposureTime                              |
---------------------------------------------------------------------------
                                                                       ↧
---------------------------------------------------------------------------
|                            startAquisition                              |
|                                                                         |
| main loop until keypress                                           <-.  |
|    . reset exposure time every exposureTimeCheckIntervall * seconds   \ |
|    . aquire both image                                                | |
|        . start two captures, set callback functions                   | |
|        . wait in 10ms increments until callback is received           | |
|        . writeImage                                                   | |
|    . process data                                                     | |
|           \                                                           / |
|            ----------------------------------------------------------`  |
|                                                                         |
---------------------------------------------------------------------------
                                                                      ↧
---------------------------------------------------------------------------
|                          Cease all captures                             |
---------------------------------------------------------------------------
                                                                     ↧
---------------------------------------------------------------------------
|                             camera_stop                                 |
---------------------------------------------------------------------------
```
