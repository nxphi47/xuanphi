import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by phi on 31/05/16.
 */
public class UserShowBalanceMode extends Mode {

    public UserShowBalanceMode(Account user, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        setUserAccount(user);
        getText().setText(getUserAccount().showBalance() + "\nCancel to end");

    }

    @Override
    public void handlingButton() {
        // set handler for the buttons
        for (JButton button: getButtons()){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(
                    e -> {
                        if (e.getActionCommand().equals("Cancel")){
                            setExecuting(false);
                            getSuperMode().execute();
                        }
                        else {
                            getText().setText(getUserAccount().showBalance() + "\nCancel to end");
                        }
                    }
            );
        }

    }


    @Override
    public void execute() {
        getText().setText(getUserAccount().showBalance() + "\nCancel to end");
        handlingButton();
    }
}
