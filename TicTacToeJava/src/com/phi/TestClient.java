package com.phi;

import com.sun.corba.se.spi.activation.LocatorPackage.ServerLocation;
import com.sun.org.apache.bcel.internal.generic.IF_ACMPEQ;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;

/**
 * Created by nxphi on 9/13/16.
 */


public class TestClient {
	public static final int PORT = 5555;
	public static final String IP_ADDRESS = "127.0.0.1";
	private Socket sock;
	private ObjectInputStream input;
	private ObjectOutputStream output;
	private String message;
	private int portNumber;
	private int clientPort;


	public TestClient() throws IOException, ClassNotFoundException {


		Scanner scanner = new Scanner(System.in);

		//System.out.printf("\nEnter line: ");
		while (true){
			// connect
			sock = new Socket(InetAddress.getByName(IP_ADDRESS), ServerLoop.portNumber);
			// get streams
			output = new ObjectOutputStream(sock.getOutputStream());
			output.flush();
			input = new ObjectInputStream(sock.getInputStream());
			if (output == null){
				System.out.printf("output null\n");
				continue;
			}

			// request
			System.out.printf("\nEnter line: ");
			String messs = scanner.nextLine();
			output.writeObject(messs);
			output.flush();
			System.out.printf("\nClient send mess");
			// receive
			String inputMess = (String) input.readObject();
			System.out.printf("\nRecevied: " + inputMess);
			// analyze

			// close, let the server close
			sock.close();

		}
	}
}
