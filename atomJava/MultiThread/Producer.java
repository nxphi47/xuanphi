package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class Producer implements Runnable{
    private final static Random generator = new Random();
    private final Buffer sharedData;

    public Producer(Buffer buff){
        sharedData = buff;
    }

    @Override
    public void run(){
        int sum = 0;
        for (int i = 1; i <= 10; i++ ) {
            try{
                Thread.sleep(generator.nextInt(3000));
                sharedData.set(i);
                sum+=i;
                System.out.printf("\t%2d\n", sum);

            }
            catch (InterruptedException e){
                e.printStackTrace();
            }
        }
        System.out.printf("Done Producer\n");
    }
}
