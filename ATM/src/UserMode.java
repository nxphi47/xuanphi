import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by phi on 31/05/16.
 */
public class UserMode extends Mode {
    private Account userAccount;
    private String[] noti;
    private int option;
    // need the mode for show balance, deposit, withdraw
    private UserShowBalanceMode showBalanceMode;
    private UserWithdrawMode withdrawMode;
    private UserDepositMode depositMode;

    public Account getUserAccount() {
        return userAccount;
    }

    public void setUserAccount(Account userAccount) {
        if (userAccount != null){
            userAccount.setAuthorized(false);
        }
        this.userAccount = userAccount;
        assert userAccount != null;
        userAccount.setAuthorized(true);
        noti[0] = String.format("User: %s", userAccount.getName());
    }

    // constructor
    public UserMode(Account user, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        // user mode in specific
        showBalanceMode = new UserShowBalanceMode(getUserAccount(), getText(), getButtons());
        withdrawMode = new UserWithdrawMode(getUserAccount(), getText(), getButtons());
        depositMode = new UserDepositMode(getUserAccount(), getText(), getButtons());
        // set the notification
        noti =new String[4];
        noti[1] = "Show balance? *";
        noti[2] = "Deposit? ";
        noti[3] = "Withdraw? ";

        option = 1;//show balance
        setUserAccount(user);

        //handling the button
        for (JButton button: buttons){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(new ButtonHandler());
        }
    }

    public void setOption(int choice){
        for (String x: noti){
            if (x.charAt(x.length() - 1) == '*'){
                x = x.substring(0, x.length() - 1);
            }
        }
        noti[choice] = noti[choice].concat("*");
        getText().setText(getNoti());
        option = choice;
    }

    //execute option when ok is pressed
    public void optionExecute(){
        switch (getOption()){
            case 1:
                //show balance
                showBalanceMode.execute();
                break;
            case 2:
                depositMode.execute();
                break;
            case 3:
                withdrawMode.execute();
                break;
        }
    }

    public String getNoti(){
        StringBuilder info = new StringBuilder();
        for (String x: noti){
            info.append(x).append("\n");
        }
        return info.toString();
    }

    public int getOption() {
        return option;
    }


    class ButtonHandler implements ActionListener{
        @Override
        public void actionPerformed(ActionEvent e) {
            if (e.getActionCommand().equals("Cancel")){
                setExecuting(false);
            }
            else if (e.getActionCommand().equals("OK")){
                optionExecute();
            }
            else if (e.getActionCommand().equals("^")){
                // up
                int choice = UserMode.this.option;
                choice -= 1;
                if (choice <= 0) choice+=3;
                setOption(choice);
            }
            else if (e.getActionCommand().equals("v")){
                int choice = UserMode.this.option;
                choice += 1;
                if (choice > 3) choice-=3;
                setOption(choice);
            }
        }
    }

    @Override
    public void execute() {
        // option pane
        setExecuting(true);
        while (isExecuting());
    }
}
