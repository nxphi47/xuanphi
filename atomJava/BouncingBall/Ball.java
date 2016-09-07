package BouncingBall;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

/*
1000 px = 10m
1px = 0.02m
g = -9.81 m/s2 = -9.81x10^-6 m/milisec^2

*/

public class Ball{
    private int x; //px
    private int y;
    private int r;
    private double realX; // meters
    private double realY;
    private double realYo;
    private double realR;
    private double velocity; // means the velocity at bouncing
    private boolean down;
    private long time; // milisec

    private final double pxToM = 0.02;
    private final double gravity = 9.81;

    public Ball(int x, int y, int r){
        this.x = x;
        this.y = y;
        this.r = r;
        realR = r * pxToM;
        realX = x * pxToM;

        realY = y * pxToM + realR; // is actually the bottom point
        realYo = realY;
        this.time = 0;
        down = true;
        int endPoint = y + r;
        //velocity = Math.sqrt(2*9.81*(1000-endPoint)*0.01);
        velocity = 0;
    }

    public Ball(int x, int y){
        this(x, y, 10); // default radius is 10
    }

    public synchronized double move(long t){
        double deltaT = (double) t / 1000;
        double newY = realY + velocity*deltaT + 0.5*gravity*deltaT*deltaT;
        if (newY < 10) {
            realY = newY;
            velocity = velocity + gravity*deltaT;
        }
        else {
            realY = 20 - newY;
            velocity = -Math.sqrt(2*gravity*(realY - realYo));
        }

        return realY;
    }

    public synchronized void setVelo(double velo){
        velocity = velo;
    }
    public synchronized double getVelo(){
        return velocity;
    }

    public synchronized void setDown(boolean down){
        this.down = down;
    }

    public synchronized boolean getDown(){
        return down;
    }

    public synchronized void setX(int x){
        this.x = x;
    }
    public synchronized int getX(){
        //x = Math.floor(realX / pxToM);
        x = (int) (realX / pxToM);
        return x;
    }

    public synchronized void setY(int y){
        this.y = y;
    }
    public synchronized int getY(){
        y = (int) ((realY - realR) / pxToM);
        //y = Math.round((realY - realR) / pxToM);
        //y = Integer.parseInt((realY - realR) / pxToM);
        return y;
    }

    public synchronized void setR(int r){
        this.r = r;
    }
    public synchronized int getR(){
        return r;
    }
}
