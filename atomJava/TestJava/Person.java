package TestJava;

import java.util.*;
import java.awt.*;
import javax.swing.*;

public class Person{
    private String name;
    private int age;
    private long FINnumber;

    public Person(String name, int age, long FINnumber){
        this.name = name;
        this.age = age;
        this.FINnumber = FINnumber;
    }

    public String getName(){
        return this.name;
    }

    public int getAge(){
        return this.age;
    }

    public long getFINnumber(){
        return this.FINnumber;
    }
}
