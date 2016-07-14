
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import java.util.Scanner;

class Main {

    public static void main(String[] args) throws ScriptException {
	// write your code here
        Scanner scanner = new Scanner(System.in);
        System.out.printf("input: ");
        String str = "123414 hell";
        System.out.printf("%s", String.valueOf(str.matches("\\d+\\s+([A-Za-z]{3,4}|[A-Za-z]+\\s[A-Za-z]+)")));

    }
}
