package BouncingBall;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class MainBouncing{
    public static void main(String[] args) {
        GUIBouncing app = new GUIBouncing();


        BouncingThreading thread = new BouncingThreading(app.getBallList(), app.getPanel());

        ExecutorService service = Executors.newCachedThreadPool();
        service.execute(thread);
        service.shutdown();
        

        /*
        BouncingSwingWorker worker = new BouncingSwingWorker(app.getBallList(), app.getPanel());
        worker.execute();
        */

        ArrayList<Ball> ballList = new ArrayList<Ball>(10);
        ballList.add(new Ball(50,50));
        System.out.printf("%d %d", ballList.get(0).getX(), ballList.get(0).getY());
    }
}
