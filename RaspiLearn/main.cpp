#include <iostream>
#include <wiringPi.h>
#include "TestLib.h"

using namespace std;

const int ledPin = 1; // BCM 18, can use for pwm, onboard 12
const int buttonPin = 0; // BCM 17, onboard 11, use pullUpDnControl

int main() {
	wiringPiSetup(); // using wiringPi numbering scheme
	// setmode
	pinMode(ledPin, OUTPUT);
	pullUpDnControl(buttonPin, PUD_UP);

	int ledState = 1;
	printHello();

	while (true){
		if (ledState == 1){
			digitalWrite(ledPin, ledState);
			cout << "Led: on, button: " << digitalRead(buttonPin) << endl;
			ledState = 0;
			delay(500);
		}
		else{
			// led off
			digitalWrite(ledPin, ledState);
			cout << "Led: off, button: " << digitalRead(buttonPin) << endl;
			ledState = 1;
			delay(500);
		}
	}

	return 0;
}