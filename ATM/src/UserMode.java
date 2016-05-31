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

    public void setUserAccount(Account user) {
        if (getUserAccount() != null){
            getUserAccount().setAuthorized(false);
        }
        if (user == null){
            System.err.println("Show balance: input useraccount = null");
            System.exit(1);
        }
        this.userAccount = user;
        getUserAccount().setAuthorized(true);
    }

    // constructor
    public UserMode(Account user, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        // user mode in specific
        setUserAccount(user);
        // set the notification
        noti =new String[4];
        noti[0] = String.format("User: %s", getUserAccount().getName());
        noti[1] = "Show balance? *";
        noti[2] = "Deposit? ";
        noti[3] = "Withdraw? ";

        option = 1;//show balance
        getText().setText(getNoti());


        //handling the button
        for (JButton button: buttons){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(new ButtonHandler());
        }
    }

    public void setOption(int choice){
        for (int i = 0; i < noti.length; i++){
            if (noti[i].charAt(noti[i].length() - 1) == '*'){
                noti[i] = noti[i].substring(0, noti[i].length() - 1);
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
                showBalanceMode = new UserShowBalanceMode(getUserAccount(), getText(), getButtons());
                showBalanceMode.execute();
                break;
            case 2:
                depositMode = new UserDepositMode(getUserAccount(), getText(), getButtons());
                depositMode.execute();
                break;
            case 3:
                withdrawMode = new UserWithdrawMode(getUserAccount(), getText(), getButtons());
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
                getText().setText(getNoti());
            }
            else if (e.getActionCommand().equals("OK")){
                optionExecute();
            }
            else if (e.getActionCommand().equals("^")){
                // up
                int choice = option;
                choice -= 1;
                if (choice <= 0) choice+=3;
                setOption(choice);
                getText().setText(getNoti());
            }
            else if (e.getActionCommand().equals("v")){
                int choice = option;
                choice += 1;
                if (choice > 3) choice-=3;
                setOption(choice);
                getText().setText(getNoti());
            }
        }
    }

    @Override
    public void execute() {
        // option pane
        setEnabled(true);
        setExecuting(true);
        while (isExecuting());
        setEnabled(false);
    }
}
