package TestJava;

import java.util.*;
import java.awt.*;
import javax.swing.*;

public class Employee extends Person{
    private String companyName;
    private double salary;

    public Employee(String name, int age, long FINnumber, String companyName, double salary){
        super(name, age, FINnumber);
        this.companyName = companyName;
        this.salary = salary;
    }

    public String getCompanyName(){
        return companyName;
    }

    public double getSalary(){
        return salary;
    }
}
