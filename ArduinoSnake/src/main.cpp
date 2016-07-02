//
// Created by nxphi on 7/2/16.
//

#include "Arduino.h"
#include "DisplayMatrix.h"

Display matrix(ARRAY);

void setup() {
	//Serial.begin(9600);
}


void loop() {
	//Serial.println("Begin loop\n");
	int i;
	for (i = 0; i < 26; ++i) {
		matrix.showAlpha(1000, (char) (i + 65));
	}
}
