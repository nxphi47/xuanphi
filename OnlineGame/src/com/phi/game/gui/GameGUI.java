package com.phi.game.gui;

import com.phi.game.control.GameController;

import javax.swing.*;

/**
 * Created by nxphi on 9/14/16.
 */
public abstract class GameGUI extends JFrame {
	private String userName;
	private String gameName;
	private GameController controller;


	public GameGUI(GameController controller, String gameName){
		super();

		setGameName(gameName);
		setController(controller);



		// finalize the JFrame
		// inalizeJFrame();
	}

	public void finalizeJFrame(){
		setTitle(gameName);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		pack();
		setResizable(false);
		setVisible(true);
	}





	public String getUserName() {
		return userName;
	}

	public void setUserName(String userName) {
		this.userName = userName;
	}

	public GameController getController() {
		return controller;
	}

	public void setController(GameController controller) {
		this.controller = controller;
	}

	public String getGameName() {
		return gameName;
	}

	public void setGameName(String gameName) {
		this.gameName = gameName;
	}
}
