Arduino-Filterwheel
===================

This project aims to provide a complete, simple, cheap (>80€, w/o
the stepper motor assembly) filterwheel/stepper motor controller.


## Hardware components

This projects does not include a stepper motor or breaker switch.
Logic power is supplied to the arduino through the usb connection,
motor power is supplied directly to the board (12V, max. 2.5A).

### Parts

| Part                                                            | approx. cost |
|-----------------------------------------------------------------|--------------|
| [Board][board]                                                  | ~ 30€        |
| [Arduino Nano](nano)                                            | ~ 30€        |
| [DRV8825 Stepper Motor Driver Carrier, High Current](stepper)   | ~ 10€        |
| [Connector Sensor (2.54mm, 2 Pins)](stift)                      | > 1€         |
| [Connector 4-Way-Switch (2.54mm, 5 Pins)](stift)                | > 1€         |
| [Connector Motor (2.54mm, 4 Pins)](stift)                       | > 1€         |
| [Power Connector (2mm, 2 Pins)](power)                          | > 1€         |
| Capacitator 100 uF                                              | > 1€         |
| 5 Resistor 1KOhm                                                | > 1€         |
| 1 Resistor 120 Ohm                                              | > 1€         |
|                                                                 | ===========  |
|                                                                 | > 80€        |


[board]: http://fritzing.org/projects/arduino-stepper-motor-controller
[nano]: https://www.arduino.cc/en/Main/ArduinoBoardNano
[stepper]: https://www.pololu.com/product/2133
[stift]:http://katalog.we-online.de/de/em/619_0xx_111_21
[power]:http://www.conrad.de/ce/de/product/740197/Stiftleiste-gewinkelt-RM-20-mm-Serie-PH-Pole-2-S2B-PH-K-S-LFSN-JST-Inhalt-1-St
