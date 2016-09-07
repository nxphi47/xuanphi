import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Created by phi on 31/05/16.
 */
public class UserWithdrawMode extends Mode {
    private boolean withdrawing;
    private double amount;
    private String inputAmount;
    private String outputInfor;

    public UserWithdrawMode(Account user, JTextArea textArea, JButton[] buttons) {
        super(textArea, buttons);
        setUserAccount(user);
        setAmount(0);
        setInputAmount("");
        setWithdrawing(false);
        outputInfor = String.format("User %s\n", getUserAccount().getName());

    }

    @Override
    public void handlingButton() {
        // handling buttons
        for (JButton button: buttons){
            for (ActionListener al: button.getActionListeners()){
                button.removeActionListener(al);
            }
            button.addActionListener(new ButtonHandler());
        }

    }


    public boolean isWithdrawing() {
        return withdrawing;
    }

    public void setWithdrawing(boolean withdrawing) {
        this.withdrawing = withdrawing;
    }

    public double getAmount() {
        return amount;
    }

    public void setAmount(double amount) {
        this.amount = amount;
    }

    public String getInputAmount() {
        return inputAmount;
    }

    public void setInputAmount(String inputAmount) {
        this.inputAmount = inputAmount;
    }

    class ButtonHandler implements ActionListener {
        @Override
        public void actionPerformed(ActionEvent e) {
            if (e.getActionCommand().equals("OK")) {
                getText().setText(getUserAccount().withdraw(amount) + "\nPress Cancel");
            } else if (e.getActionCommand().equals("Cancel")) {
                setWithdrawing(false);
                getSuperMode().execute();
            } else if (Character.isDigit(e.getActionCommand().charAt(0))) {
                inputAmount += e.getActionCommand().charAt(0);
                getText().setText(outputInfor + inputAmount);
                setAmount(Double.parseDouble(inputAmount));
            } else if (e.getActionCommand().equals("Back")) {
                inputAmount = inputAmount.substring(0, inputAmount.length() - 1);
                getText().setText(outputInfor + inputAmount);
                setAmount(Double.parseDouble(inputAmount));
            }
        }
    }

    public void start_withdraw(){
        setWithdrawing(true);
        outputInfor += "Withdraw ($): ";
        getText().setText(outputInfor);
        handlingButton();
    }

    @Override
    public void execute() {
        start_withdraw();
    }
}
