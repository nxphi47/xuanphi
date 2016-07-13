//
// Created by nxphi on 7/2/16.
//

#include "Arduino.h"

/* This is for board
#include "DisplayMatrix.h"
#include "SnakeLogic.h"
uint8_t rx = 0;// using serial
uint8_t tx = 0;

SnakeLogic game(0,0, SERIAL_COM);
void setup() {
	game.init();
}


void loop() {
	game.run();
	delay(10000);
}

*/

// --------------------------------------------------------------------------

/* this is for SnakeController
*/
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