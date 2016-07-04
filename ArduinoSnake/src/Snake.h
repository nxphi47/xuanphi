//
// Created by phi on 04/07/16.
//

#ifndef ARDUINOSNAKE_SNAKE_H
#define ARDUINOSNAKE_SNAKE_H


#include "Arduino.h"
#include "Vector.h"
#include "DisplayMatrix.h"

extern enum Direction;	// defined in Vector.h
extern struct Coor; 	// defined in Vector.h

class Snake {
public:
	Snake(uint8_t boardSize = 8, uint8_t initialLength = 2, bool wall = false) {
		this->boardSize = boardSize;
		this->initialLength = initialLength;
		wallOrNot = wall;
		list = Vector<Coor>();
		direct = RIGHT;
		food = {(uint8_t) (boardSize - 1), (uint8_t) (boardSize - 1)};

		// intialise the list
		for (uint8_t i = 0; i < initialLength; ++i) {
			Coor newDot = { i, 0};
			list.insert(0, newDot);
		}
		updateLength();
	}

	// Operational function
	Snake * Destroy(){
		while (list.size() > 0){
			list.pop();
		}
		// intialise the list
		for (uint8_t i = 0; i < initialLength; ++i) {
			Coor newDot = { i, 0};
			list.insert(0, newDot);
		}
		updateLength();
		direct = RIGHT;
		foodGenerate();
		return this;
	}

	// ----------------variable function-------------------
	Snake *updateLength() {
		length = (uint8_t) list.size();
		return this;
	}

	int getLength() {
		updateLength();
		return length;
	}

	Vector<Coor> getList() {
		return list;
	}

	Direction getDirection(){
		return direct;
	}

	Snake *setDirection(Direction orient){
		direct = orient;
		return this;
	}

	bool getWallOrNot(){
		return wallOrNot;
	}

	Snake *setWallOrNot(bool wOn){
		wallOrNot = wOn;
		return this;
	}


	// ------ snake moving and playing functions-----------------
	bool checkNewPosition(uint8_t x, uint8_t y) {
		updateLength();

		// check through all the snake, exept the last and the begining
		for (uint8_t i = 1; i < length - 1; ++i) {
			if (x == list.get(i).X && y == list.get(i).Y) {
				//printf("\ntouch snake at: %d %d, index %d\n", x, y, i);
				return false;
			}
		}

		if (wallOrNot) {
			//check if it is wall to touch
		}
		return true;
	}

	// set move of the snake
	bool moveDirection(Direction orient) {
		uint8_t x = list.get(0).X;
		uint8_t y = list.get(0).Y;
		switch (orient) {
			case UP:
				y--;
				if (y < 0) {
					y += boardSize;
				}
				break;
			case RIGHT:
				x++;
				if (x == boardSize) {
					x = 0;
				}
				break;
			case DOWN:
				y++;
				if (y == boardSize) {
					y = 0;
				}
				break;
			case LEFT:
				x--;
				if (x < 0) {
					x += boardSize;
				}
				break;
			default:
				break;
				//fprintf(stderr, "Snake: movedirection: unable to move with orient");
		}
		// start move with x and y
		return moveReal(x, y);
	}
	bool moveReal(uint8_t x, uint8_t y) {
		if (checkNewPosition(x, y)) {
			// bring the tail to the head
			// if it is the position of food
			if (x == food.X && y == food.Y) {
				// eat the food
				Coor newNode = { x, y};
				list.insert(0, newNode);
				updateLength();

				// generate food, temporary hasing
				food.X = (food.X * 3) % boardSize;
				food.Y = (food.Y * 3) % boardSize;
				return true;
			}
			else {
				// move normally
				Coor tail = list.pop();
				tail.X =  x;
				tail.Y =  y;
				list.insert(0, tail);
				updateLength();

				return true;
			}
		}
		else {
			return false;
		}
	}
	bool move(){
		return moveDirection(direct);
	}


	// ----------------food generation -----------------
	Coor &getFood(){
		return food;
	}
	Coor &setFood(uint8_t x, uint8_t y){
		food.X = x;
		food.Y = y;
		return food;
	}
	Coor &foodGenerate(){	// random function
		uint8_t x;
		uint8_t y;
		do{
			x = (uint8_t) random(MATRIXSIZE);
			y = (uint8_t) random(MATRIXSIZE);
		} while (!checkNewPosition(x, y));
		return setFood(x, y);
	}
	///////

private:
	Vector<Coor> list;
	Coor food;
	uint8_t length;
	uint8_t initialLength;
	Direction direct;
	bool wallOrNot; // to be
	uint8_t boardSize;
};


#endif //ARDUINOSNAKE_SNAKE_H
