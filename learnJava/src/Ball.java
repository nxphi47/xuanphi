/*
250 px = 2.5m
1px = 0.01m
g = -9.81 m/s2 = -9.81x10^-6 m/milisec^2

*/

import java.awt.*;

public class Ball {
	private int x; //px
	private int y0;
	private int y;
	private int r;
	private long periodLong;
	private long timeLong;
	private boolean active;
	private Color color;

	//private final double pxToM = 1;
	private final double gravity = 9.81e-4; // cm/milisec^2
	private static final double maxHeight = 650; // centimeters

	public Ball(int x, int y, Color color, int r) {
		this.x = x;
		this.y0 = y + r;
		this.y = y;
		this.r = r;
		this.color = color;
		//this.time = 0;
		timeLong = 0;
		periodLong = (int) (2 * Math.round(Math.sqrt(2 * (maxHeight - y0) / gravity)));
		//period = 2 * Math.sqrt(2 * (maxHeight - realY) / gravity);

	}

	public Ball(int x, int y) {
		this(x, y, Color.BLACK, 10); // default radius is 10
	}

	public long move(long t) { // milisection
		if (!active){
			return y;
		}

		timeLong += t;
		while (timeLong > periodLong){
			timeLong -= periodLong;
		}
		if (timeLong < periodLong / 2){
			y = (int) (y0 + gravity * timeLong * timeLong / 2 - r);
		}
		else {
			// moving up
			long upTime = periodLong - timeLong;
			y = (int) (y0 + gravity * upTime * upTime / 2 - r);
		}


		return y;
	}

	public static double getMaxHeight(){
		return maxHeight;
	}

	public synchronized void activate(){
		active = true;
	}
	public synchronized void deactivate(){
		active = false;
	}
	public synchronized boolean getActive(){
		return active;
	}

	public synchronized void setX(int x) {
		this.x = x;
	}

	public synchronized int getX() {
		//x = Math.floor(realX / pxToM);
		//x = (int) (realX / pxToM);
		return x;
	}

	public synchronized void setY(int y) {
		this.y = y;
	}

	public synchronized int getY() {
		return y;
	}

	public synchronized void setR(int r) {
		y0 = y0 - this.r + r;
		periodLong = (int) (2 * Math.round(Math.sqrt(2 * (maxHeight - y0) / gravity)));
		this.r = r;
	}

	public synchronized int getR() {
		return r;
	}

	public synchronized Color getColor() {
		return color;
	}

	public synchronized void setColor(Color color) {
		this.color = color;
	}
}
