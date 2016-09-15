package com.phi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * Created by nxphi on 9/13/16.
 */
public class ServerThreaded {
	private ServerSocket serverSocket;
	private Socket connection;
	private ObjectInputStream input;
	private ObjectOutputStream output;

	public static final int portNumber = 5556;
	public static final String IP_ADDRESS = "127.0.0.1";

	public ServerThreaded() throws IOException {
		serverSocket = new ServerSocket(portNumber, 100);
		runServer();
	}

	public void runServer() throws IOException {
		connection = serverSocket.accept();

		new Thread(new Runnable() {
			@Override
			public void run() {
				try {
					runServer();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}).start();
		System.out.printf("\nserver accepted, new thread started\n");
		//System.out.printf("\n%s", connection.getChannel().getLocalAddress());
		//ystem.out.printf("\n%s", connection.getChannel().getRemoteAddress());
		System.out.printf("\n%s", connection.getLocalAddress());
		System.out.printf("\n%s",connection.getInetAddress().getHostAddress());
		System.out.printf("\n%s", connection.getInetAddress().getHostName());
		System.out.printf("\n%s", connection.getLocalSocketAddress());
		System.out.printf("\n%s", connection.getLocalPort());
		while (connection.isConnected());
	}
}
