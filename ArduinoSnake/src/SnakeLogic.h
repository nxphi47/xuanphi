//
// Created by phi on 04/07/16.
//

#ifndef ARDUINOSNAKE_SNAKELOGIC_H
#define ARDUINOSNAKE_SNAKELOGIC_H

#include "Arduino.h"
#include "Snake.h"
#include "Vector.h"
#include "DisplayMatrix.h"
/*
 * process of the snake logic in run loop, when time interval reach the speed, proceed the move
 * 1/ check incomming message, if yes, process the message and update para
 * 2/ if (time() - lastTime >= speed)
 *          move, update status
 * 3/ check status
 * 4/ print
 * 5/ process output and send back to controller
 */

extern void display(Snake theSnake);
extern void processOutput(int status, int score);

class SnakeLogic {
public:
	SnakeLogic(){
		speed = 100;
		direct = RIGHT;
		status = 0;
		directQueue = Vector<Direction >();
		// initialise the commshub
		// display module
	}
	void init(){
		// init is to initialise (begin) any other class
		snake = Snake(8, 2, false);
		score = snake.getLength();
		display.init(VECTOR);
	}

	// standby, waiting for starting signal from the controller
	void standbyMode(){
		status = 0;
		speed = getSpeedFromCommsHub();	// if nothing received, return 0;
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
			display.showArray(1000);
			speed = getSpeedFromCommsHub();
		}
		display.setMode(VECTOR);
		status = 1;
		// ending standby mode
	}

	// execute mode, keep looping send and receive until failed or something happen
	void executeMode(){
		status = 1;
		int lastTime = speed;
		// lastTime = millis();
		while (status == 1){
			// check incomming message, updating
			"FIXME";
			getDirectFromCommsHub();

			// moving operation
			if (millis() - lastTime >= speed){
				snake.setDirection(direct);
				bool status = snake.move();
				if (!status){
					status = 2;
				}
				score = snake.getLength();
			}
			// displaying the snake and food, interval of 20ms
			displaySnake2Matrix();

			// process the output
			"FIXME";
			processOutput();
		}

		// goto result
		status = 2;
	}
	// endMode, just showing the result
	void endMode(){
		status = 2;
		while (true){
			int end = getEndOptionFromCommsHub();
			if(end != 0){
				break;
			}
			else{
				displaySnake2Matrix();
			}
		}

		// at the end of this part, clear everything of the snake and all variable

	}

	// set of function to use
	void processOutput();   // implement the commsHub to send to the controller

	// display the matrix of snake
	void displaySnake2Matrix() {   // implement the display class with input is the snake
		// speed of showing is 20
		unsigned int interval = 20;
		Coor food = snake.getFood();
		Vector<Coor> snakeAndFood = snake.getList()
		snakeAndFood.append(snake.getFood());
		// showing
		display.showVector(interval, snakeAndFood);

		// cleaning
		delete snakeAndFood;
		delete food;
	}

	int getDirectFromCommsHub(); // must consider the affect of stagnation and direction queue
	int getSpeedFromCommsHub();
	int getEndOptionFromCommsHub(); // endMode, 0 if nothing, 1 if restart, 2 if restart whole program(to be developt)

private:
	int status; // 1 for running, 0 for stanby, 2, for failed
	int score;
	int speed;
	Direction direct;
	Vector<Direction > directQueue;
	Snake snake;
	Display8x8Matrix display;
	// the communication hubs
	// display module
};

#endif //ARDUINOSNAKE_SNAKELOGIC_H
