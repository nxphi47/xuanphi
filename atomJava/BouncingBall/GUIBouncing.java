package BouncingBall;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

public class GUIBouncing extends JFrame{
    //private BouncingThreading thread;
    private final long interval = 10;
    private class BallPanel extends JPanel{
        private ArrayList<Ball> ballList;

        public BallPanel(ArrayList<Ball> list){
            ballList = list;

            // mouse listener
            addMouseListener(
                new MouseAdapter(){
                    @Override
                    public void mouseClicked(MouseEvent e){
                        BallPanel.this.addBall(new Ball(e.getX(), e.getY(), 10)); // radius r
                        System.out.printf("add new ball\n");
                        repaint();
                    }
                }
            );

            // bouncing right here
            /*
            Thread game = new Thread(){
                @Override
                public void run(){
                    // update all the ballList
                    while(true){
                        try{
                            Thread.sleep(interval);
                        }
                        catch (InterruptedException e){
                            e.printStackTrace();
                        }
                        for (int i = 0; i < ballList.size(); i++) {
                            ballList.get(i).move(interval);
                        }
                        repaint();
                    }
                }
            };
            game.start();
            */
        }


        public void addBall(Ball newBall){
            ballList.add(newBall);
        }

        @Override
        public void paintComponent(Graphics g){
            super.paintComponent(g); // clear the Panel


            Graphics2D g2d = (Graphics2D) g;
            g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
            g2d.setColor(Color.BLUE);

            for (int i = 0; i < ballList.size(); i++) {
                int x = ballList.get(i).getX();
                int y = ballList.get(i).getY();
                int r = ballList.get(i).getR();
                g2d.fillOval(x - r, y - r, r * 2, r * 2);
            }
        }
    }
    private ArrayList<Ball> ballList;
    private BallPanel panel;

    public GUIBouncing(){
        super("Bouncing game");
        // declare component
        ballList = new ArrayList<Ball>(10);
        //ballList.add(new Ball(100,400));

        panel = new BallPanel(ballList);

        //add component
        setContentPane(panel);
        // setup component

        //System.out.printf("done start the thread");

        //final setup

        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        pack();
        setSize(1000,500);
        setVisible(true);
    }

    public JPanel getPanel(){
        return panel;
    }

    public ArrayList<Ball> getBallList(){
        return ballList;
    }
}
