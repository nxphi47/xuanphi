//
// Created by phi on 05/07/16.
//

#ifndef ARDUINOSNAKE_COMMSHUB_H
#define ARDUINOSNAKE_COMMSHUB_H

//#include <zconf.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "Vector.h"
#include "Wire.h"

extern bool DEBUG;

// --- COMMSHUB-----
/* MODE: software serial, or computer serial, define in the contructor
 * Method:
 * iniatialise: create the the hub (in setup).begin and start listening
 * String Request(timeout): get the input String and return. timeout to be small, if nothing, return "\0"
 * Byte send(String): send the thing
 * Byte send(Byte): send the byte
 * getRXpin, getTXpin, available, end
 *
 * -------------------
 * I2C pin: A4: SDA, A5: SDL, I2C will act as Master
 * Serial Pin: 0 1
 * Wireless: using ESP module, to be develop
 */
enum CommMode {SERIAL_COM, SOFTWARESERIAL_COM, I2C_COM, SPI_COM, WIRELESS_COM};

class CommsHub{
public:
	CommsHub(){
		// default pin, should not be use
		// dothing
	}

	// need to set the commMode for comms first, begining is Serial,
	// for I2C, third is address = 10 (default)
	// for Serial, third is baud rate, 144000 (default)
	CommsHub *setMode(CommMode protocol, uint8_t rx_sda, uint8_t tx_sdl, uint8_t thirdPara = 0){
		mode = protocol;
		if (mode == SERIAL_COM){
			// do nothing
			pinRX = 0;
			pinTX = 0;
			baudRate = (thirdPara == 0)?144000:thirdPara;
		}
		else if (mode == SOFTWARESERIAL_COM){
			pinRX = rx_sda;
			pinTX = tx_sdl;
			baudRate = (thirdPara == 0)?144000:thirdPara;
			portSoftwarePtr = new SoftwareSerial(pinRX, pinTX);
		}
		else if (mode == I2C_COM){
			pinSDA = rx_sda;
			pinSDL = tx_sdl;
			i2cAddr = (thirdPara == 0)?10:thirdPara;
		}
		return this;
	}


	void setPinRxTx(uint8_t rx, uint8_t tx){
		pinRX = rx;
		pinTX = tx;
	}
	uint8_t *getPinRxTx(){
		uint8_t pins[2] = {pinRX, pinTX};
		return pins;
	}
	void setPinSDA_SDL(uint8_t sda, uint8_t sdl){
		pinSDA = sda;
		pinSDL = sdl;
	}
	uint8_t *getPinSDA_SDL(){
		uint8_t  pins[2] = {pinSDA, pinSDL};
		return pins;
	}

	int available(){
		switch (mode){
			case SERIAL_COM:
				return Serial.available();
			case SOFTWARESERIAL_COM:
				return portSoftwarePtr->available();
			case I2C_COM:
				return Wire.available();
			case WIRELESS_COM:
			case SPI_COM:
				break;
		}
		return -1;
	}

	// starting the program
	// para = baud rate for serial, address for i2c
	void initialise(){
		switch (mode){
			case SERIAL_COM:
				Serial.begin(baudRate);
				break;
			case SOFTWARESERIAL_COM:
				portSoftwarePtr->begin(baudRate);
				break;
			case I2C_COM:
				Wire.begin(i2cAddr);
			case WIRELESS_COM:
			case SPI_COM:
				"FIXME:----";
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
			case SOFTWARESERIAL_COM:
				portSoftwarePtr->end();
				break;
			case I2C_COM:
				Wire.end();
				break;
			default:
				break;
		}
	}

	// requesting function -- byteSize, every byte delay 100 us
	String request(uint16_t byteSize = 64){
		String result = "";
		unsigned long goal = millis() + byteSize + 5;
		while (goal > millis()){
			switch (mode){
				case SERIAL_COM:
					if (Serial.available()){
						result += Serial.read();
						delayMicroseconds(100);
					}
					break;
				case SOFTWARESERIAL_COM:
					if (portSoftwarePtr->available()){
						result += portSoftwarePtr->read();
						delayMicroseconds(100);
					}
					break;
				case I2C_COM:
					requestI2C_master(result, byteSize);
					break;
				case WIRELESS_COM:
				case SPI_COM:
					break;
			}
		}


		// need to trim down all other space, \n or " "
		result.trim();
		if (DEBUG){
			Serial.print("request result: ");
			Serial.println(result);
		}
		return result;
	}
	// request when i2c master
	void requestI2C_master(String &string, uint16_t byteSize) {
		Wire.requestFrom(i2cAddr, byteSize + 5);
		while (Wire.available() > 0){
			string += Wire.read();
			delayMicroseconds(100);
		}
		if (DEBUG){
			Serial.print("requestI2C_master: ");
			Serial.println(string);
		}
	}
	void requestI2C_slave(String &string, uint16_t byteSize);

	// Sending function set ----
	// main send function
	size_t send(String output){
		char out[70];
		size_t outSize = 0;
		output.toCharArray(out, sizeof(out));
		switch (mode){
			case SERIAL_COM:
				outSize = Serial.write(out);
				break;
			case SOFTWARESERIAL_COM:
				outSize = portSoftwarePtr->write(out);
				break;
			case I2C_COM:
				// only when it is master
				outSize = sendI2C_master(out);
				break;
			case WIRELESS_COM:
			case SPI_COM:
				break;
		}
		if (DEBUG){
			Serial.print("Send: ");
			Serial.println(outSize);
		}
		return outSize;
	}
	// send when it is i2c master
	size_t sendI2C_master(char *output){
		Wire.beginTransmission(i2cAddr);
		size_t size = Wire.write(output);
		Wire.endTransmission();
		if (DEBUG){
			Serial.print("Send I2C: ");
			Serial.println(size);
		}
		return size;
	}

private:
	CommMode mode;

	// I2C
	uint8_t i2cAddr;
	uint8_t pinSDA;
	uint8_t pinSDL;

	// SoftwareSerial
	SoftwareSerial *portSoftwarePtr;
	uint32_t baudRate;
	uint8_t pinTX;
	uint8_t pinRX;
};

#endif //ARDUINOSNAKE_COMMSHUB_H
