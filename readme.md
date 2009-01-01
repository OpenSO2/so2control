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

- johann: bilder verarbeiten
- johann: Bug: neue Datei anlegen
- johann: Bug: Kamera in Bildnamen oder Header
- morten: Bilder gleichzeitig aufnehmen oder Zeitdifferenz messen
- morten: Zeitstempel direkt nach aufnahme setzen
- Belichtungszeit? -> Matthias 
- Zeitsynchro/Batterie -> Matthias
Mi: Kamera bauen
Do: testen
Fr: packen
--
- morten: improve time management to millisecond accuracy
- morten: measure frame rate -> Interframedelay etc.
- johann: implement automated filter wheel control
- johann: implement dark-frame subtraction
- morten: improve error management
- morten: ExposureTimeControl: improve loop detection
