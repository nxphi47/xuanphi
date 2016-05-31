import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.Arc2D;

/**
 * Created by phi on 31/05/16.
 */
public class UserDepositMode extends Mode {
    private Account userAccount;
    private boolean depositing;
    private boolean deposited;
    private double amount;
    private String inputAmount;
    private String outputInfor; //to output to text area

    public UserDepositMode(Account user, JTextArea textArea, JButton[] buttons) {
        super(textArea, buttons);
        setUserAccount(user);
        setAmount(0);
        setInputAmount("");
        setDeposited(false);
        setDepositing(false);
        outputInfor = String.format("User %s\n", getUserAccount().getName());
        // handler for the buttons
        for (JButton button: buttons){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(new ButtonHandler());
        }
    }

    public Account getUserAccount() {
        return userAccount;
    }

    public void setUserAccount(Account userAccount) {
        if (userAccount != null) {
            userAccount.setAuthorized(false);
        }
        this.userAccount = userAccount;
        assert userAccount != null;
        userAccount.setAuthorized(true);
    }


    public boolean isDepositing() {
        return depositing;
    }

    public void setDepositing(boolean depositing) {
        this.depositing = depositing;
    }

    public boolean isDeposited() {
        return deposited;
    }

    public void setDeposited(boolean deposited) {
        this.deposited = deposited;
    }


    public double getAmount() {
        return amount;
    }

    public void setAmount(double amount) {
        this.amount = amount;
    }

    @Override
    public void execute() {
        start_deposit();
    }

    public String getInputAmount() {
        return inputAmount;
    }

    public void setInputAmount(String inputAmount) {
        this.inputAmount = inputAmount;
    }

    public void start_deposit(){
        setDepositing(true);
        outputInfor += "Deposit($): ";
        getText().setText(outputInfor);
        while (isDepositing());
        setDeposited(true);
    }

    class ButtonHandler implements ActionListener{
        @Override
        public void actionPerformed(ActionEvent e) {
            if (e.getActionCommand().equals("OK")){
                getText().setText(userAccount.deposit(amount) + "\nPress Cancel");
            }
            else if (e.getActionCommand().equals("Cancel")){
                setDepositing(false);
            }
            else if (Character.isDigit(e.getActionCommand().charAt(0))){
                inputAmount += e.getActionCommand().charAt(0);
                getText().setText(outputInfor + inputAmount);
                setAmount(Double.parseDouble(inputAmount));
            }
            else if (e.getActionCommand().equals("Back")){
                inputAmount = inputAmount.substring(0, inputAmount.length() - 1);
                getText().setText(outputInfor + inputAmount);
                setAmount(Double.parseDouble(inputAmount));
            }
        }
    }
}
