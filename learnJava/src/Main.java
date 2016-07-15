
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Scanner;

class Main {

    public static void main(String[] args) throws ScriptException {
	// write your code here
        Time x = new Time(3,10);
        ArrayList<Time> list1 = new ArrayList<>();
        list1.add(new Time(6,24,34));
        list1.add(new Time(20, 30, 2));
        ArrayList<Time> list2 = new ArrayList<>();
        list2.add(x);
        list2.add(new Time(5, 20));
        list2.add(new Time(12, 5,1));
        Time[] arr = {
                new Time(5,29),
                new Time(12,5,1),
                x
        };

        Collections.addAll(list1, arr);
        Collections.sort(list1);
        for (Time y : list1){
            System.out.printf("%s\n", y);
        }

        System.out.printf("min: %s\n", Collections.min(list1));
        System.out.printf("max: %s\n", Collections.max(list1));
        System.out.printf("search 3,10: %d\n", Collections.binarySearch(list1, x));
    }
}
