import java.io.*;
import java.util.NoSuchElementException;
import java.util.Scanner;

/**
 * Created by nxphi on 7/24/16.
 */
public class FileSequential {
	private ObjectOutputStream outputStream;
	private ObjectInputStream inputStream;

	public void openWrite(){
		try {
			outputStream = new ObjectOutputStream(new FileOutputStream("clients.ser"));
		} catch (FileNotFoundException e) {
			System.err.println("File not found\n");
		} catch (IOException e) {
			System.err.println("File cannot be opened\n");
		}
	}
	public void openRead(){
		try {
			inputStream = new ObjectInputStream(new FileInputStream("clients.ser"));
		} catch (IOException e) {
			System.err.println("Error opening read file.");
		}
	}

	public void readRecords(){
		AccountRecordSerial record;
		System.out.printf("%-10s%-12s%-12s%10s\n",
				"Account", "First name", "Last Name", "Balance");
		try {
			while (true){
				record = (AccountRecordSerial) inputStream.readObject();
				System.out.printf("%-10d%-12s%-12s%10.2f\n",
						record.getAcc(), record.getFirst(), record.getLast(),
						record.getBal());
			}
		}
		catch (EOFException eof){
			return;
		}
		catch (ClassNotFoundException cls){
			System.err.println("Class error");
		}
		catch (IOException io){
			System.err.println("IO readiing file error");
		}
	}

	public void addRecords(){
		AccountRecordSerial record;
		int acc = 0;
		String first;
		String last;
		double bal;

		Scanner input = new Scanner(System.in);

		System.out.printf("Ctrl d to end\nEneter acc, fist, last, bal");

		while (input.hasNext()){
			try {
				acc = input.nextInt();
				first = input.next();
				last = input.next();
				bal = input.nextDouble();

				if (acc > 0) {
					record = new AccountRecordSerial(acc, first, last, bal);
					outputStream.writeObject(record);
				} else {
					System.out.printf("Acc must be > 0\n");
				}
			}
			catch (IOException io){
				System.err.println("Error writing to file");
				return;
			}
			catch (NoSuchElementException ele){
				System.err.println("Invalid input, try again");
				input.nextLine(); // this is to discard to input to try again
			}

		}
	}

	public void closeFileWrite(){
		try {
			if (outputStream != null){
				outputStream.close();
			}
		}
		catch (IOException io){
			System.err.println("Error closing file.");
			System.exit(1);
		}
	}

	public void closeFileRead(){
		try {
			if (inputStream != null){
				inputStream.close();
			}
		}
		catch (IOException io){
			System.err.println("Error closing the file.");
			System.exit(1);
		}
	}

}
