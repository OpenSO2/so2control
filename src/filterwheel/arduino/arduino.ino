const int zeroPosition = 1;

// set pin numbers:
const int buttonPin = 8;	// the number of the pushbutton pin
const int ledPin = 13;		// the number of the LED pin
const int setting1Pin = 12;
const int setting2Pin = 11;
const int setting3Pin = 10;
const int setting4Pin = 9;

// set orientation
int orientation_forward = LOW;
int orientation_backward = HIGH;

const int transl_m = 45;	// 134*10/30;
const int RES = 200;		// RESOLUTION per full revolve
const int QUARTREVOLV = RES / 4;

const int STEP = A1;		// STEPPING PIN
const int DIR = A0;		// DIRECTION PIN
const int SLP = A2;		//
const int RST = 2;		//
const int ENABLE = 6;		//
const int MS1 = 4;		//
const int MS2 = 5;		//
const int MS3 = 3;

int buttonState = 0;		// variable for reading the pushbutton status
int setting1PinState = 0;	// variable for reading the pushbutton status
int setting2PinState = 0;	// variable for reading the pushbutton status
int setting3PinState = 0;	// variable for reading the pushbutton status
int setting4PinState = 0;	// variable for reading the pushbutton status
int setting = 1;
int calibrated = 0;
int statusDeg = 0;
int oldbuttonState = -1;
int oldsetting = 0;

int enableDriver()
{
	digitalWrite(ENABLE, LOW);
}

int disableDriver()
{
	digitalWrite(ENABLE, HIGH);
}

int sleepON()
{
	digitalWrite(SLP, LOW);
}

int sleepOFF()
{
	digitalWrite(SLP, HIGH);
}

int resetDriver()
{
	digitalWrite(RST, HIGH);
}

void setup()
{
	Serial.begin(9600);

	// initialize the LED pin as an output:
	pinMode(ledPin, OUTPUT);

	// initialize the pushbutton pin as an input:
	pinMode(buttonPin, INPUT);
	pinMode(setting1Pin, INPUT);
	pinMode(setting2Pin, INPUT);
	pinMode(setting3Pin, INPUT);
	pinMode(setting4Pin, INPUT);

	pinMode(DIR, OUTPUT);
	pinMode(STEP, OUTPUT);
	pinMode(MS1, OUTPUT);
	pinMode(MS2, OUTPUT);
	pinMode(MS3, OUTPUT);
	pinMode(SLP, OUTPUT);
	pinMode(ENABLE, OUTPUT);
	pinMode(RST, OUTPUT);

	enableDriver();
	resetDriver();
}

int rotate(int steps)
{
	int sleep_us = 800;

	char str[50];
	sprintf(str, "rotate by %i steps \n", steps);
	Serial.write(str);

	if (steps > 0) {
		digitalWrite(DIR, orientation_forward);
		Serial.write("> forward\n");
	} else {
		steps = 0 - steps;
		digitalWrite(DIR, orientation_backward);
		Serial.write("< backward\n");
	}
	delayMicroseconds(100 * 1000);

	int real_steps = steps * transl_m / 10;

	sprintf(str, "now rotate by %i, was %i \n", real_steps, steps);
	Serial.write(str);

	for (int i = 0; i < real_steps; i++) {
		// Trigger the motor to take one step.
		digitalWrite(STEP, HIGH);
		delayMicroseconds(sleep_us);
		digitalWrite(STEP, LOW);
		delayMicroseconds(sleep_us);
	}
}

/**
 *
 *
 */
int get_button_setting()
{
	int setting = 0;
	if (digitalRead(setting1Pin) == HIGH) {
		setting = 1;
	}
	if (digitalRead(setting2Pin) == HIGH) {
		setting = 2;
	}
	if (digitalRead(setting3Pin) == HIGH) {
		setting = 3;
	}
	if (digitalRead(setting4Pin) == HIGH) {
		setting = 4;
	}

	return setting;
}

/**
 *
 *
 */
int calibrate()
{
	sleepOFF();

	while (1) {
		Serial.write("looking for 0-Position\n");
		if (digitalRead(buttonPin) == HIGH)
			break;
		rotate(2);
	}

	sleepON();
	return 1;
}

void done(){
	Serial.write("done\n");
	digitalWrite(ledPin, LOW);
}

void loop()
{
	int diffToGo;
	int newDeg = 0.;
	int i;

	if (calibrated == 0) {
		calibrated = calibrate();
		Serial.write("calibrated\n");
	}

	/*
	   FIXME: document who overrides who
	 */
	int buttonState = get_button_setting();
	if (buttonState > 0 && buttonState < 5 && buttonState != oldbuttonState) {	// eg. button has been switched
		setting = buttonState;
		oldbuttonState = buttonState;
		Serial.write("button state changed\n");
	}
	// check to see if there is any new command on the serial port waiting
	if (Serial.available()) {
		int ser = Serial.read();

		digitalWrite(ledPin, HIGH);
		if (setting != ser && ser > 0 && ser < 5)
			setting = ser;
		else
			done();
	}

	if (setting != oldsetting) {
		oldsetting = setting;

		// blink new setting on led
		for (i = 0; i < setting; i++) {
			digitalWrite(ledPin, HIGH);
			delay(100);
			digitalWrite(ledPin, LOW);
			delay(100);
		}

		digitalWrite(ledPin, HIGH);

		// calculate distance that we have to rotate
		newDeg = QUARTREVOLV * (setting - 1) + zeroPosition;
		diffToGo = statusDeg - newDeg;
		statusDeg = newDeg;

		// do the actual rotation.
		sleepOFF();
		rotate(diffToGo);
		sleepON();

		done();
	}
}
