package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class MainConcurrent{
    public static void main(String[] args) {
        GUIPrimeFinding app = new GUIPrimeFinding();

        /*
        PrintTask task1 = new PrintTask("task1");
        PrintTask task2 = new PrintTask("task2");
        PrintTask task3 = new PrintTask("task3");

        System.out.printf("main thread started\n");

        Buffer shared = new BlockingBuffer();
        ExecutorService threadExecutor = Executors.newCachedThreadPool();

        threadExecutor.execute(new Producer(shared));
        threadExecutor.execute(new Consumer(shared));

        threadExecutor.shutdown();

        try{
            threadExecutor.awaitTermination(1, TimeUnit.MINUTES);
            System.out.println("main ended.");

        }
        catch (InterruptedException e){
            System.err.println("main got InterruptedException");
        }
        // main end;

        */
    }
}
