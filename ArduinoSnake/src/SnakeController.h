//
// Created by phi on 13/07/16.
//

#ifndef ARDUINOSNAKE_SNAKECONTROLLER_H
#define ARDUINOSNAKE_SNAKECONTROLLER_H

#include "Arduino.h"
#include "LiquidCrystal.h"
#include "Vector.h"
#include "CommsHub.h"

#define I2C_ADDRESS 8 // I2C address
#define BAUD_RATE 9600
#define SPEED_DEFAULT 100
#define SCORE_DEFAULT 2

extern bool DEBUG;

enum ButtonShield {
	BUTTON_LEFT, BUTTON_RIGHT, BUTTON_UP, BUTTON_DOWN, BUTTON_SELECT, BUTTON_NOTHING
};
enum GameMode {
	GAME_STANDBY, GAME_INGAME, GAME_END
};

class SnakeController {
public:
	SnakeController() {
		// LCD shield seting
		lcd = new LiquidCrystal(8, 9, 4, 5, 6, 7); // RS, enable, DB4, DB5, DB6, DB7
		buttonPin = 0; // A0
		commsHub = new CommsHub();
	}

	void initialize(uint8_t rx_SDA, uint8_t tx_SDL, CommMode protocol) {
		// for coms
		pinRx_SDA = rx_SDA;
		pinTx_SDL = tx_SDL;
		commMode = protocol;
		commsHub->setMode(protocol, pinRx_SDA, pinTx_SDL, (commMode == SOFTWARESERIAL_COM) ? BAUD_RATE : I2C_ADDRESS);

		// LCD
		lcd->begin(16, 2);
		lcd->setCursor(0, 0);

		// for game
		status = GAME_STANDBY;
		buttonCurrent = getButtonVal();
		speed = SPEED_DEFAULT;
		score = SCORE_DEFAULT;

		String standByMessage = "---Snake Game---\nSpeed: < " + String(speed) + " >";
		lcdPrint(standByMessage);
	}

	// -------LCD Control, might need to optimize when sending only 1 value----------
	void lcdPrint(String output) {
		// examples: "Score: xxx\n, direct: LEFT, RIGHT....
		String upper;
		String lower;
		uint16_t newLineIndex = (uint16_t) output.indexOf('\n');
		lcd->clear();
		lcd->setCursor(0, 0);
		if (output.indexOf('\n') > 0) {
			upper = output.substring(0, newLineIndex);
			lower = output.substring(newLineIndex + 1);
			lcd->print(upper);
			lcd->setCursor(0, 1);
			lcd->print(lower);
		}
		else {
			upper = output;
			lcd->print(upper);
		}
	}

	// to be call in everycycle
	void print() {
		if (status == GAME_STANDBY) {
			// ---Snake Game---
			// Speed: < 1234 >
			// keep speed as 4 digit
			if (speed > 999) {
				lcd->setCursor(9, 1);
				lcd->print(String(speed));
			}
			else {
				lcd->setCursor(9, 1);
				lcd->print(' ');
				lcd->print(String(speed));
			}
		}
		else if (status == GAME_INGAME) {
			// Score: 64
			// Direction: RIGHT
			lcd->setCursor(7,0);
			if (score < 10){
				lcd->print(" ");
				lcd->print(score);
			}
			else {
				lcd->print(score);
			}

			lcd->setCursor(11, 1);
			switch (direction) {
				case UP:
					lcd->print("UP   ");
					break;
				case DOWN:
					lcd->print("DOWN ");
					break;
				case LEFT:
					lcd->print("LEFT ");
					break;
				case RIGHT:
					lcd->print("RIGHT");
					break;
			}
		}
		else{
			// ---game end---
			// score: 54
			lcdPrint("---Game End---\nScore: " + String(score));
		}
	}

	// ----------set of getButton function-----------
	ButtonShield getButtonVal() {
		// threshold may need to change base on the real shield
		int x = analogRead(buttonPin);
		if (x < 60) {
			// RIGHT
			return BUTTON_RIGHT;
		}
		else if (x < 200) {
			return BUTTON_UP;
		}
		else if (x < 400) {
			return BUTTON_DOWN;
		}
		else if (x < 600) {
			return BUTTON_LEFT;
		}
		else if (x < 800) {
			return BUTTON_SELECT;
		}
		else {
			return BUTTON_NOTHING;
		}
	}

