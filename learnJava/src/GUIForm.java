import javax.swing.*;

/**
 * Created by nxphi on 8/28/16.
 */
public class GUIForm extends JFrame{
	private JPanel mainPanel;
	private JPanel northPanel;
	private JPanel centerPanel;
	private JPanel southPanel;
	private JTextField upperTextField;
	private JLabel usernameLabel;
	private JTextField usernameTextField;
	private JLabel passwordLabel;
	private JPasswordField passwordField1;
	private JComboBox domainComboBox;
	private JButton connectButton;
	private JTabbedPane userInfoTabPane;
	private JPanel personalInfoTab;
	private JPanel accountSetInfoTab;
	private JLabel fullnameLabel;
	private JTextField fullnameTextField;
	private JLabel genderLabel;
	private JRadioButton maleRadioButton;
	private JRadioButton femaleRadioButton;
	private JLabel birthDateLabel;
	private JTextField textField1;
	private JTextField educationtextField;
	private JLabel educationLabel;
	private JComboBox degreeComboBox;
	private JLabel nationalLabel;
	private JComboBox nationalityComboBox;
	private JComboBox raceComboBox;
	private JLabel emailLabel;
	private JTextField emailTextField;
	private JButton updateButton;
	private JButton clearButton;
	private JProgressBar progressBar;

	public GUIForm(){
		super("User application");
		setContentPane(mainPanel);
		pack();
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

		setVisible(true);
	}
}
