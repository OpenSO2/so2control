readme.md

SO2-Control Software


Programmierprojekt zur erzeugung einer Bild Aquisitionssoftware fuer die Hamammatsu CCD Camera C8484-16.
Geschrieben in der Programmiersprache C. Am Institut für Geophysik der Universitaet Hamburg. In der 
Arbeitsgruppe Vulkanologie.

Autoren:
	Morten Harms


Dateien und ihre Bedeutung:

SO2-Control.c
	Inhalt:
		- main()
	Funktion:
		- Programmeinstieg, start aller wichtigen Funktionen
	
configurations.c
	Inhalt:
		struct sParameterStruct
		struct flagStruct
		readConfig()
		configurationFunktion()
		structInit()
		triggerConfig()
		defaultConfig()
		defaultCameraConfig()
		sendMessage()
		
	Funktion:
		- einrichten der Variablen Struckturen
		- Konfiguration der Kamera
		- Konfiguration des Framegrabbers
	
exposureTimeControl.c
	Inhalt:
		- setExposureTime()
		fixEposureTime()
		setElektronicShutter()
		setFrameBlanking()
		getOneBuffer()
		evalHist()
		rountToInt()
	
	Funktion:
		
		

ImageCreation.c
	Inhalt:
		callbackFunction()
		writeImage()
		startAquisition()
	
	Funktion:



============ZU=ERREICHENDE=ZIELE============

- Zeitmanagement mit Millisekunden Genauigkeit
- feststellen der genauen Bildwiederholrate
	->	Interframedelay etc.
- bug in ExposureTimeControl
	->	bei bestimmten Lichtverhältnissen wird keine Belichtungszeit gefunden
		das Programm springt dann immer zwischen zwei Belichtungszeiten.
- Dunkelstrommessung implementieren
	->	So lange das Filterrad sich nicht Computergesteuert drehen laesst ist das schwierig
		eventuell trotzdem rudimaentaer implementieren
- erstellen eines Logfiles mit allen relevanten events
- besseres Fehlermanagement!!!!
