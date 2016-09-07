#include "Arduino.h"
#include "display.h"
/*
const int row[8] = {
  2, 7, 19, 5, 13, 18, 12, 16
};

// 2-dimensional array of column pin numbers:
const int col[8] = {
  6, 11, 10, 3, 17, 4, 8, 9
};
*/
Display matrix;

void setup() {
    Serial.begin(9600);
    matrix = Display();
    matrix.setArrayDot(1,1);
    matrix.setArrayDot(4,6);
    matrix.setArrayDot(1,2);

}


void loop() {
    matrix.showArray(100);
    //digitalWrite(pin[2], LOW);
    //digitalWrite(pin[3], LOW);

}
