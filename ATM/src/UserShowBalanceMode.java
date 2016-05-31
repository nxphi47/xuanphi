import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by phi on 31/05/16.
 */
public class UserShowBalanceMode extends Mode {
    private Account userAccount;

    public UserShowBalanceMode(Account user, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        setUserAccount(user);
        // set handler for the buttons
        for (JButton button: getButtons()){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(
                    new ActionListener() {
                        @Override
                        public void actionPerformed(ActionEvent e) {
                            if (e.getActionCommand().equals("Cancel")){
                                setExecuting(false);
                            }
                            else {
                                getText().setText(getUserAccount().showBalance() + "\nCancel to end");
                            }
                        }
                    }
            );
        }
    }

    public Account getUserAccount() {
        return userAccount;
    }

    public void setUserAccount(Account user) {
        if (getUserAccount() != null){
            getUserAccount().setAuthorized(false);
        }
        if (user == null){
            System.err.println("Show balance: input user account = null");
            System.exit(1);
        }
        this.userAccount = user;
        getUserAccount().setAuthorized(true);
    }

    @Override
    public void execute() {
        setExecuting(true);
        getText().setText(getUserAccount().showBalance() + "\nCancel to end");
        while (isExecuting()){
            System.out.printf("Showbalance going\n");
        }
    }
}
