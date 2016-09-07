//
// Created by nxphi on 7/2/16.
//

#ifndef ARDUINOSNAKE_DISPLAYMATRIX_H
#define ARDUINOSNAKE_DISPLAYMATRIX_H

// muse be link with vector
#include "Arduino.h"
#include "Vector.h"

#define MATRIXSIZE 8

/* USAGE:
every time, set all row to LOW, and all col to HIGH
for each dot, set it row to HIGH and all col to LOW to light it up

on Board:
maxtrix pin 1 - 8 is on the side of the middle dot on the matrix
*/

const uint8_t row[8] = {
		2, 7, 19, 5, 13, 18, 12, 16
};

// 2-dimensional array of column pin numbers:
const uint8_t col[8] = {
		6, 11, 10, 3, 17, 4, 8, 9
};

unsigned long goal;

// const int matrixSize = 8;
// main array to be use
uint8_t array[MATRIXSIZE][MATRIXSIZE] = {
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0}
};

// SET OF PRE-DEFINED NUMBER AND CHARACTER, SEE AT THE END OF THE FILE
extern uint8_t NUMBER_SET_MATRIX[10][MATRIXSIZE][MATRIXSIZE];

// for alphabet use 'char' to transfer to number 0-25
extern uint8_t ALPHABET_SET_MATRIX[26][MATRIXSIZE][MATRIXSIZE];


enum DisplayMode {
	ARRAY, VECTOR, DOT, POINTER
};

//extern enum Direction;	// defined in Vector.h

// Main class of matrix display
class Display8x8Matrix {
public:
	Display8x8Matrix(DisplayMode mode = ARRAY) {
		// serial
		// use init to initialise, at void setup();
	};
	void init(DisplayMode mode = ARRAY){
		for (size_t i = 0; i < MATRIXSIZE; i++) {
			pinMode(row[i], OUTPUT);
			pinMode(col[i], OUTPUT);
		}
		setMode(mode);
		vector = Vector<Coor>();
	}

	Display8x8Matrix *clearDisplay() {
		for (size_t i = 0; i < MATRIXSIZE; i++) {
			digitalWrite(row[i], LOW);
			digitalWrite(col[i], HIGH);
		}
		return this;
	};

	Display8x8Matrix *setMode(DisplayMode mode) {
		this->mode = mode;
		return this;
	}

	DisplayMode getMode() {
		return mode;
	}

	// array manipulation
	Display8x8Matrix *clearArray() {
		for (size_t i = 0; i < MATRIXSIZE; i++) {
			for (size_t j = 0; j < MATRIXSIZE; j++) {
				array[i][j] = 0;
			}
		}
		return this;
	};

	Display8x8Matrix *setArrayDot(unsigned int x, unsigned int y) {
		array[x][y] = 1;
		return this;
	};

	Display8x8Matrix *clearArrayDot(unsigned int x, unsigned int y) {
		array[x][y] = 0;
		return this;
	}

	Display8x8Matrix *setArrayAll(uint8_t arr[][MATRIXSIZE]) {
		for (uint8_t i = 0; i < MATRIXSIZE; ++i) {
			for (uint8_t j = 0; j < MATRIXSIZE; ++j) {
				array[i][j] = arr[i][j];
			}
		}
		return this;
	}

	Display8x8Matrix *showArray(unsigned int interval) {
		goal = millis() + (unsigned long) interval;
		clearDisplay();
		while (goal > millis()) {
			for (size_t i = 0; i < MATRIXSIZE; i++) {
				for (size_t j = 0; j < MATRIXSIZE; j++) {
					if (array[i][j] == 1) {
						// turn it on
						digitalWrite(row[i], HIGH);
						digitalWrite(col[j], LOW);
						// turn it off
						digitalWrite(row[i], LOW);
						digitalWrite(col[j], HIGH);
					}
				}
			}
		}
		clearDisplay();
		return this;
	};

	// vector manipulation
	Display8x8Matrix *addVector(Vector<Coor> vec) {
		vector = vec;
		return this;
	}

	Display8x8Matrix *showVector(unsigned int interval, Vector<Coor> &vec) {
		// for the timming, need a wrap around feature
		goal = millis() + (unsigned long) interval;
		clearDisplay();
		int i;
		while (goal > millis()) {
			for (i = 0; i < vec.size(); i++) {
				// turn it on
				digitalWrite(vec.get(i).X, HIGH);
				digitalWrite(vec.get(i).Y, LOW);
				delay(100);
				// turn it off
				digitalWrite(vec.get(i).X, LOW);
				digitalWrite(vec.get(i).Y, HIGH);
			}
		}
		clearDisplay();
		return this;
	}


	// main show function, decide whether array(to use the global array, need to change to local)
	// or use local vector
	Display8x8Matrix *show(unsigned int interval) {
		switch (getMode()) {
			case ARRAY:
				showArray(interval);
				break;
			case VECTOR:
				showVector(interval, vector);
				break;
			case POINTER:
				"FIXME: PLease";
				break;
			case DOT:
				"FIXME: PLease";
				break;
			default:
				break;
		}
		return this;
	}

