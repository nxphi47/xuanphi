import javax.swing.*;
import java.awt.*;
import java.io.File;

/**
 * Created by nxphi on 7/26/16.
 */
public class FileChoose extends JFrame {
	private JTextArea outputArea;
	private JScrollPane pane;

	public FileChoose(){
		super("Testing");
		outputArea = new JTextArea();
		pane = new JScrollPane(outputArea);
		add(pane, BorderLayout.CENTER);
		setSize(400,400);
		setVisible(true);

		outputArea.setText(analyzePath());
	}

	private File getFileOrDirectory(){
		JFileChooser fileChooser = new JFileChooser();
		fileChooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);

		int result = fileChooser.showOpenDialog(this);
		if (result == JFileChooser.CANCEL_OPTION){
			System.exit(1);
		}

		File name = fileChooser.getSelectedFile();

		if ((name == null) || (name.getName().equals(""))){
			JOptionPane.showMessageDialog(this, "Invalid Name", "Invalid name",
					JOptionPane.ERROR_MESSAGE);
			System.exit(1);
		}
		return name;
	}

	public String analyzePath(){
		File name = getFileOrDirectory();
		StringBuilder output = new StringBuilder(1000);
		if (name.exists()){
			output.append(String.format("%s%s\n%s\n%s\n%s\n%s\n%s\n%s",
					name.getName(), " exits",
					(name.isFile() ? "is a file": " not a file"),
					(name.isDirectory() ? " is directory": " not a directory"),
					(name.isAbsolute() ? " is absolute": " not absolute"),
					(name.getPath()),
					(name.getAbsolutePath()),
					(name.getParent())
			));

			if (name.isDirectory()) {
				String[] list = name.list();
				/*
				System.out.printf("\nContainer: \n");
				for (String x: list) {
					System.out.printf("%s\n", x);
				}
				*/
				output.append("\nContainer:\n");
				for (String x: list){
					output.append(String.format("%s\n", x));
				}
			}

		}
		else {
			System.out.printf("Not exists");
			return "Error";
		}
		return output.toString();
	}
}
