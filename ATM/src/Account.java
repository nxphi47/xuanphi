import java.util.NoSuchElementException;

/**
 * Created by phi on 31/05/16.
 */
public class Account implements Notification{
    private String name;
    private int number;
    private double balance;
    private String password;
    private boolean authorized;


    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public double getBalance() {
        return balance;
    }

    public void setBalance(double balance) {
        this.balance = balance;
    }

    public String deposit(double amount) {
        if (!isAuthorized()){
            System.err.println("No authorized yet");
            System.exit(1);
        }
        if (amount <= 0){
            return getNoti("Amounter entered < 0");
        }
        else {
            this.balance += amount;
            return getNoti("Deposit successful");
        }
    }

    public String withdraw(double amount){
        if (!isAuthorized()){
            System.err.println("No authorized yet");
            System.exit(1);
        }
        if (amount > this.balance){
            return getNoti("Amount greater than balance");
        }
        else {
            this.balance -= amount;
            return getNoti("Widthdraw successful, take your cash");
        }
    }

    public String showBalance(){
        if (!isAuthorized()){
            System.err.println("No authorized yet");
            System.exit(1);
        }
        return getNoti(String.format("Balance: $%.2f", getBalance()));
    }

    public Account(int num, String name, String pass, double bal){
        setName(name);
        setBalance(bal);
        setPassword(pass);
        setNumber(num);
        this.authorized = false;
    }

    public Account(int num, String name, String pass){
        this(num, name, pass, 0);
    }

    @Override
    public String getNoti(String content) {
        String hearder = String.format("User: %s\n", getName());
        return hearder + content;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public boolean isAuthorized() {
        return authorized;
    }

    public void setAuthorized(boolean authorized) {
        this.authorized = authorized;
    }

    public boolean enterPassword(String pass){
        if (getPassword().equals(pass)){
            setAuthorized(true);
            return true;
        }
        else {
            setAuthorized(false);
            return false;
        }
    }

    public int getNumber() {
        return number;
    }

    public void setNumber(int number) {
        this.number = number;
    }
}
