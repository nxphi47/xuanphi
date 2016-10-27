package MultiThread;

import java.util.*;
import java.util.concurrent.*;
import java.awt.*;
import javax.swing.*;

public class BlockingBuffer implements Buffer{
    private final ArrayBlockingQueue<Integer> buff;
    public BlockingBuffer(){
        buff = new ArrayBlockingQueue<Integer>(1);

    }

    @Override
    public void set(int val) throws InterruptedException{
        buff.put(val);
        System.out.printf("%s%2d\t%s%d\n", "Producer writes ", val, "cells occupied: ", buff.size());
    }

    @Override
    public int get() throws InterruptedException{
        int readVal = buff.take();
        System.out.printf("%s%2d\t%s%d\n", "Consumer reads  ", readVal, "cells occupied: ", buff.size());
        return readVal;
    }
}
