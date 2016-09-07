package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class UnsynchronizedBuffer implements Buffer{
    private int buff = -1; // -1 to indicate when the consumed before produced

    public UnsynchronizedBuffer(){

    }

    @Override
    public int get() throws InterruptedException{
        System.out.printf("Consumer reads \t%d", buff);
        return buff;
    }
    @Override
    public void set(int val) throws InterruptedException{
        System.out.printf("Producer write \t%d", val);
        buff = val;
    }

}
