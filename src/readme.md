SO2 Camera: Subsystems
======================

The SO2 camera system is organized into subsystem. This is done so
that each subsystem can be tested, mocked and over-/rewritten
independently.

Each component/subsystem abstracts the underlying
hardware/SDK/problem space in a way that encapsulates the actual
implementation, which improves test- and readability.

- `camera subsystem` - drives the dual view camera system
- `log` - provides convinient logging methods
- `spectrometer` - not yet implemented
- `filterwheel` - not yet implemented
- `processing` - not yet implemented
- `webcam` - not yet implemented
- `gpstime` - not yet implemented
- `io` - not yet implemented

Each subsystem has its own readme with further documentation.
