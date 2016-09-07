import com.sun.jmx.snmp.SnmpStringFixed;

import java.io.Serializable;

/**
 * Created by nxphi on 7/24/16.
 */
public class AccountRecordSerial implements Serializable {
	private int acc;
	private String first;
	private String last;
	private double bal;

	public AccountRecordSerial(){
		this(0, "", "", 0.0);
	}

	public AccountRecordSerial(int acc, String first, String last, double bal){
		setAcc(acc);
		setFirst(first);
		setLast(last);
		setBal(bal);
	}



	public int getAcc() {
		return acc;
	}

	public void setAcc(int acc) {
		this.acc = acc;
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
