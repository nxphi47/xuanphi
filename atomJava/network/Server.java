package network;

import java.util.*;
import java.net.*;
import java.io.*;
import java.applet.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class Server extends JFrame{
	private JTextField text;
	private JTextArea area;
	private ObjectOutputStream output;
	private ObjectInputStream input;
	private ServerSocket server;
	private Socket connection;

	private int counter = 0;

	public Server(){
		super("Server side");

		// create and add component
		text = new JTextField();
		text.setEditable(false);
		area = new JTextArea(30, 20);
		area.setEditable(false);
		add(text, BorderLayout.NORTH);
		add(new JScrollPane(area), BorderLayout.CENTER);

		// setup component
		text.addActionListener(
			new ActionListener(){
				@Override
				public void actionPerformed(ActionEvent e){
					sendData(e.getActionCommand());
					text.setText("");
				}
			}
		);

		// final operation for JFrame
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation(0,350);
		setSize(500,300);
		setVisible(true);
	}

	public void runServer(){
		try{
			server = new ServerSocket(5555, 100); // port 5555, max 100 clients
			while(true){
				try{
					//wait for connection
					waitForConnection();
					// get input and output stream
					getStreams();
					// process information
					process();
				}
				catch (EOFException eof){
					// display end of file disconnection
					displayMessage("\nServer - client disconnected\n\n");
				}
				finally{
					// disconnection
					closeConnection();
					counter++;
				}
			}
		}
		catch(IOException io){
			io.printStackTrace();
		}
	}

	// waiting for connection
	public void waitForConnection() throws IOException {
		displayMessage("Waiting for connection\n");
		connection = server.accept(); // Waiting
		System.out.printf("\nConnected to client\n");
		displayMessage(String.format("Connection %d from %s\n", counter,
			connection.getInetAddress().getHostName()));

	}

	public void getStreams() throws IOException {
		output = new ObjectOutputStream(connection.getOutputStream());
		output.flush(); // flush the output to get it ready to send new data

		input = new ObjectInputStream(connection.getInputStream());
		if (output == null || input == null) {
			System.out.printf("\noutput or input null\n");
		}
		displayMessage("\nGot streams\n");
	}

	public void process() throws IOException {
		String  mess = "Connection Successful!\n";
		sendData(mess);

		setTextFieldEditable(true); // let the server user edit to send emss
		do{
			try {
				mess = (String) input.readObject();
				displayMessage("\n" + mess + "\n");
			}
			catch (ClassNotFoundException classEx){
				displayMessage("\nUnknown object type!\n");
			}
			System.out.printf("Input_mess = " + mess);
		} while(!mess.equals("CLIENT>>>TERMINATE"));
	}

	public void sendData(String content){
		try{
			output.writeObject("SERVER>>>" + content);
			output.flush();
			displayMessage("\nSERVER>>>" + content);
		}
		catch (IOException io){
			area.append("Error to write to output\n");
		}
	}

	public void closeConnection(){
		displayMessage("\nDisconnecting connection\n");
		setTextFieldEditable(false);
		try {
			output.close();
			input.close();
			connection.close();
		}
		catch (NullPointerException nullptr){
			System.err.printf("Out, in or connected null\n");
		}
		catch (IOException io){
			io.printStackTrace();
		}
	}

	// manipulate GUI component in the event-dispatched thread
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
