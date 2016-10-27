package network;

// library
import java.io.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.SwingUtilities;

// class Server Datagram

public class ServerDatagram extends JFrame{
	private JTextArea area;
	private DatagramSocket socket;
	public static final int PORT = 5000;

	public ServerDatagram(){
		super("Server Datagram");

		area = new JTextArea();
		add(new JScrollPane(area), BorderLayout.CENTER);

		// final initialization GUI
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(400,300);
		setVisible(true);

		// connection to socket
		try {
			socket = new DatagramSocket(PORT);
			display("Connected to socket port: " + String.valueOf(PORT));
		}
		catch (SocketException sockEx){
			sockEx.printStackTrace();
			System.exit(1);
		}

		// waiting for packet
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
				);

				sendPacketToClient(packet);
			} catch (IOException io){
				display(io + " error!");
				io.printStackTrace();
			}
		}
	}


	public void sendPacketToClient(DatagramPacket packet) throws IOException{
		display("\nEcho data to client...");

		DatagramPacket sendPacket = new DatagramPacket(packet.getData(), packet.getLength(),
			packet.getAddress(), packet.getPort());

		socket.send(sendPacket);

		display("Packet sent.");
	}

	public void display(String mess){
		SwingUtilities.invokeLater(
			new Runnable(){
				@Override
				public void run(){
					ServerDatagram.this.area.append(mess + "\n");
				}
			}
		);
	}

}
