package com.phi;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * Created by nxphi on 9/13/16.
 * main server operated by looping create and terminate connection
 * <p>
 * INPUT from Client:
 * player,request,role  -  to request the role at start - server,role,x or o
 * player,request,nothing - send nothing - server,nothing
 * player,x or o,request,nothing
 * player,x or o,game,start - to start the game from o, send directly to x
 * player,x or o,turn,1,2 - x or o move 1,2, send directly to o or x
 */
public class ServerLoop {
	private ServerSocket serverSocket;
	private Socket connection;
	private ObjectInputStream input;
	private ObjectOutputStream output;

	public static final int portNumber = 5555;
	public static final String IP_ADDRESS = "127.0.0.1";

	// operational parameter
	private ArrayBlockingQueue<String> queueX;
	private ArrayBlockingQueue<String> queueO;
	private ArrayBlockingQueue<String> queueSendBack;
	private boolean roleXOccupied = false;
	private int currentRole = 0; // 1 = x, 0 = o
	private static final String nothing = "server,nothing";

	public ServerLoop() throws IOException, InterruptedException {

		queueX = new ArrayBlockingQueue<String>(100);
		queueO = new ArrayBlockingQueue<String>(100);
		queueSendBack = new ArrayBlockingQueue<String>(1);

		// runServer
		runServer();
	}

	// --------- communication functions- ------------------
	public void runServer() throws IOException, InterruptedException {
		serverSocket = new ServerSocket(portNumber, 1);


		while (true) {

			try {
				Thread.sleep(10);
				// accept connection
				connection = serverSocket.accept();
				//connection.setSoTimeout(1000); // timeout 1sec
				//System.out.printf("\nServer: connection Established ");
				// get Streams
				getStreams();
				// analyze inputs[]
				analyzeInput();
				// send back []
				sendBack();
				// close connection
				closeConnection();
			} catch (InterruptedException | IOException e) {
				System.err.println("\nERROR: connection corrupt\n");
				// reset the game
				roleXOccupied = false;
				currentRole = 0;
				queueX.clear();
				queueO.clear();
				continue;
				//e.printStackTrace();
			}
		}

	}

	public synchronized void getStreams() throws IOException {
		output = new ObjectOutputStream(connection.getOutputStream());
		output.flush();

		input = new ObjectInputStream(connection.getInputStream());
		if (output != null) {
			//System.out.printf("\nServer: got input outut");
		} else {
			System.out.printf("\nServer: ERROR output null");
		}
	}

	public synchronized void analyzeInput() throws IOException, InterruptedException, SocketException {
		do {
			try {
				String message = (String) input.readObject();
				String[] parts = message.split(",");
				if (Main.DEBUG && !message.contains("nothing")) {
					System.out.printf("\nServer receive: " + message);
					System.out.printf("\nArray: " + Arrays.toString(parts));
				}

				if (parts[1].equals("request")) {
					// request role or nothing
					if (parts[2].equals("role")) {
						if (roleXOccupied) {
							// assign role O
							currentRole = -1;
							queueO.put("server,role,o");
						} else {
							// assign role X
							currentRole = 1;
							queueX.put("server,role,x");
							roleXOccupied = true;
						}
					} else {
						// something need to be return server,nothing
						/*
						if (currentRole == 1){
							queueX.put(nothing);
						}
						else {
							queueO.put(nothing);
						}
						*/
					}
				} else if (parts[1].equals("x")) {
					// x is doing
					if (!message.contains("nothing")) {
						queueO.put(message);
					}
					// need to put something to queueX
					//queueX.put(nothing);
					currentRole = 1;
					if (Main.DEBUG && !message.contains("nothing")) {
						System.out.printf("\nSever: X in access: " + message + " ");
					}
				} else if (parts[1].equals("o")) {
					if (!message.contains("nothing")) {
						queueX.put(message);
					}
					//queueO.put(nothing);
					currentRole = -1;
					if (Main.DEBUG && !message.contains("nothing")) {
						System.out.printf("\nSever: O in access: " + message + " ");
					}
				} else {
					System.err.println("\nERROR: analyze mess: " + message);
				}

			} catch (ClassNotFoundException e) {
				System.err.println("\nERROR: input class not found");
				e.printStackTrace();
			}
		} while (input.available() > 0);
	}

	public synchronized void sendBack() throws InterruptedException, IOException {
		if (currentRole == 1) {
			if (queueX.size() > 0) {
				sendToClient(queueX.take());
			} else {
				sendToClient(nothing);
			}
			return;
		} else if (currentRole == -1) {
			if (queueO.size() > 0) {
				sendToClient(queueO.take());
			} else {
				sendToClient(nothing);
			}
			return;
		} else {
			System.err.println("\nERROR: sendBack: role undefined before sendback ");
		}
		sendToClient("server,nothing");

	}

	public synchronized void sendToClient(String message) throws IOException {
		output.writeObject(message);
		output.flush();
		if (Main.DEBUG && !message.contains("nothing")) {
			System.out.printf("\nServer: write to " + message + " to client ");
		}
	}

	public synchronized void closeConnection() throws IOException {
		output.close();
		input.close();
		connection.close();
		//System.out.printf("\n\n");
		//System.out.printf("\nServer CLose the connection ");
	}


	// get setter --------------------
	public ArrayBlockingQueue<String> getQueueX() {
		return queueX;
	}

	public void setQueueX(ArrayBlockingQueue<String> queueX) {
		this.queueX = queueX;
	}

	public ArrayBlockingQueue<String> getQueueO() {
		return queueO;
	}

	public void setQueueO(ArrayBlockingQueue<String> queueO) {
		this.queueO = queueO;
	}
}