	// number and character manipulation
	Display8x8Matrix *showNumber(unsigned int interval, int num) {
		clearArray();
		setArrayAll(NUMBER_SET_MATRIX[num]);
		showArray(interval);
		clearArray();
		return this;
	}

	Display8x8Matrix *showAlpha(unsigned int interval, char ch) {
		if (!isalnum(ch)) {
			// invalid character
			Serial.println("Invalid input chat for show alpha");
			return this;
		}
		int charNum;
		if (isUpperCase(ch)) {
			charNum = ch - 65; // A = 65
		}
		else {
			charNum = ch - 97;    // a = 97
		}
		Serial.println(charNum);
		clearArray();
		setArrayAll(ALPHABET_SET_MATRIX[charNum]);
		showArray(interval);
		clearArray();
		return this;
	}

	// ----------ANIMATION, must input the pointer to the first set --------------
	Display8x8Matrix *showShift(unsigned int speed, unsigned int interval, uint8_t *ptr, int size,
							   Direction direct, bool wrapAround, bool horizon) {
		// we change to showUpArray to be the first 8x8 matrix of the set and show, then update it
		// interval continuity only possible when wrapAround is TRUE
		clearArray();
		uint8_t showUpArray[MATRIXSIZE][MATRIXSIZE] = {{0}};
		goal = millis() + (unsigned long) interval;
		int pos = 0;
		while (goal > interval) {
			// main operation, showing the the specify array at speed time and continue
			// the transition to the next
			if (horizon){
				// can only be horizon transition
				if (direct == UP || direct == DOWN){
					Serial.println("Error: transit up down in horizon");
					return this;
				}
				pos = shift(showUpArray, ptr, size, pos, direct, horizon);
			}
			else{
				// can only be vertical
				if (direct == LEFT || direct == RIGHT){
					Serial.println("Error: transift left right in vertical");
					return this;
				}
				pos = shift(showUpArray, ptr, size, pos, direct, horizon);
			}
			setArrayAll(showUpArray);
			showArray(speed);
		}
		return this;
	}

	// Copy the target array to the pointer of the source array, by the position of Hoz or Ver
	void copyArrayPointerHorizontal(uint8_t arr[][MATRIXSIZE], uint8_t *ptr, int size, int posOfPtr) {
		// size is the horizontal length of the pointer array
		// the PTR must point to the first array element to be PRECISE
		// 01234567,8 9 10 11 12 13 14 15
		"Need to handlde wrap around";
		if (size - posOfPtr >= MATRIXSIZE) {
			for (int i = 0; i < MATRIXSIZE; ++i) {
				for (int j = 0; j < MATRIXSIZE; ++j) {
					arr[i][j] = *(ptr + j + posOfPtr + size * i);
				}
			}
		}
		else {
			for (int i = 0; i < MATRIXSIZE; ++i) {
				for (int j = 0; j < size - posOfPtr; ++j) {
					arr[i][j] = *(ptr + j + posOfPtr + size * i);
				}
				for (int k = 0; k < MATRIXSIZE - (size - posOfPtr); ++k) {
					arr[i][k] = *(ptr + k + posOfPtr + size * i);
				}
			}
		}
	}

	void copyArrayPointerVertical(uint8_t arr[][MATRIXSIZE], uint8_t *ptr, int size, int postOfPtr) {
		// size is the vertical length of the pointer array, must be > MATRIXSIZE
		for (int i = 0; i < MATRIXSIZE; ++i) {
			for (int j = 0; j < MATRIXSIZE; ++j) {
				arr[i][j] = *(ptr + j + size * (i + postOfPtr));
			}
		}
		"Need to handle wrap around";
		if (size - postOfPtr >= MATRIXSIZE){
			for (int i = 0; i < MATRIXSIZE; ++i){
				for (int j = 0; j < MATRIXSIZE; ++j) {
					arr[i][j] = *(ptr + j + MATRIXSIZE * (i + postOfPtr));
				}
			}
		}
		else{
			for (int i = 0; i < size - postOfPtr; ++i) {
				for (int j = 0; j < MATRIXSIZE; ++j) {
					arr[i][j] = *(ptr + j + MATRIXSIZE * (i + postOfPtr));
				}
			}
			for (int k = size - postOfPtr; k < MATRIXSIZE; ++k) {
				for (int i = 0; i < MATRIXSIZE; ++i) {
					arr[k][i] = *(ptr + i + MATRIXSIZE * (k + postOfPtr - size));
				}
			}
		}
	}

