#include <iostream>
#include <wiringPi.h>
#include "TestLib.h"

using namespace std;

const int ledPin = 1; // BCM 18, can use for pwm, onboard 12
const int recvPin = 0; // BCM 17, onboard 11, use pullUpDnControl
const int lastPin = 29; //BCM 21, onboard 40

int main() {
	wiringPiSetup(); // using wiringPi numbering scheme
	// setmode
	pinMode(ledPin, OUTPUT);
	pinMode(recvPin, OUTPUT);
	pinMode(lastPin,  OUTPUT);
	//int ledState = 1;
	digitalWrite(ledPin, 1);
	digitalWrite(recvPin, 1);
	digitalWrite(lastPin, 1);

	unsigned long goal = 1000000000 + millis();
	while (goal > millis()){
		/*
		for (int i = 0; i < 50; i++){
			printHello();
			digitalWrite(ledPin, i%2);
			cout << "send " << i%2 << " recv: " << digitalRead(recvPin) << endl;
			delay(300);
		}
		*/
		printHello();
		delay(1000);
	}

	return 0;
}