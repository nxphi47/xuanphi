package com.phi.game.gui;

import com.phi.game.control.GameController;

import javax.swing.*;

/**
 * Created by nxphi on 9/14/16.
 */
public class GameGUIForm extends GameGUI{
	private JPanel mainPanel;
	private JPanel northPanel;
	private JPanel southPanel;
	private JPanel centerPanel;
	private JLabel userName;
	private JTextField userNameTextField;
	private JComboBox gameChooser;
	private JButton connectButton;
	private JButton restartButton;
	private JButton playerListButton;
	private JLabel roleLabel;
	private JLabel statusLabel;
	private JProgressBar progressBar;
	private int x = 0;

	public GameGUIForm(GameController controller, String gameName){
		super(controller, gameName);
		setContentPane(mainPanel);

		new Thread(new Runnable() {
			@Override
			public void run() {
				//int x = 0;
				while (true){
					if (GameGUIForm.this.x == 100){
						GameGUIForm.this.x = 0;
					}
					try {
						Thread.sleep(30);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					SwingUtilities.invokeLater(new Runnable() {
						@Override
						public void run() {
							GameGUIForm.this.progressBar.setValue(GameGUIForm.this.x++);
						}
					});
				}
			}
		}).start();

		finalizeJFrame();
	}






	// setter and getters -----------------------------------------
	public JPanel getCenterPanel() {
		return centerPanel;
	}

	public void setCenterPanel(JPanel centerPanel) {
		this.centerPanel = centerPanel;
	}
}
