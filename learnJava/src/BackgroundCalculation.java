import javax.swing.*;
import java.util.concurrent.ExecutionException;

/**
 * Created by nxphi on 9/5/16.
 */
public class BackgroundCalculation extends SwingWorker<Long, Object>{
	private final int n;
	private final JLabel resultLabel;

	public BackgroundCalculation(int num, JLabel label){
		n = num;
		resultLabel = label;
	}

	@Override
	protected Long doInBackground() throws Exception {
		return fibonacci(n);
	}

	@Override
	protected void done() {
		// to process the GUI component
		try{
			resultLabel.setText(get().toString());
		}
		catch (InterruptedException e){
			System.err.printf("program interrupted\n");
		}
		catch (ExecutionException e){
			System.err.printf("program got execution error\n");
		}

	}

	private long fibonacci(long num) {
		if (num == 0 || num == 1){
			return num;
		}
		else {
			return fibonacci(num - 1) + fibonacci(num - 2);
		}
	}
}
