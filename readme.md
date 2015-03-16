SO2-Control Software
====================

SO2 Camera control software written in C, based on the Hamammatsu CCD Camera
C8484-16. Licenced under MIT.

Contributors:
- Morten Harms (Universität Hamburg)
- Johann Jacobsohn (Universität Hamburg)

Status:
[![Build Status](https://drone.io/bitbucket.org/jjacobsohn/so2-camera/status.png)](https://drone.io/bitbucket.org/jjacobsohn/so2-camera/latest)

Files
------
- **SO2-Control.c** - main program
- **configurations.c|h** - set up structs, configure camera and frame grabber
- **exposureTimeControl.c|h** - sets and monitors exposure time
- **imageCreation.c|h** - aquires and saves images
- **darkCurrent.c|h** - to be implemented
- **log.c|h** - log messages to file
- **messages.c|h** - static messages

Interface:
----------
### configurations.c|h

- **struct sParameterStruct** -
- **struct flagStruct** -
- **readConfig()** -
- **configurationFunktion()** -
- **structInit()** -
- **triggerConfig()** -
- **defaultConfig()** -
- **defaultCameraConfig()** -
- **sendMessage()** -

### exposureTimeControl.c|h

- **setExposureTime()** -
- **fixEposureTime()** -
- **setElektronicShutter()** -
- **setFrameBlanking()** -
- **getOneBuffer()** -
- **evalHist()** -
- **rountToInt()** -

### imageCreation.c|h

- **callbackFunction()** -
- **writeImage()** -
- **startAquisition()** -

### imageFunctions.c|h

- **rotateImage()** -
- **findDisplacement()** -
- **displaceImage()** -
- **calcCorrelation()** -

### darkCurrent.c|h

- **dunkelstromMessung** - To be implemented

### log.c|h

- **initLog()** -
- **logMessage(char *message)** -
- **logError(char *message)** -
- **logExit()** -

### messages.c|h

- **printOpening()** - prints a friendly startup statement to stdout

TODO
====

- image flipped
- distance correction
- vignette correction
- Zeitstempel direkt nach aufnahme setzen
- Belichtungszeit? -> Matthias
- improve time management to millisecond accuracy
- measure frame rate -> Interframedelay etc.
- implement automated filter wheel control
- implement dark-frame subtraction
- improve error management
- ExposureTimeControl: improve loop detection
- online evaluation
- cleanup make target
- "--start-new-measurement" - neuen Ordner anlegen, neues Logfile
- Parameter wie "FixTime" lieber als Konstanten?
- don't just twiddle your thumb while waiting for acquisition callback, put that hardware to work!
- wir brauchen einen Camera Identifier in der config-Struktur.
- document linux & windows configuration (Puppet? Chef?)
- document hardware
- Tests
- Linting über splint


Approximate program flow:
-------------------------

```
---------------------------------------------------------------------------
|                         PHX_CameraConfigLoad                            |
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
|                       Release the Phoenix board                         |
---------------------------------------------------------------------------
```





Korrekturfaktur = Strahlungsfluss durch Filter x empfindlichkeit bei Filterwellenlänge * Belichtungszeitdifferenz
