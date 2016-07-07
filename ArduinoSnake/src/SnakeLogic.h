//
// Created by phi on 04/07/16.
//

#ifndef ARDUINOSNAKE_SNAKELOGIC_H
#define ARDUINOSNAKE_SNAKELOGIC_H

#include "Arduino.h"
#include "Snake.h"
#include "Vector.h"
#include "DisplayMatrix.h"
#include "CommsHub.h"

bool DEBUG = true;

/* ---------- WORKFLOW------------
 * process of the snake logic in run loop, when time interval reach the speed, proceed the move
 * 1/ check incomming message, if yes, process the message and update para
 * 2/ if (time() - lastTime >= speed)
 *          move, update status
 * 3/ check status
 * 4/ print
 * 5/ process outputUpdate and send back to controller
 *
 * ---------- COMMUNICATION protocal --------
 * wired: serial and software serial
 * 			see class commshub
 * wireless: wifi via ESP module
 */


extern void display(Snake theSnake);
extern void processOutput(int status, int score);

class SnakeLogic {
public:
	SnakeLogic(uint8_t rx = 0, uint8_t tx = 0){
		speed = 100;
		direct = RIGHT;
		status = 0;
		directQueue = Vector<Direction >();

		pinTX = tx;
		pinRX = rx;
		if (pinRX == 0 || pinTX == 0) {
			// Serial communication
			commsHub = new CommsHub();
		}
		else {
			commsHub = new CommsHub(pinRX, pinTX, SOFTWARE_SERIAL);
		}
	}
	void init(){
		// init is to initialise (begin) any other class
		snake = Snake(8, 2, false);
		score = snake.getLength();
		display.init(VECTOR);
		commsHub->initialise(14400); // use the max
	}

	// ------ OPERATION -------------------
	// standby, waiting for starting signal from the controller
	void standbyMode(){
		status = 0;
		speed = inputGetSpeed();	// if nothing received, return 0;
		// print standBy message on matrix
		uint8_t welcomeMess[MATRIXSIZE][MATRIXSIZE] = {
				{0,1,0,0,0,0,0,0},
				{0,1,0,0,0,1,0,0},
				{0,1,1,1,0,0,0,0},
				{0,1,0,1,0,1,0,0},
				{0,1,0,1,0,1,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0},
				{0,0,0,0,0,0,0,0}
		};
		display.setMode(ARRAY);
		display.setArrayAll(welcomeMess);
		while (speed <= 0){
			display.showArray(10);
			speed = inputGetSpeed();
		}
		display.setMode(VECTOR);
		status = 1;
		// ending standby mode
	}

	// execute mode, keep looping send and receive until failed or something happen
	void executeMode(){
		status = 1;
		unsigned long lastTime = millis();
		// lastTime = millis();
		while (status == 1){
			// check incomming message, updating
			inputGetDirect();

			// moving operation
			if (millis() - lastTime >= speed){
				snake.setDirection(direct);
				bool succeed = snake.move();
				if (!succeed){
					status = 2;
				}
				score = snake.getLength();
				lastTime = millis();
			}
			// displaying the snake and food, interval of 20ms
			displaySnake2Matrix();

			// process the outputUpdate
			"FIXME";
			outputUpdate();
		}

		// goto result
		status = 2;
	}
	// endMode, just showing the result
	void endMode(){
		status = 2;
		while (true){
			int end = inputGetOption();
			if(end != 0){
				break;
			}
			else{
				displaySnake2Matrix();
			}
		}

		" at the end of this part, clear everything of the snake and all variable";

	}
	// main method to run
	void run(){
		standbyMode();
		executeMode();
		endMode();
	}


	// display the matrix of snake
	void displaySnake2Matrix() {   // implement the display class with input is the snake
		// speed of showing is 20
		unsigned int interval = 20;
		Coor food = snake.getFood();
		Vector<Coor> snakeAndFood = snake.getList();
		snakeAndFood.append(snake.getFood());
		// showing
		display.showVector(interval, snakeAndFood);

		// cleaning
		delete snakeAndFood;
		delete food;
	}

	// ---- Communication set of functions
	// must consider the affect of stagnation and direction queue (done by controller)
	// inputGetDirect take 15ms
	int inputGetDirect(){
		uint8_t byteToGet = 16;
		String result;
		if (commsHub->available()){
			result = commsHub->request(byteToGet);
			result.toUpperCase();
			// format: DIRECTION:UP___/DOWN_/RIGHT/LEFT_
			if (result.substring(0, 9) == "DIRECTION"){
				if (DEBUG){
					Serial.println(result.substring(0, 10));
				}

				String directStr = result.substring(11, 16);
				if (directStr == "UP___"){
					if (direct == LEFT || direct == RIGHT){
						direct = UP;
					}
				}
				else if (directStr == "DOWN_"){
					if (direct == LEFT || direct == RIGHT){
						direct = DOWN;
					}
				}
				else if (directStr == "LEFT_"){
					if (direct == UP || direct == DOWN){
						direct = LEFT;
					}
				}
				else if (directStr == "RIGHT"){// RIGHT
					if (direct == UP || direct == DOWN){
						direct = RIGHT;
					}
				}
				else{
					return -1;
				}
				return 0;
			}
			else{
				if (DEBUG){
					Serial.print("Error directStr: ");
					Serial.println(result);
				}
				return -1;
			}
		}
		else {  // nothing received
			if (DEBUG){
				Serial.println("getDirect: nothing");
			}
			delay(byteToGet);
			return 0;
		}

		// return 0 if get the result
		// return -1 if error or something
	}

	// get speed take 11
	// format "SPEED:0123"
	short inputGetSpeed(){
		// if get the speed, return the speed, if error, return -1, if nothing, return 0
		uint8_t byteToGet = 12;
		String result;
		short speedVal;
		if (commsHub->available()){
			result = commsHub->request(byteToGet);
			result.toUpperCase();

			// check the result
			if (result.substring(0, 5) == "SPEED"){
				String speedStr = result.substring(6);
				if (DEBUG){
					Serial.print("getSpeed: ");
					Serial.println(speedStr);
				}
				try {
					speedVal = (short) speedStr.toInt();
				} catch (int e){
					if (DEBUG){
						Serial.print("Error getSpeed: ");
						Serial.println(result);
						return -1;
					}
				}
				return speedVal; // return the speedVal
			}
			else{
				if (DEBUG){
					Serial.print("Error getSpeed: ");
					Serial.println(result);
				}
				return -1;
			}
		}
		else{
			// nothing receive
			if (DEBUG){
				Serial.println("GetSpeed: Nothing");
			}
			delay(byteToGet);
			return 0;
		}
	}

	int inputGetOption(); // endMode, 0 if nothing, 1 if restart, 2 if restart whole program(to be developt)
	// set of function to use for sending

	// implement the commsHub to send to the controller
	bool outputUpdate(){
		// RESPONSE:SCORE:05
		String out = "RESPONSE:SCORE:" + String(score);
		try {
			commsHub->send(out);
		}
		catch (int e){
			if (DEBUG){
				Serial.print("output failed: ");
				Serial.println(out);
			}
			return false;
		}

		return true;
	}

private:
	int status; // 1 for running, 0 for stanby, 2, for failed
	int score;
	short speed;
	uint8_t pinRX;
	uint8_t pinTX;
	Direction direct;
	Vector<Direction > directQueue;
	Snake snake;
	Display8x8Matrix display;
	CommsHub *commsHub;
	// the communication hubs
	// display module
};

#endif //ARDUINOSNAKE_SNAKELOGIC_H
