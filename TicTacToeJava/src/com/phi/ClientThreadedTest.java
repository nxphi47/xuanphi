package com.phi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Created by nxphi on 9/13/16.
 */
public class ClientThreadedTest {
	public static final int PORT = 5556;
	public static final String IP_ADDRESS = "127.0.0.1";
	private Socket sock;
	private ObjectInputStream input;
	private ObjectOutputStream output;
	private String message;
	private int portNumber;
	private int clientPort;

	public ClientThreadedTest() throws IOException, InterruptedException {
		sock = new Socket(InetAddress.getByName(IP_ADDRESS), PORT);
		Thread.sleep(100000);
	}
}
