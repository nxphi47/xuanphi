package MultiThread;

import javax.swing.JTextArea;
import javax.swing.JLabel;
import javax.swing.JButton;
import javax.swing.SwingWorker;
import java.util.Arrays;
import java.util.Random;
import java.util.List;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;

public class PrimeCalculation extends SwingWorker<Integer, Integer> {
    private final Random generator = new Random();
    private final JTextArea textArea;
    private final JButton getButton;
    private final JButton cancelButton;
    private final JLabel label;
    private final int max;

    public PrimeCalculation(int max, JTextArea area, JLabel statusLabel, JButton getButton, JButton cancelButton){
        this.max = max;
        textArea = area;
        label = statusLabel;
        this.getButton = getButton;
        this.cancelButton = cancelButton;
    }

    @Override
    public Integer doInBackground(){
        int count = 0; // the number of primes found
        for (int i = 2; i <= max; i++) {
            if (isCancelled()) {
                return count;
            }
            else {
                setProgress(100*(i+1)/max);

                /*
                try{
                    Thread.sleep(4);

                }
                catch (InterruptedException e){
                    label.setText("process interrupted.");
                    return count;
                }
                */
                boolean isPrime = true;
                for (int j = 2; j <= Math.sqrt(i); j++) {
                    if (i%j == 0) {
                        // not prime
                        isPrime = false;
                        break;
                    }
                }
                // run all the loop, then it is prime
                if (isPrime) {
                    publish(i);
                    count++;
                }
            }
        }
        return count;
    }

    // to be executed when done
    @Override
    protected void done(){
        getButton.setEnabled(true);
        cancelButton.setEnabled(false);

        int numPrimes = -1;
        try{
            numPrimes = get();
        }
        catch (InterruptedException intE){
            label.setText("interrupted");

            return;
        }
        catch (ExecutionException exE){
            label.setText("Execution error");
            return;
        }
        catch (CancellationException cancelE){
            label.setText("process cancelled");
        }
        label.setText("Found: " + numPrimes + " primes");
    }

    // process, display the list
    @Override
    protected void process(List<Integer> publishedVals){
        for (int i = 0; i < publishedVals.size(); i++) {
            textArea.append(publishedVals.get(i) + "\n");
        }
    }


}
