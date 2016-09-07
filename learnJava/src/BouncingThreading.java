import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

/*
500 px = 10m
1px = 0.02m
g = -9.81 m/s2 = -9.81x10^-6 m/milisec^2

*/

public class BouncingThreading implements Runnable {
	private ArrayList<Ball> ballList;
	private JPanel panel;
	private long time;
	private final long interval = 50;

	public BouncingThreading(ArrayList<Ball> list, JPanel panel) {
		ballList = list;
		this.panel = panel;
		time = 0;
	}

	// supposed tobe called every 1 milisec
	public void updateList(long t) {
		for (int i = 0; i < ballList.size(); i++) {
			ballList.get(i).move(t);
			//System.out.printf("ball %d at %d %d\n", i, ballList.get(i).getX(), ballList.get(i).getY());
		}
	}

	@Override
	public void run() {
		while (true) {
			try {
				Thread.sleep(interval);

			} catch (InterruptedException e) {
				System.err.println("Program interrupted");
				return;
			}
			updateList(interval);
			SwingUtilities.invokeLater(
					new Runnable() {
						@Override
						public void run() {
							panel.repaint();
							//System.out.printf(String.valueOf(SwingUtilities.isEventDispatchThread()));
						}
					}
			);
		}
	}

}
