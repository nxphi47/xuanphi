import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by phi on 31/05/16.
 */
public class UserMode extends Mode {
    private String[] noti;
    private int option;


    // constructor
    public UserMode(Account user, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        // user mode in specific
    }

    @Override
    public void handlingButton() {
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
                UserShowBalanceMode showBalanceMode = new UserShowBalanceMode(getUserAccount(), getText(), getButtons());
                showBalanceMode.setSuperMode(this);
                showBalanceMode.execute();
                //showBalanceMode.execute();
                break;
            case 2:
                UserDepositMode depositMode = new UserDepositMode(getUserAccount(), getText(), getButtons());
                depositMode.setSuperMode(this);
                depositMode.execute();
                break;
            case 3:
                UserWithdrawMode withdrawMode = new UserWithdrawMode(getUserAccount(), getText(), getButtons());
                withdrawMode.setSuperMode(this);
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
                getUserAccount().setAuthorized(false);
                getText().setText(getNoti());

                // update the database in lockMode
                LockMode lockMode = (LockMode) UserMode.this.getSuperMode();
                lockMode.getDataBase().outputFile();
                getSuperMode().execute();
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
        // set the notification
        if (getUserAccount() == null){
            System.err.printf("%s: Account not specified before execution", this.getClass());
            System.exit(1);
        }
        noti =new String[4];
        noti[0] = String.format("User: %s", getUserAccount().getName());
        noti[1] = "Show balance? *";
        noti[2] = "Deposit? ";
        noti[3] = "Withdraw? ";

        option = 1;//show balance
        // option pane
        getText().setText(getNoti());
        handlingButton();
    }
}
