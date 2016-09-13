package network;

import java.util.*;
import java.net.*;
import java.io.*;
import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class Client extends JFrame{
	private JTextField text;
	private JTextArea area;
	private ObjectOutputStream output;
	private ObjectInputStream input;
	private Socket sock;

	private String mess;
	private String chatServer;

	public Client(String host){
		super("Client application");

		chatServer = host;

		// initialize components
		text = new JTextField();
		text.setEditable(false);
		text.addActionListener(
			new ActionListener(){
				@Override
				public void actionPerformed(ActionEvent e){
					// sendData
					sendData(e.getActionCommand());
					text.setText("");
				}
			}
		);
		area = new JTextArea();

		// setup component
		add(text, BorderLayout.NORTH);
		add(new JScrollPane(area), BorderLayout.CENTER);

		// final setting for JFrame
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation(0,0);
		setSize(500,300);
		setVisible(true);
	}

	// run the client
	public void runClient(){
		try{
			// connect to server
			connectToServer();
			// get the streams
			getStreams();
			// process information
			processInfo();
		}
		catch (IOException io){
			io.printStackTrace();
		}
		finally{
			closeConnection();
			// disconnection
		}
	}

	// --------- operation function ---------
	public void connectToServer() throws IOException {
		displayMessage("\nAttempting to connect to server\n");
		sock = new Socket(InetAddress.getByName(chatServer), 5555);
		// server address and port number
		displayMessage("\nConnected to " + sock.getInetAddress().getHostName() + "\n");

	}

	public void getStreams() throws IOException {
		output = new ObjectOutputStream(sock.getOutputStream());
		output.flush();

		input = new ObjectInputStream(sock.getInputStream());

		displayMessage("\nGot the streams\n");
	}

	public void processInfo() throws IOException{
		setTextFieldEditable(true);
		try{
			do{
				try {
					mess = (String) input.readObject();
					displayMessage("\n" + mess);
				}
				catch (ClassNotFoundException classEx){
					displayMessage("\nInput class not found!!!\n");
				}

			} while(!mess.equals("SERVER>>>TERMINATE"));
		}
		catch (IOException io){
			io.printStackTrace();
		}
	}

	public void sendData(String mess){
		try {
			output.writeObject("CLIENT>>>" + mess);
			output.flush();
			displayMessage("\nCLIENT>>>" + mess);
		}
		catch (IOException io){
			displayMessage("\nSending data failed\n");
		}
	}

	public void closeConnection(){
		setTextFieldEditable(false);
		displayMessage("\nDisconnect from client\n");
		try {
			output.close();
			input.close();
			sock.close();
		}
		catch (IOException io){
			io.printStackTrace();
		}
	}


	// update GUi from  event dispatch thread
	public void displayMessage(String str){
		SwingUtilities.invokeLater(
			new Runnable(){
				@Override
				public void run(){
					area.append(str);
				}
			}
		);
	}

	public void setTextFieldEditable(boolean edit){
		SwingUtilities.invokeLater(
			new Runnable(){
				@Override
				public void run(){
					text.setEditable(edit);
				}
			}
		);
	}

}
