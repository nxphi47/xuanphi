import javax.swing.*;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.util.ArrayList;

/**
 * Created by nxphi on 8/29/16.
 * Paint panel in realtime, handle all mouse listener and draw the panel
 */
public class PaintRealTimePanel extends JPanel {
	private JLabel mouseInfoLabel; // the bottom information of mouse motion
	private Color currentColor;

	// for test polygon
	private ArrayList<Integer> xPoints;
	private ArrayList<Integer> yPoints;
	private int counter;
	private final int totalPoints = 10000;
	private int dotSize;

	// contructor
	public PaintRealTimePanel() {
		super();
		// need to add the mouse Infor label
		currentColor = Color.BLACK;
		xPoints = new ArrayList<>(totalPoints);
		//System.out.printf(String.valueOf(xPoints.get(xPoints.size() - 1)));
		yPoints = new ArrayList<>(totalPoints);
		counter = 0;
		setDotSize(5);
	}

	// main paintComponent method;

	@Override
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);
		g.setColor(currentColor);
		//int[] xArray = convertListToInt(xPoints);
		//int[] yArray = convertListToInt(yPoints);

		for (int i = 0; i < xPoints.size(); i++) {

			if (xPoints.get(i) > getDotSize() && yPoints.get(i) > getDotSize()) {
				g.fillOval(xPoints.get(i) - getDotSize(), yPoints.get(i) - getDotSize(), getDotSize() * 2, getDotSize() * 2);
				//g.drawPolyline(xPoints.);
			}
		}

		//g.fillOval(xPoints.get(xPoints.size() - 1) - getDotSize(), yPoints.get(yPoints.size() - 1) - getDotSize(), getDotSize() * 2, getDotSize() * 2);

	}

	// add mouse listener for information Label and other listener
	public void addInfoLabelListener() {
		addMouseMotionListener(new InforLabelListener());
	}

	public void addPolygonDrawListener() {
		removeListenersExceptInfoListener();

		PolygonDrawListener polygonDrawListener = new PolygonDrawListener();
		addMouseMotionListener(polygonDrawListener);
		addMouseListener(polygonDrawListener);
	}

	public void addEraseDrawListener() {
		removeListenersExceptInfoListener();

		EraseListener eraseListener = new EraseListener(5);
		addMouseMotionListener(eraseListener);
		addMouseListener(eraseListener);
	}


	// remove all listener except the information listener
	public void removeListenersExceptInfoListener() {
		MouseMotionListener[] listOfListener = getMouseMotionListeners();
		for (MouseMotionListener listener : listOfListener) {
			if (listener.getClass() != InforLabelListener.class) {
				removeMouseMotionListener(listener);
			}
		}
	}


	public JLabel getMouseInfoLabel() {
		return mouseInfoLabel;
	}

	public void setMouseInfoLabel(JLabel mouseInfoLabel) {
		this.mouseInfoLabel = mouseInfoLabel;
	}

	// operation
	public void clearPaint() {
		resetPoints();
		repaint();
	}

	public void resetPoints() {
		xPoints.clear();
		yPoints.clear();
		counter = 0;
	}
	public int[] convertListToInt(ArrayList<Integer> list){
		int[] array = new int[list.size()];
		for (int i = 0; i < list.size(); i++){
			array[i] = list.get(i);
		}
		return array;
	}

	public void setColor(Color color) {
		currentColor = color;
	}

	public int getDotSize() {
		return dotSize;
	}

	public void setDotSize(int dotSize) {
		this.dotSize = dotSize;
	}

	// List of listener
	private class PolygonDrawListener extends MouseAdapter {
		@Override
		public void mouseClicked(MouseEvent e) {
			if (counter < totalPoints) {
				/*
				xPoints.set(counter, e.getX());
				yPoints.set(counter, e.getY());
				counter++;
				*/
				xPoints.add(e.getX());
				yPoints.add(e.getY());
			} else {
				resetPoints();
			}
			repaint();
		}

		@Override
		public void mouseDragged(MouseEvent e) {
			if (counter < totalPoints) {
				/*
				xPoints.set(counter, e.getX());
				yPoints.set(counter, e.getY());
				counter++;
				*/
				xPoints.add(e.getX());
				yPoints.add(e.getY());
			} else {
				resetPoints();
			}
			repaint();
		}

		@Override
		public void mouseMoved(MouseEvent e) {
		}
	}

	// listener for infoLabel
	private class InforLabelListener implements MouseMotionListener {
		@Override
		public void mouseDragged(MouseEvent e) {
			mouseInfoLabel.setText(String.format("Mouse dragged at [%d, %d]", e.getX(), e.getY()));
		}

		@Override
		public void mouseMoved(MouseEvent e) {
			mouseInfoLabel.setText(String.format("Mouse moved at [%d, %d]", e.getX(), e.getY()));
		}
	}

	//listener for erase button
	// FIXME: since the eraser has to cut off the the polygon, how can I deal with the xPoints and yPoints
	private class EraseListener extends MouseAdapter {
		private int eraseSize;

		public EraseListener() {
			this(5);

		}

		public EraseListener(int eraseSize) {
			setEraseSize(eraseSize);
		}

		public int getEraseSize() {
			return eraseSize;
		}

		public void setEraseSize(int eraseSize) {
			this.eraseSize = eraseSize;
		}

		@Override
		public void mousePressed(MouseEvent e) {

		}
	}
}
