import javax.swing.*;
import java.awt.event.*;

/**
 * Created by nxphi on 8/28/16.
 * in order to practice create GUI panel using form
 */
public class SampleGUIFORM extends JFrame {
	private JButton startButton;
	private JTextArea helloWorldEditMeTextArea;
	private JTextField userNameTextField;
	private JLabel iconLabel;
	private JPanel mainPanel;
	private JPasswordField passWordTextField;
	private JProgressBar progressBar1;

	private void createUIComponents() {
		// TODO: place custom component creation code here
	}

	public SampleGUIFORM(){
		super("Hello world");
		setContentPane(mainPanel);
		pack();
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		startButton.addActionListener(new ActionListener(){
			@Override
			public void actionPerformed(ActionEvent e){
				startButton.setText("Wait for progress");

				class ProgressThread extends Thread{
					@Override
					public void run() {
						startProgress();
					}
				}
				ProgressThread newThread = new ProgressThread();
				newThread.start();

			}
		});

		setVisible(true);
	}

	public void startProgress(){
		while (progressBar1.getValue() < 100){
			try {
				Thread.sleep(100);
				progressBar1.setValue(progressBar1.getValue() + 1);
				progressBar1.setString(String.format("Progress %d%%", progressBar1.getValue()));
			} catch (InterruptedException e) {
				System.err.println("program interrupted\n");
				e.printStackTrace();
			}
		}
		progressBar1.setValue(0);
		progressBar1.setString(String.format("Progress %d%%", progressBar1.getValue()));
		startButton.setText("Click here to start");
	}
}
