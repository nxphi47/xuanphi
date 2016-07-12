//
// Created by nxphi on 7/2/16.
//

//#include <sys/types.h>
#include "Arduino.h"
#include "DisplayMatrix.h"
#include "SnakeLogic.h"

uint8_t rx = 0;// using serial
uint8_t tx = 0;

SnakeLogic game(0,0, SERIAL_COM);

void setup() {
	//Serial.begin(9600);
	game.init();
}


void loop() {
	//Serial.println("Begin loop\n");
	game.run();
	delay(10000);
}
