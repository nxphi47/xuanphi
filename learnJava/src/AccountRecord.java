/**
 * Created by nxphi on 7/16/16.
 */
public class AccountRecord {
	private int account;
	private String first;
	private String last;
	private double bal;

	public AccountRecord(int acc, String first, String last, double bal){
		setAccount(acc);
		setLast(last);
		setFirst(first);
		setBal(bal);
	}

	public AccountRecord(){
		this(0, "", "", 0.0);
	}

	public int getAccount() {
		return account;
	}

	public void setAccount(int account) {
		this.account = account;
	}

	public String getFirst() {
		return first;
	}

	public void setFirst(String first) {
		this.first = first;
	}

	public String getLast() {
		return last;
	}

	public void setLast(String last) {
		this.last = last;
	}

	public double getBal() {
		return bal;
	}

	public void setBal(double bal) {
		this.bal = bal;
	}
}
