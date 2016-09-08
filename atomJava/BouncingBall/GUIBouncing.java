package BouncingBall;

import java.util.*;
import java.awt.*;
import java.util.concurrent.*;

import javax.swing.*;
import java.awt.event.*;

public class GUIBouncing extends JFrame {
	//private BouncingThreading thread;
	private final long interval = 10;

	private class BallPanel extends JPanel {
		private ArrayList<Ball> ballList;
		private Color[] ballColorList = new Color[]{
				Color.RED,
				Color.ORANGE,
				Color.YELLOW,
				Color.GREEN,
				Color.CYAN,
				Color.BLUE,
				new Color( 128, 0, 128 )
		};
		private Random randGenerator = new Random();


		public BallPanel(ArrayList<Ball> list) {
			ballList = list;

			// mouse listener
			addMouseListener(
					new MouseAdapter() {
						//private boolean released = false;
						private Ball newBall;
						private Thread increaseRThread;

						@Override
						public void mousePressed(MouseEvent e) {
							newBall =new Ball(e.getX(), e.getY(), ballColorList[randGenerator.nextInt(ballColorList.length)], 2);
							ballList.add(newBall);
							increaseRThread = new Thread(new Runnable() {
								private final int delay = 10;
								@Override
								public void run() {
									while (!newBall.getActive()){
										try{
											Thread.sleep(delay);
										}
										catch (InterruptedException except){
											System.err.println("Increase radius thread interrupted\n");
											return;
										}
										newBall.setR(newBall.getR() + delay / 10);
									}
								}
							});
							increaseRThread.start();
						}

						@Override
						public void mouseReleased(MouseEvent e) {
							//released = true;
							newBall.activate();
						}

					}
			);

		}

		public void addBall(Ball newBall) {
			ballList.add(newBall);
		}

		@Override
		public void paintComponent(Graphics g) {
			super.paintComponent(g); // clear the Panel

			Graphics2D g2d = (Graphics2D) g;
			g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

			g2d.setColor(Color.BLACK);
			g2d.drawLine(0, (int) Ball.getMaxHeight(), GUIBouncing.this.getWidth(), (int) Ball.getMaxHeight());

			//g2d.setColor(Color.BLUE);

			for (Ball aBallList : ballList) {
				int x = aBallList.getX();
				int y = aBallList.getY();
				int r = aBallList.getR();
				Color color = aBallList.getColor();
				g2d.setColor(color);
				g2d.fillOval(x - r, y - r, r * 2, r * 2);
			}
		}
	}

	private ArrayList<Ball> ballList;
	private BallPanel panel;

	public GUIBouncing() {
		super("Bouncing game");

		// to setup the accelerated openGL.
		System.setProperty("sun.java2d.opengl", "true");

		// declare component
		ballList = new ArrayList<Ball>(10);
		//ballList.add(new Ball(100,400));

		panel = new BallPanel(ballList);

		//add component
		setContentPane(panel);
		// setup component
		BouncingThreading thread = new BouncingThreading(getBallList(), getPanel());
		ExecutorService service = Executors.newCachedThreadPool();
		service.execute(thread);
		service.shutdown();

		//final setup

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		pack();
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		setSize((int) screenSize.getWidth(), (int) Ball.getMaxHeight() + 50);
		setVisible(true);
	}

	public JPanel getPanel() {
		return panel;
	}

	public ArrayList<Ball> getBallList() {
		return ballList;
	}
}
