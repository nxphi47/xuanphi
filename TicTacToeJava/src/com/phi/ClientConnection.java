package com.phi;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Arrays;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Created by nxphi on 9/13/16.
 *
 *  * ouput from Client:
 * player,request,role  -  to request the role at start - server,role,x or o
 * player,request,nothing - send nothing - server,nothing
 * player,x or o,game,start - to start the game from o, send directly to x
 * player,x or o,turn,1,2 - x or o move 1,2, send directly to o or x
 *
 * INPUT from server or client
 * server,role,x or o
 * server,nothing
 * player,x or o,game,start
 * player,x or o,turn,1,2
 */

public class ClientConnection implements Runnable {
	private Socket socket;
	private ObjectInputStream input;
	private ObjectOutputStream output;
	private LinkedBlockingQueue<String> queueMessage;

	// interface to the GUI
	private ClientTTT gui;
	private int userRole = 0;

	public ClientConnection(ClientTTT gui) throws InterruptedException, IOException {
		this.gui = gui;
		queueMessage = new LinkedBlockingQueue<>();
		addMessage("player,request,role"); // asking for role right away

	}

	@Override
	public void run() {
		try {
			runClient();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	public synchronized void runClient() throws IOException, InterruptedException {
		while (true){
			// connect the server
			socket = new Socket(ServerLoop.IP_ADDRESS, ServerLoop.portNumber);
			// get the streams
			output = new ObjectOutputStream(socket.getOutputStream());
			output.flush();
			input = new ObjectInputStream(socket.getInputStream());
			if (output == null){
				System.out.printf("output null\n");
				continue;
			}
			// request
			request();
			// receive and analyze
			analyzeInput();

			// close
			socket.close();
		}
	}

	public synchronized void analyzeInput() throws IOException {
		do {
			try {
				String message = (String) input.readObject();
				String[] parts = message.split(",");
				if (Main.DEBUG){
					System.out.printf("\nClient receive: " + message);
					System.out.printf("\nArray: " + Arrays.toString(parts));

				}

				// analyzing parts
				if (parts[0].equals("server")){
					// server command
					if (parts[1].equals("role")){
						// setting role
						int role = (parts[2].equals("x")?1:-1);
						userRole = role;
						gui.setRole(role);
					}
					else {
						// send nothing
					}
				}
				else if (parts[0].equals("player")){
					// start the game
					if (parts[2].equals("game") && parts[3].equals("start")) {
						gui.enterGame(true);
						// probably from o
						gui.enableRole(true);
					}
					// other player command
					// player,x,turn,1,2
					else if (parts[1].equals("x")){
						if (userRole == 1){
							System.err.println("\nERROR: two x roles");
						}
						else {
							gui.setMatrixPoint(Integer.parseInt(parts[3]), Integer.parseInt(parts[4]), 1);
						}
					}
					else {
						if (userRole == -1){
							System.err.println("\nERROR: two o roles");
						}
						else {
							gui.setMatrixPoint(Integer.parseInt(parts[3]), Integer.parseInt(parts[4]), -1);
						}
					}
				}
				else {
					// error
				}

			} catch (ClassNotFoundException e) {
				e.printStackTrace();
				System.err.println("\nERROR: object message class not found");
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

		} while (input.available() > 0);
	}


	public synchronized void request() throws IOException, InterruptedException {
		String mess = "player," + ((userRole == 1)?"x":"o") + ",request,nothing";
		if (queueMessage.size() > 0){
			mess = queueMessage.take();
		}
		if (Main.DEBUG){
			System.out.printf("\nClient: request " + mess);
		}
		output.writeObject(mess);
		output.flush();
	}

	public void addMessage(String mess) throws IOException, InterruptedException {
		queueMessage.put(mess);
		if (Main.DEBUG){
			System.out.printf("\nSuccessfully add " + mess + " to  queue");
		}
	}

	public void moveTurn(int i, int j) throws InterruptedException {
		String message = "player," + ((userRole == 1)?"x":"o") + ",turn," + i + "," + j;
		if (Main.DEBUG){
			System.out.printf("\nThread: " + Thread.currentThread().getName());
		}
		queueMessage.put(message);
		System.out.printf("\nSuccessfully add " + message + " to  queue");
	}

	// setter and getter --------------------------------------
	public synchronized LinkedBlockingQueue<String> getQueueMessage() {
		return queueMessage;
	}

	public synchronized void setQueueMessage(LinkedBlockingQueue<String> queueMessage) {
		this.queueMessage = queueMessage;
	}
}
