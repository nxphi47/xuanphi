#ifndef MATRIX_DISPLAY_H
#define  MATRIX_DISPLAY_H

#include "Arduino.h"


/* USAGE:
every time, set all row to LOW, and all col to HIGH
for each dot, set it row to HIGH and all col to LOW to light it up

on Board:
maxtrix pin 1 - 8 is on the side of the middle dot on the matrix
*/
const int row[8] = {
  2, 7, 19, 5, 13, 18, 12, 16
};

// 2-dimensional array of column pin numbers:
const int col[8] = {
  6, 11, 10, 3, 17, 4, 8, 9
};

const int matrixSize = 8;
int array[matrixSize][matrixSize] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

class Display{
public:
    Display(){
        for (size_t i = 0; i < matrixSize; i++) {
            pinMode(row[i], OUTPUT);
            pinMode(col[i], OUTPUT);
        }
    };

    void clearArray(){
        for (size_t i = 0; i < matrixSize; i++) {
            for (size_t j = 0; j < matrixSize; j++) {
                array[i][j] = 0;
            }
        }
    };

    void setArrayDot(int x, int y){
        array[x][y] = 1;
    };

    void showArray(int interval){
        unsigned long goal = millis() + interval;
        while (goal > millis()) {
            for (size_t i = 0; i < matrixSize; i++) {
                for (size_t j = 0; j < matrixSize; j++) {
                    if (array[i][j] == 1) {
                        clearDisplay();
                        digitalWrite(row[i], HIGH);
                        digitalWrite(col[j], LOW);
                    }
                }
            }
        }
        clearDisplay();
    };

    void clearDisplay(){
        for (size_t i = 0; i < matrixSize; i++) {
            digitalWrite(row[i], LOW);
            digitalWrite(col[i], HIGH);
        }
    };

private:
    int * listDot;
};

#endif
