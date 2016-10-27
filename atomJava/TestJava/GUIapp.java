package TestJava;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class GUIapp extends JFrame{
    private JLabel label;
    private JSlider slider;
    private class Rounded extends JPanel{
        private int diameter = 10;

        @Override
        public void paintComponent(Graphics g){
            super.paintComponent(g);
            g.fillOval(10,10,diameter,diameter);
        }

        public void setDiameter(int x){
            diameter = x;
            repaint();
        }

        public int getDiameter(){
            return diameter;
        }

    }

    private Rounded roundPanel;

    public GUIapp(){
        super("example application");
        label = new JLabel("hellow world");

        //add(label, BorderLayout.CENTER);

        addSlider();
        setSize(400,200);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setVisible(true);
    }

    public void addSlider(){
        roundPanel = new Rounded();
        slider = new JSlider(JSlider.HORIZONTAL, 10, 200, 10);
        slider.setMajorTickSpacing(20);
        slider.setPaintTicks(true);
        /*
        slider.addChangeListener(new ChangeListener() {
            @Override
            public void stateChanged(ChangeEvent e){
                roundPanel.setDiameter(slider.getValue());
            }
        });
        */

        slider.addChangeListener(new ChangeListener() {
			@Override
			public void stateChanged(ChangeEvent e) {
				roundPanel.setDiameter(slider.getValue());
			}
		});


        add(roundPanel, BorderLayout.CENTER);
        add(slider, BorderLayout.SOUTH);
    }
}
