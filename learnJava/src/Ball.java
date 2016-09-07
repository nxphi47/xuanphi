/*
250 px = 2.5m
1px = 0.01m
g = -9.81 m/s2 = -9.81x10^-6 m/milisec^2

*/

public class Ball {
	private int x; //px
	private int y0;
	private int y;
	private int r;
	private double realX; // meters
	private double realY;
	private double realYo;
	private double realR;
	private double velocity; // means the velocity at bouncing
	private boolean down;
	private double time; // milisec
	private double period;
	private long periodLong;
	private long timeLong;

	private final double pxToM = 1;
	private final double gravity = 9.81e-4; // cm/milisec^2
	private final double maxHeight = 250; // centimeters

	public Ball(int x, int y, int r) {
		this.x = x;
		this.y0 = y;
		this.y = y;
		this.r = r;
		realR = r * pxToM;
		realX = x * pxToM;

		realY = y * pxToM + realR; // is actually the bottom point
		realYo = realY;
		this.time = 0;
		timeLong = 0;
		periodLong = (int) (2 * Math.round(Math.sqrt(2 * (maxHeight - y0) / gravity)));
		period = 2 * Math.sqrt(2 * (maxHeight - realY) / gravity);
		down = true;
		int endPoint = y + r;
		//velocity = Math.sqrt(2*9.81*(1000-endPoint)*0.01);
		velocity = 0;
	}

	public Ball(int x, int y) {
		this(x, y, 10); // default radius is 10
	}

	public long move(long t) { // milisection

		/*
		time += (double) t / 1000;
		while (time > period){
			time -= period;
		}
		if (time < period / 2){
			realY = realYo + 0.5 * gravity * time * time;
		}
		else {
			// moving up
			double upTime = period - time;
			realY = realYo + 0.5 * gravity * upTime * upTime;
		}
		y = (int) ((realY - realR) / pxToM);
		*/
		// change to int version
		timeLong += t;
		while (timeLong > periodLong){
			timeLong -= period;
		}
		if (timeLong < periodLong / 2){
			y = (int) (y0 + gravity * timeLong * timeLong / 2);
		}
		else {
			// moving up
			long upTime = periodLong - timeLong;
			y = (int) (y0 + gravity * upTime * upTime / 2);
		}


		return y;
	}

	public synchronized void setVelo(double velo) {
		velocity = velo;
	}

	public synchronized double getVelo() {
		return velocity;
	}

	public synchronized void setDown(boolean down) {
		this.down = down;
	}

	public synchronized boolean getDown() {
		return down;
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

		//y = Math.round((realY - realR) / pxToM);
		//y = Integer.parseInt((realY - realR) / pxToM);
		return y;
	}

	public synchronized void setR(int r) {
		this.r = r;
	}

	public synchronized int getR() {
		return r;
	}
}
