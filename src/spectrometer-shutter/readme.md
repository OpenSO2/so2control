Spectrometer Shutter
====================

This subsystem drives a shutter mechanism for the spectrometer.

Three mechanisms are provided

- pololu-maestro - a simple servo with an usb servo controller, see below
- manual - prints a message to stdout and waits for keyboard input, useful for manual operation
- mock - a mock implementation, useful for testing

These mechanisms are *compile time* options and can be set via the
MOCK_SPECTROMETER_SHUTTER cmake variable. Eg.

```
	$ cmake .. -DMOCK_SPECTROMETER_SHUTTER=ON      # mock shutter
	$ cmake .. -DMOCK_SPECTROMETER_SHUTTER=MANUAL  # manual operation
	$ cmake .. -DMOCK_SPECTROMETER_SHUTTER=OFF     # use pololu-maestro
```

The pololu maestro is the default value.

pololu-maestro
--------------

This can be used in conjunction with any small servo, eg. the
[modelcraft mc1811](https://www.conrad.de/de/modelcraft-mini-servo-mc1811-analog-servo-getriebe-material-kunststoff-stecksystem-jr-275460.html).

