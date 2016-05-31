import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by nxphi on 5/31/16.
 */
public class LockMode extends Mode implements Notification{
    private String accNumber;
    private String pass;
    private DataBase dataBase;
    private boolean unlocked;
    private boolean foundAcc;
    private Account account;

    public String getAccNumber() {
        return accNumber;
    }

    public void setAccNumber(String accNumber) {
        this.accNumber = accNumber;
    }

    public String getPass() {
        return pass;
    }

    public void setPass(String pass) {
        this.pass = pass;
    }

    public DataBase getDataBase() {
        return dataBase;
    }

    public void setDataBase(DataBase dataBase) {
        this.dataBase = dataBase;
    }

    public boolean isUnlocked() {
        return unlocked;
    }

    public void setUnlocked(boolean unlocked) {
        this.unlocked = unlocked;
    }

    public LockMode(DataBase dataBase, JTextArea textArea, JButton[] buttons){
        super(textArea, buttons);
        setDataBase(dataBase);
        setFoundAcc(false);
        setUnlocked(false);
        setEnabled(false);
        setAccNumber("");
        setPass("");

        // handling the button
        for (JButton button :buttons) {
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(new ButtonHandler());
        }
    }

    public boolean isFoundAcc() {
        return foundAcc;
    }

    public void setFoundAcc(boolean foundAcc) {
        this.foundAcc = foundAcc;
    }

    @Override
    public void execute() {
        setEnabled(true);
        // printing to the text area
        getText().setText(getNoti(""));
        while (!isFoundAcc()){
            System.out.printf("Looking for acc\n");
        }
        while (!isUnlocked() && isFoundAcc()){
            System.out.printf("Password???\n");
        }
        setEnabled(false);
    }


    @Override
    public String getNoti(String content) {
        String noti = "Wellcome to phi ATM\n";
        if (!isFoundAcc()){
            noti += "Enter Account No.: " + accNumber;
        }
        else if (!isUnlocked() && isFoundAcc()){
            noti += String.format("User: %s\n", account.getName()) + "Enter Password: " + pass;
        }
        else {
            noti += "Unlocked but not respones(ERROR)";
        }
        noti += "\n" + content;
        return noti;
    }

    public Account getAccount() {
        return account;
    }

    public void setAccount(Account account) {
        this.account = account;
    }

    class ButtonHandler implements ActionListener{
        @Override
        public void actionPerformed(ActionEvent e) {
            if (!isFoundAcc()){
                // not found the account, keep looking for number
                if (Character.isDigit(e.getActionCommand().charAt(0))){
                    // it is number, number only < 6 digits
                    if (accNumber.length() < 6){
                        accNumber += e.getActionCommand().charAt(0);
                        getText().setText(getNoti(""));
                    }
                }
                else if (e.getActionCommand().equals("Back")){
                    if (accNumber.length() > 0){
                        accNumber = accNumber.substring(0, accNumber.length() - 1);
                        getText().setText(getNoti(""));
                    }
                }
                else if (e.getActionCommand().equals("OK")){
                    // confirm
                    account = dataBase.getAccount(Integer.parseInt(accNumber));
                    if (account == null){
                        // not found in the system
                        getText().setText(getNoti("Number incorrect"));
                    }
                    else {
                        setFoundAcc(true);
                        getText().setText(getNoti(""));
                    }
                }
            }
            else if (!isUnlocked() && isFoundAcc()){
                if (Character.isDigit(e.getActionCommand().charAt(0))){
                    // it is number, number only < 6 digits
                    if (pass.length() <= 6){
                        pass += e.getActionCommand().charAt(0);
                        getText().setText(getNoti(""));
                    }
                }
                else if (e.getActionCommand().equals("Back")){
                    if (pass.length() > 0){
                        pass = pass.substring(0, pass.length() - 1);
                        getText().setText(getNoti(""));
                    }
                }
                else if (e.getActionCommand().equals("OK")){
                    // confirm pass
                    if (account.getPassword().equals(pass)){
                        // access to acocunt
                        setUnlocked(true);
                        account.setAuthorized(true);
                    }
                    else {
                        // wrong password
                        getText().setText(getNoti("Password incorrect"));
                    }
                }
            }
        }
    }
}