	// to be call everytime in the loop. consider it as a button up
	void reactButton() {
		ButtonShield buttonNew = getButtonVal();
		if (buttonCurrent == buttonNew) {
			// nothing happen, still pressing or do not press
			return;
		}
		else {
			// button state change
			if (buttonNew == BUTTON_NOTHING) {
				// keyup
				if (status == GAME_STANDBY) {
					// response to the button
					if (buttonCurrent == BUTTON_SELECT) {
						// activate the game with speed by send the speed
						"SENDING start and speed message to board";
						// SPEED:0100
						String mess = "SPEED:" + String(speed);
						commsHub->send(mess);
						status = GAME_INGAME;

						// move lcd to INGAME commMode:
						// Score: __
						// Direction: _____
						String ingameMessage = "Score: \nDirection: ";
						lcdPrint(ingameMessage);
						print();
					}
					else {
						// update speed selection
						switch (buttonCurrent){
							case BUTTON_DOWN:
							case BUTTON_LEFT:
								// 100 is minimum speed
								if (speed >= 150){
									speed -= 50;
								}
								else {
									speed = 100;
								}
								print();
								break;
							case BUTTON_UP:
							case BUTTON_RIGHT:
								if (speed >= 2950){
									speed = 3000;
								}
								else {
									speed += 50;
								}
								print();
								break;
							default:
								break;

						}
					}
				}
				else if (status == GAME_INGAME){
					// new direction
					"UPDATE the direction add to queue";
					// add to queue
					switch (buttonCurrent){
						case BUTTON_LEFT:
							if (directQueue.get(directQueue.size() - 1) != LEFT){
								directQueue.append(LEFT);
							}
							break;
						case BUTTON_RIGHT:
							if (directQueue.get(directQueue.size() - 1) != RIGHT){
								directQueue.append(RIGHT);
							}
							break;
						case BUTTON_UP:
							if (directQueue.get(directQueue.size() - 1) != UP){
								directQueue.append(UP);
							}
							break;
						case BUTTON_DOWN:{
							if (directQueue.get(directQueue.size() - 1) != DOWN){
								directQueue.append(DOWN);
							}
							break;
						}
						default:
							break;
					}
				}
				else {
					// endgame
					// to be develop, select to continue or reset....

				}
			}
			else {
				buttonCurrent = buttonNew;
			}
		}
	}



	// --------Communication control -------------
	void reactResponse(){
		if (commMode == SOFTWARESERIAL_COM){
			// SCORE:64
			String response = commsHub->request(15);
			response.trim();
			if (response.substring(0, 5) == "SCORE"){
				try {
					score = (unsigned int) response.substring(6, 8).toInt();
				}
				catch (int e){
					lcdPrint("Error:\n" + response);
				}
			}
			else if (response.substring(0,3) == "END"){
				status = GAME_END;
			}
		}
		else if (commMode == I2C_COM){
			"FIXME";
		}
		else{
			"FIXME:";
		}
	}
	void reactSend(){
		// in standby, nothing is send, the kick start is send at the reactButton
		if (status == GAME_INGAME){
			if (directQueue.size() > 0){
				Direction currentOrient = directQueue.shift();
				String output = "DIRECTION:";
				switch (currentOrient) {
					case UP:
						output.concat("UP___");
						break;
					case DOWN:
						output.concat("DOWN_");
						break;
					case LEFT:
						output.concat("LEFT_");
						break;
					case RIGHT:
						output.concat("RIGHT");
						break;
					default:
						if (DEBUG){
							lcdPrint("Error:\n" + output);
						}
						break;
				}
				commsHub->send(output);
			}
		}
		else if (status == GAME_END){
			"FIXME:";
		}
	}

	// -----------running process----------
	void run(){
		bool keepRunning = true;
		while (keepRunning){
			reactResponse();
			print();
			reactButton();
			reactSend();
			"react send, if ingame: sending the queue if exist element";
		}
	}

private:
	// for shield
	LiquidCrystal *lcd;
	uint8_t buttonPin;

	// for communication, for now, only alow I2C or SoftwareSerial, future to allow via wifi
	uint8_t pinRx_SDA;
	uint8_t pinTx_SDL;
	CommMode commMode;
	CommsHub *commsHub;

	// for controller module
	unsigned short speed;
	unsigned int score;
	unsigned int analogVal;
	Direction direction;
	ButtonShield buttonCurrent;
	GameMode status; // standby when select speed
	Vector<Direction> directQueue;
};

#endif //ARDUINOSNAKE_SNAKECONTROLLER_H
