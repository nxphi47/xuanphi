package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class PrintTask implements Runnable{
    private String taskName;
    private final int time;
    private static Random generator = new Random();

    public PrintTask(String name){
        taskName = name;
        time = generator.nextInt(5000); // random time up to 5s
    }

    public String getTaskName(){
        return taskName;
    }

    @Override
    public void run(){
        // try to put in a sleep mode
        try{
            System.out.printf("task %s start to sleep for %d sec.\n", taskName, time);
            Thread.sleep(time);
            System.out.printf("task %s finished\n", taskName);
        }
        catch (InterruptedException e){
            e.printStackTrace();
        }
        System.out.printf("task %s finished\n", taskName);

    }
}
