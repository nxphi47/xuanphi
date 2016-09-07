#include <iostream>
#include <wiringPi.h>
#include "TestLib.h"

using namespace std;

const int ledPin = 1; // BCM 18, can use for pwm, onboard 12
const int recvPin1= 0; // BCM 17, onboard 11, use pullUpDnControl
const int recvPin2 = 2; // BCM 27, onboard 13
const int lastPin = 29; //BCM 21, onboard 40

int main() {
	wiringPiSetup(); // using wiringPi numbering scheme
	// setmode
	pinMode(ledPin, OUTPUT);
	pinMode(recvPin1, INPUT);
	pinMode(recvPin2, INPUT);
	pinMode(lastPin,  OUTPUT);

	bool state = true;

	unsigned long goal = 1000000000 + millis();
	while (goal > millis()){
		cout << "pin 1: " << digitalRead(recvPin1) << " pin 2: " << digitalRead(recvPin2) << endl;
		if (state){
			digitalWrite(ledPin, 1);
			digitalWrite(lastPin, 0);
			state = !state;
		} else{
			state = !state;
			digitalWrite(ledPin, 0);
			digitalWrite(lastPin, 1);
		}
		delay(1000);
	}

	return 0;
}