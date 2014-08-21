SO2-Control Software
====================

SO2 Camera control software written in C, based on the Hamammatsu CCD Camera
C8484-16. Licenced under MIT.

Contributors:
- Morten Harms (Universität Hamburg)
- Johann Jacobsohn (Universität Hamburg)


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

- implement second camera
- improve time management to millisecond accuracy
- measure frame rate -> Interframedelay etc.
- implement dark-frame subtraction
- implement automated filter wheel control
- improve error management
- ExposureTimeControl: improve loop detection
