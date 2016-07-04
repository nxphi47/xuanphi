//
// Created by phi on 01/07/16.
//

#ifndef CPPLIB_SNAKELOGIC_H
#define CPPLIB_SNAKELOGIC_H

#include "Snake.h"
/*
 * process of the snake logic in run loop, when time interval reach the speed, proceed the move
 * 1/ check incomming message, if yes, process the message and update para
 * 2/ if (time() - lastTime >= speed)
 *          move, update status
 * 3/ check status
 * 4/ print
 * 5/ process output and send back to controller
 */

extern unsigned long millis();
extern void display(Snake theSnake);
extern void processOutput(int status, int score);

class SnakeLogic {
public:
    SnakeLogic(){
        snake = Snake(8, 2, false);
        speed = 100;
        direct = RIGHT;
        status = 0;
        directQueue = Vector<Direction >();
        score = snake.getLength();
        // initialise the commshub
        // display module
    }

    // stanby, waiting for starting signal from the controller
    void standbyMode(){
        status = 0;
        speed = getSpeedFromCommsHub();
        // print standBy message on matrix
    }

    // execute mode, keep looping send and receive until failed or something happen
    void executeMode(){
        status = 1;
        int lastTime = speed;
        // lastTime = millis();
        while (status == 1){
            // check incomming message, updating
            getDirectFromCommsHub();

            if (millis() - lastTime >= speed){
                snake.setDirection(direct);
                bool status = snake.move();
                if (!status){
                    status = 2;
                }
                score = snake.getLength();
            }
            // displaying the snake and food
            displayMatrix();

            // process the output
            processOutput();
        }
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
                displayMatrix();
            }
        }
    }

    // set of function to use
    void processOutput();   // implement the commsHub to send to the controller
    void displayMatrix();   // implement the display class with input is the snake
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
    // the communication hubs
    // display module
};


#endif //CPPLIB_SNAKELOGIC_H
