package BouncingBall;

import java.util.*;
import java.util.concurrent.*;
//import java.awt.*; // may cause collision
import javax.swing.JPanel;
import javax.swing.SwingWorker;

public class BouncingSwingWorker extends SwingWorker<Void, Void> {
    private ArrayList<Ball> ballList;
    private JPanel panel;
    private long time;
    private final long interval = 10; // milisec

    public BouncingSwingWorker(ArrayList<Ball> list, JPanel panel){
        ballList = list;
        this.panel = panel;
        time = 0;
    }

    // supposed tobe called every 1 milisec
    public void updateList(long t){
        for (int i = 0; i < ballList.size(); i++ ) {
            ballList.get(i).move(t);
            //System.out.printf("ball %d at %d %d\n", i, ballList.get(i).getX(), ballList.get(i).getY());
        }
    }

    @Override
    public Void doInBackground(){
        while(time >= 0){
            try{
                Thread.sleep(interval);

                time++;
            }
            catch (InterruptedException e){
                System.err.println("Program interrupted");
                return null;
            }
            updateList(interval);
            publish();
        }
        return null;
    }

    @Override
    protected void process(List<Void> list){
        panel.repaint();
    }
}
