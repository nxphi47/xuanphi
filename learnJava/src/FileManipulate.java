import java.io.File;
import java.io.FileNotFoundException;
import java.util.Formatter;
import java.util.FormatterClosedException;
import java.util.NoSuchElementException;
import java.util.Scanner;

/**
 * Created by nxphi on 7/16/16.
 */
public class FileManipulate {
	private Formatter out;
	private Scanner in;

	public void openToRead(){
		try {
			in = new Scanner(new File("clients.txt"));
		}
		catch (FileNotFoundException fileNotFound){
			System.err.println("Error Opening the file");
			System.exit(1);
		}
	}

	public void readRecords(){
		AccountRecord record = new AccountRecord();
		System.out.printf("%-10s%-12s%-12s%10s\n", "Account", "First name",
				"Last Name", "balance");
		try {
			while (in.hasNext()){
				record.setAccount(in.nextInt());
				record.setFirst(in.next());
				record.setLast(in.next());
				record.setBal(in.nextDouble());

				System.out.printf("%-10d%-12s%-12s%10.2f\n",
						record.getAccount(),
						record.getFirst(),
						record.getLast(),
						record.getBal());
			}
		}
		catch (NoSuchElementException eleEx){
			System.err.println("Filed impropersly formatted");
			System.exit(1);
		}
		catch (IllegalStateException stateException){
			System.err.println("Error reading file");
			System.exit(1);
		}
	}

	public void closeFileRead(){
		if (in != null){
			in.close();
		}
	}

	public void openToWrite(){
		try {
			out = new Formatter("clients.txt");
		}
		catch (SecurityException ex1){
			System.err.println("No permission for this file");
			System.exit(1);
		}
		catch (FileNotFoundException ex2){
			System.err.println("FIle not found");
			System.exit(1);
		}
	}

	public void addRecords(){
		AccountRecord record = new AccountRecord();
		Scanner input = new Scanner(System.in);
		System.out.printf("Linux: crtl D to end");
		System.out.printf("Enter: acc, first, last, bal: ");

		while (input.hasNext()){
			try {
				record.setAccount(input.nextInt());
				record.setFirst(input.next());
				record.setLast(input.next());
				record.setBal(input.nextDouble());

				if (record.getAccount() > 0) {
					out.format("%d %s %s %.2f\n", record.getAccount(),
							record.getFirst(),
							record.getLast(),
							record.getBal());
				}
				else {
					System.out.printf("Acc must be > 0");
				}
			}
			catch (FormatterClosedException fomartterErr){
				System.err.println("Error writing the file.");
				return;
			}
			catch (NoSuchElementException elemtnEx){
				System.err.println("Invalid input, try again.");
				input.nextLine();
			}
		}
	}

	public void closeFileOpen(){
		if (out != null){
			out.close();
		}
	}
}
