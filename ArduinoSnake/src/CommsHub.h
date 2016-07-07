//
// Created by phi on 05/07/16.
//

#ifndef ARDUINOSNAKE_COMMSHUB_H
#define ARDUINOSNAKE_COMMSHUB_H

#include <zconf.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "Vector.h"

// --- COMMSHUB-----
/* MODE: software serial, or computer serial, define in the contructor
 * Method:
 * iniatialise: create the the hub (in setup).begin and start listening
 * String Request(timeout): get the input String and return. timeout to be small, if nothing, return "\0"
 * Byte send(String): send the thing
 * Byte send(Byte): send the byte
 * getRXpin, getTXpin, available, end
 */
enum CommMode {SERIAL_COM, SOFTWARE_SERIAL, I2C_COM, SPI_COM, WIRELESS_COM};

class CommsHub{
public:
	CommsHub(uint8_t rx, uint8_t tx, CommMode protocol){
		pinRX = rx;
		pinTX = tx;
		mode = protocol;
		if (mode == SOFTWARE_SERIAL){
			portSoftwarePtr = new SoftwareSerial(pinRX, pinTX);
		}
		else if (mode == SERIAL_COM){
			// using Serial
		}
		else{
			// other, do something
		}
	}
	CommsHub(){
		// default pin, should not be use
		this->CommsHub(0, 0, SERIAL_COM);
	}
	void setPin(uint8_t rx, uint8_t tx){
		pinRX = rx;
		pinTX = tx;
	}
	uint8_t *getPin(){
		uint8_t pins[2] = {pinRX, pinTX};
		return pins;
	}
	int available(){
		switch (mode){
			case SERIAL_COM:
				return Serial.available();
			case SOFTWARE_SERIAL:
				return portSoftwarePtr->available();
			case WIRELESS_COM:
			case I2C_COM:
			case SPI_COM:
				break;
		}
		return -1;
	}

	// starting the programe
	void initialise(uint32_t baud){
		baudRate = baud;
		switch (mode){
			case SERIAL_COM:
				Serial.begin(baudRate);
				break;
			case SOFTWARE_SERIAL:
				portSoftwarePtr->begin(baudRate);
				break;
			default:
				break;
		}
	}
	void end(){
		switch (mode){
			case SERIAL_COM:
				Serial.end();
				break;
			case SOFTWARE_SERIAL:
				portSoftwarePtr->end();
				break;
			default:
				break;
		}
	}

	// requesting function -- byteSize, every byte delay 1ms
	String request(uint16_t timeout = 64){
		String result = "";
		unsigned long goal = millis() + timeout + 5;
		while (goal > millis()){
			switch (mode){
				case SERIAL_COM:
					if (Serial.available()){
						result += Serial.read();
						delay(1);
					}
					break;
				case SOFTWARE_SERIAL:
					if (portSoftwarePtr->available()){
						result += portSoftwarePtr->read();
						delay(1);
					}
					break;
				case I2C_COM:
				case WIRELESS_COM:
				case SPI_COM:
					break;
			}
		}

		// need to trim down all other space, \n or " "
		result.trim();
		return result;
	}

	// Sending function set ----
	size_t send(String output){
		char out[70];
		size_t outSize = 0;
		output.toCharArray(out, sizeof(out));
		switch (mode){
			case SERIAL_COM:
				outSize = Serial.write(out);
				break;
			case SOFTWARE_SERIAL:
				outSize = portSoftwarePtr->write(out);
				break;
			case WIRELESS_COM:
			case I2C_COM:
			case SPI_COM:
				break;
		}
		return outSize;
	}

private:
	SoftwareSerial *portSoftwarePtr;
	uint32_t baudRate;
	CommMode mode;
	uint8_t pinTX;
	uint8_t pinRX;
};

#endif //ARDUINOSNAKE_COMMSHUB_H
