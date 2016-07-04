//
// Created by phi on 01/07/16.
//

#ifndef CPPLIB_SNAKE_H
#define CPPLIB_SNAKE_H

#include "Vector.h"

struct Coor {
    unsigned int X;
    unsigned int Y;
};

enum Direction {
    UP, RIGHT, DOWN, LEFT
};

class Snake {
public:

    Snake(int boardSize = 8, int initialLength = 2, bool wall = false) {
        this->boardSize = boardSize;
        wallOrNot = wall;
        list = Vector<Coor>();
        direct = RIGHT;
        food = {(unsigned int) (boardSize - 1), (unsigned int) (boardSize - 1)};

        // intialise the list
        for (int i = 0; i < initialLength; ++i) {
            Coor newDot = {(unsigned int) i, 0};
            list.insert(0, newDot);
        }
        updateLength();
    }

    Snake *updateLength() {
        length = list.size();
        return this;
    }

    int getLength() {
        updateLength();
        return length;
    }

    Vector<Coor> getList() {
        return list;
    }


    bool checkNewPosition(int x, int y) {
        updateLength();

        // check through all the snake, exept the last and the begining
        for (int i = 1; i < length - 1; ++i) {
            if (x == list.get(i).X && y == list.get(i).Y) {
                printf("\ntouch snake at: %d %d, index %d\n", x, y, i);
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
        int x = list.get(0).X;
        int y = list.get(0).Y;
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
                fprintf(stderr, "Snake: movedirection: unable to move with orient");
        }
        // start move with x and y
        return moveReal(x, y);
    }
    bool moveReal(int x, int y) {
        if (checkNewPosition(x, y)) {
            // bring the tail to the head
            // if it is the position of food
            if (x == food.X && y == food.Y) {
                // eat the food
                Coor newNode = {(unsigned int) x, (unsigned int) y};
                list.insert(0, newNode);
                updateLength();

                // generate food, temporary hasing
                food.X = (food.X * 11) % boardSize;
                food.Y = (food.Y * 11) % boardSize;
                return true;
            }
            else {
                // move normally
                Coor tail = list.pop();
                tail.X = (unsigned int) x;
                tail.Y = (unsigned int) y;
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

    // direction set function
    Direction getDirection(){
        return direct;
    }
    void setDirection(Direction orient){
        direct = orient;
    }

    // food generation
    Coor getFood(){
        return food;
    }
    ///////

private:
    Vector<Coor> list;
    Coor food;
    int length;
    Direction direct;
    bool wallOrNot; // to be
    int boardSize;
};


#endif //CPPLIB_SNAKE_H