	// shift to return the new vertical or horizontal position of the target pointer, copy to the array
	int shift(uint8_t arr[][MATRIXSIZE], uint8_t *ptr, int size, int currentPos, Direction direct, bool horizon) {
		"FIXME: ";
		switch (direct){
			case UP:
				currentPos--;
				if (currentPos < 0){
					if (horizon){
						currentPos = MATRIXSIZE - 1;
					}
					else{
						currentPos = size - 1;
					}
				}
				copyArrayPointerVertical(arr, ptr, size, currentPos);
				break;
			case DOWN:
				currentPos++;
				if (horizon){
					if (currentPos == MATRIXSIZE){
						currentPos = 0;
					}
				}
				else{
					if (currentPos == size){
						currentPos = 0;
					}
				}
				copyArrayPointerVertical(arr, ptr, size, currentPos);
				break;
			case LEFT:
				currentPos--;
				if (currentPos < 0){
					if (horizon){
						currentPos = size - 1;
					}
					else{
						currentPos = MATRIXSIZE - 1;
					}
				}
				copyArrayPointerHorizontal(arr, ptr, size, currentPos);
				break;
			case RIGHT:
				currentPos++;
				if (horizon){
					if (currentPos == size){
						currentPos = 0;
					}
				}
				else{
					if (currentPos == MATRIXSIZE){
						currentPos = 0;
					}
				}
				copyArrayPointerHorizontal(arr, ptr, size, currentPos);
				break;
			default:
				// fault found hear
				break;
		}
		return currentPos;
	}


private:
	DisplayMode mode;
	int *listDot;
	Vector<Coor> vector;
};

// number set
uint8_t NUMBER_SET_MATRIX[10][MATRIXSIZE][MATRIXSIZE] = {
		{// 0
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0}
		},
		{// 1
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 1, 1, 0, 0, 0},
				{0, 1, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// 2
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 0, 1, 0, 0, 0},
				{0, 0, 0, 1, 0, 0, 0, 0},
				{0, 0, 1, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// 3
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 1, 1, 1, 0, 0},
				{0, 0, 0, 1, 1, 1, 0, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// 4
				{0, 0, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 0, 1, 1, 0, 0},
				{0, 0, 0, 1, 0, 1, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 0, 0, 1, 0, 0}
		},
		{// 5
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 0, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// 6
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// 7
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 0, 1, 1, 0},
				{0, 0, 0, 0, 1, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 1, 0, 0, 0, 0},
				{0, 1, 1, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0}
		},
		{// 8
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// 9
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 0, 1, 0, 0, 0},
				{0, 0, 0, 1, 0, 0, 0, 0}
		}
};

uint8_t ALPHABET_SET_MATRIX[26][MATRIXSIZE][MATRIXSIZE] = {
		{// A
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// B
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 0, 0}
		},
		{// C
				{0, 0, 0, 1, 1, 1, 0, 0},
				{0, 0, 1, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 0, 1, 0, 0, 0, 1, 0},
				{0, 0, 0, 1, 1, 1, 0, 0}
		},
		{// D
				{0, 1, 1, 1, 1, 0, 0, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 1, 1, 1, 0, 0, 0}
		},
		{// E
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// F
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0}
		},
		{// G
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 1, 1, 0, 0},
				{0, 0, 1, 1, 0, 1, 0, 0}
		},
		{// H
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// I
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// J
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 1, 0, 1, 1, 0, 0, 0},
				{0, 1, 1, 1, 1, 0, 0, 0}
		},
		{// K
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 1, 0, 0, 0},
				{0, 1, 1, 1, 0, 0, 0, 0},
				{0, 1, 1, 1, 0, 0, 0, 0},
				{0, 1, 0, 0, 1, 0, 0, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// L
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		},
		{// M
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 0, 0, 1, 1, 0},
				{0, 1, 0, 1, 1, 0, 1, 0},
				{0, 1, 0, 1, 1, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// N
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 0, 0, 0, 1, 0},
				{0, 1, 0, 1, 0, 0, 1, 0},
				{0, 1, 0, 1, 0, 0, 1, 0},
				{0, 1, 0, 0, 1, 0, 1, 0},
				{0, 1, 0, 0, 1, 0, 1, 0},
				{0, 1, 0, 0, 0, 1, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// O
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// P
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0}
		},
		{// Q
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 1, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 1, 0}
		},
		{// R
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// S
				{0, 0, 1, 1, 1, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 0, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// T
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0}
		},
		{// U
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 1, 1, 1, 0, 0}
		},
		{// V
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0}
		},
		{// W
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 1, 1, 0, 1, 0},
				{0, 1, 0, 1, 1, 0, 1, 0},
				{0, 1, 0, 1, 1, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0}
		},
		{// X
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0}
		},
		{// Y
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 1, 0, 0, 0, 0, 1, 0},
				{0, 0, 1, 0, 0, 1, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0},
				{0, 0, 0, 1, 1, 0, 0, 0}
		},
		{// Z
				{0, 1, 1, 1, 1, 1, 1, 0},
				{0, 0, 0, 0, 0, 0, 1, 0},
				{0, 0, 0, 0, 0, 1, 0, 0},
				{0, 0, 0, 0, 1, 0, 0, 0},
				{0, 0, 0, 1, 0, 0, 0, 0},
				{0, 0, 1, 0, 0, 0, 0, 0},
				{0, 1, 0, 0, 0, 0, 0, 0},
				{0, 1, 1, 1, 1, 1, 1, 0}
		}
};

#endif //ARDUINOSNAKE_DISPLAYMATRIX_H
