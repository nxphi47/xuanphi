import java.util.Comparator;

/**
 * Created by phi on 15/07/16.
 */
public class Time implements Comparator<Time>, Comparable<Time > {
    private int hour;
    private int minute;
    private int second;
    public Time(int h, int m, int s){
        hour = h;
        minute = m;
        second = s;
    }
    public Time(int h, int m){
        this(h,m, 0);
    }
    public Time(int h){
        this(h,0);
    }
    public Time(){
        this(0);
    }

    public int getOverallTime(){
        return hour*3600 + minute*60 + second;
    }

    @Override
    public String toString(){
        return String.format("%d:%d:%d", hour, minute, second);
    }


    @Override
    public int compare(Time o1, Time o2) {
        return o1.getOverallTime() - o2.getOverallTime();
    }

    @Override
    public int compareTo(Time o) {
        return this.getOverallTime() - o.getOverallTime();
    }
}
