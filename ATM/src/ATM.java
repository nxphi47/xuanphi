import javax.swing.*;
import java.awt.*;
import java.security.Key;

/**
 * Created by nxphi on 5/31/16.
 */
public class ATM extends JFrame {
    private UserMode userMode;
    private LockMode lockMode;
    private Account account;
    private DataBase dataBase;

    private JTextArea textArea;
    private Keypad keypad;

    private void display(){
        textArea = new JTextArea("Singapore abcedasd\nsdasdsad\nasdasd\nasdsaaww", 5,15);
        textArea.setFont(new Font("Serif", Font.PLAIN, 20));
        textArea.setEditable(false);
        textArea.setDisabledTextColor(Color.BLACK);
        keypad = new Keypad();
        setSize(500,500);
        setLayout(new GridLayout(2,1,10,10));
        add(textArea);
        add(keypad);
        setVisible(true);
    }

    public ATM(){
        super("Phi's ATM machine");
        dataBase = new DataBase("dataBase.txt");
        display();
        lockMode = new LockMode(dataBase, textArea, keypad.getButtons());
        userMode = new UserMode(lockMode.getUserAccount(), textArea, keypad.getButtons());
        lockMode.setSuperMode(userMode);
        userMode.setSuperMode(lockMode);
        lockMode.execute();
    }

    public void execute(){
        lockMode.execute();
        account = lockMode.getUserAccount();

        if (account == null){
            System.err.println("Error, lockmode get null account");
            System.exit(1);
        }

        userMode = new UserMode(account, textArea, keypad.getButtons());
        userMode.execute();
    }

}
