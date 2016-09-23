package network;

import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.SwingUtilities.*;

public class ClientDatagram extends JFrame{
	private JTextField textField;
	private JTextArea area;
	private DatagramSocket socket;

	public ClientDatagram(){
		super("Client side");

		// GUI Component
		textField = new JTextField();
		textField.addActionListener(
			new ActionListener(){
				@Override
				public void actionPerformed(ActionEvent e){
					try {
						String mess = e.getActionCommand();
						display("Sending: " + mess);

						byte[] data = mess.getBytes();
						DatagramPacket pack = new DatagramPacket(data, data.length,
							InetAddress.getLocalHost(), ServerDatagram.PORT);
						socket.send(pack);

						area.append("Packet sent: " + mess + "\n");
						area.setCaretPosition(area.getText().length());

					} catch (IOException io){
						io.printStackTrace();
					} finally {
						textField.setText("");
					}
				}
			}
		);

		add(textField, BorderLayout.NORTH);
		area = new JTextArea();
		add(new JScrollPane(area), BorderLayout.CENTER);

		// final GUI initialization
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(400, 300);
		setVisible(true);
		Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation(dim.width - 450, 0);

		// network operation
		try {
			socket = new DatagramSocket();
		} catch (IOException io){
			display(io + " ERROR");
			io.printStackTrace();
			System.exit(1);
		}

		waitForPacket();
	}

	public void waitForPacket(){
		while(true){
			try {
				byte[] data = new byte[1000];
				DatagramPacket packet = new DatagramPacket(data, data.length);
				socket.receive(packet);

				display("Packet received: \n" +
					"From host: " + packet.getAddress() +
					"\nFromt port: " + packet.getPort() +
					"\nLength: " + packet.getLength() +
					"\nMessage: " + new String(packet.getData(), 0, packet.getLength())
					+"\n"
				);


			}
			catch (IOException io){
				display(io + " ERROR");
				io.printStackTrace();
			}
		}
	}

	public void display(String mess){
		SwingUtilities.invokeLater(
			new Runnable(){
				@Override
				public void run(){
					ClientDatagram.this.area.append(mess + "\n");
				}
			}
		);
	}
}
