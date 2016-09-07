package TestJava;

import java.util.*;
import java.awt.*;
import javax.swing.*;

public class Test {
	public static void main(String[] args) {
		System.out.printf("Hello world\n");
		Person phi = new Person("Nguyen Xuan Phi", 20, 1234251);
		System.out.printf(String.format("%-15s%5d%10d\n", phi.getName(), phi.getAge(), phi.getFINnumber()));
        Employee chi = new Employee("Dang Lien Chi", 20, 12323132, "Intel dotcom", 1234.2);
        System.out.printf("%-15s%5d%10d%-15s%10f\n", chi.getName(), chi.getAge(), chi.getFINnumber(), chi.getCompanyName(),
                            chi.getSalary());

		for (String x: args ) {
			System.out.println(x);
		}
		GUIapp app = new GUIapp();
	}
}
