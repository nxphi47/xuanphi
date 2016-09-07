//
// Created by nxphi on 7/2/16.
//

#include "Arduino.h"

/* This is for board
*/
#include "DisplayMatrix.h"
#include "SnakeLogic.h"
uint8_t rx = 0;// using serial
uint8_t tx = 0;

//SnakeLogic game(SERIAL_COM,0,0);
Display8x8Matrix  matrix;
void setup() {
	//game.init();
	matrix.init(ARRAY);
	Serial.begin(9600);
}


void loop() {
	//game.run();
	//delay(10000);

	Coor x = {0,3};
	Coor y = {1,2};
	Vector<Coor> zzz(x);
	Serial.print(zzz.size());
	Serial.print(zzz.get(1).X);
	Serial.print(zzz.get(1).Y);
	Serial.print("done!\n");
	delay(1000);
	//matrix.showVector(10000, zzz);

}


// --------------------------------------------------------------------------

/* this is for SnakeController
#include "SnakeController.h"

SnakeController controller;
uint8_t rx = 15; //A2
uint8_t tx = 16; //A3

// using pin 15, 16 as RX and TX in Software Serial

void setup() {
	controller.initialize(rx, tx, SOFTWARESERIAL_COM);
}

void loop(){
	controller.run();
	delay(10000);
}
*/
