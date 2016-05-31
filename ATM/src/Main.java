import javax.swing.*;
import java.util.Scanner;

public class Main {

    public static void main(String[] args) {
	// write your code here
        Keypad keypad = new Keypad();
        JFrame app = new JFrame("Testing");
        app.add(keypad);
        app.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        app.setSize(500,300);
        app.setVisible(true);
    }
}