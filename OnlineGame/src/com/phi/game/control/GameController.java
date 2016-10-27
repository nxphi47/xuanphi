package com.phi.game.control;

import com.phi.game.gui.GameGUI;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.HashMap;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;


/**
 * Created by nxphi on 9/14/16.
 * This Game controller declare methods for communication with the server
 * to be Override by specific controllers of each game
 */
public abstract class GameController implements Runnable{
	private String userName;
	private Socket socket = null;
	private ObjectOutputStream outputStreams = null;
	private ObjectInputStream inputStreams = null;
	private LinkedBlockingQueue<HashMap> queueMessage;
	private GameGUI gui;

	public GameController() {

	}

	@Override
	public void run() {

	}

	// communication functions -------------------------------------------

	// operational functions - to be Override-----------------------------

	// setter and getter -------------------------------------------------
	public String getUserName() {
		return userName;
	}

	public void setUserName(String userName) {
		this.userName = userName;
	}

	public Socket getSocket() {
		return socket;
	}

	public void setSocket(Socket socket) {
		this.socket = socket;
	}

	public ObjectOutputStream getOutputStreams() {
		return outputStreams;
	}

	public void setOutputStreams(ObjectOutputStream outputStreams) {
		this.outputStreams = outputStreams;
	}

	public ObjectInputStream getInputStreams() {
		return inputStreams;
	}

	public void setInputStreams(ObjectInputStream inputStreams) {
		this.inputStreams = inputStreams;
	}

	public LinkedBlockingQueue<HashMap> getQueueMessage() {
		return queueMessage;
	}

	public void setQueueMessage(LinkedBlockingQueue<HashMap> queueMessage) {
		this.queueMessage = queueMessage;
	}

	public GameGUI getGui() {
		return gui;
	}

	public void setGui(GameGUI gui) {
		this.gui = gui;
	}
}
