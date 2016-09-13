package com.phi;

import javax.swing.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;

/**
 * Created by nxphi on 9/12/16.
 */
public class ClientTTT extends JFrame {
	private JPanel maintPanel;
	private JPanel northPanel;
	private JPanel centerPanel;
	private JPanel southPanel;
	private JLabel userNameLabel;
	private JTextField userNameTextField;
	private JLabel userRoleLabel;
	private JLabel statusLabel;
	private JLabel square00;
	private JLabel square01;
	private JLabel square02;
	private JLabel square10;
	private JLabel square11;
	private JLabel square12;
	private JLabel square20;
	private JLabel square21;
	private JLabel square22;

	// matrixIcon
	private JLabel[][] matrixIcon;
	private int[][] matrix; // 0 = empty, 1 = x, -1 = o

	// iconImage
	private final ImageIcon emptyIcon = new ImageIcon(ClientTTT.this.getClass().getResource("icon/iconImage/EmptyIcon.png"));
	private final ImageIcon xIcon = new ImageIcon(ClientTTT.this.getClass().getResource("icon/iconImage/Xicon.png"));
	private final ImageIcon oIcon = new ImageIcon(ClientTTT.this.getClass().getResource("icon/iconImage/Oicon.png"));
	private final ImageIcon hoveredIcon = new ImageIcon(ClientTTT.this.getClass().getResource("icon/iconImage/HoveredIcon.png"));

	// operational parameter
	private String userName;
	private String rivaryName;
	private boolean inGame = false; // if false, not ready to
	private boolean enabledRole = false; // this player turn
	private int role = 0; // 1 = x, -1 = o, 0 = error
	private int teamOther = 0; // the other role
	private GameControl controller;
	private GameControl.GameStatus status;

	// communication parameter
	private ClientConnection connection;

	public ClientTTT() throws IOException, InterruptedException {
		super("Tic Tac Toe client");
		setContentPane(maintPanel);
		// setup components
		setupMatrix();
		setupMatrixIcon();
		setLabels();

		controller = new GameControl(matrix);
		status = controller.updateResult();
		// setup communication module in a new thread
		connection = new ClientConnection(this);
		new Thread(connection).start();
		// FIXME: start in a new thread


		// final JFrame setting
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		pack();
		setVisible(true);
	}


	// setup functions -----------------------------
	public void setLabels() {
		userRoleLabel.setText("You are ");
		statusLabel.setText("Enter your name");
		userNameTextField.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent actionEvent) {
				ClientTTT.this.userName = userNameTextField.getText();
				ClientTTT.this.userNameTextField.setEnabled(false);
			}
		});
	}

	public void setupMatrix() {
		matrixIcon = new JLabel[][]{
				{square00, square01, square02},
				{square10, square11, square12},
				{square20, square21, square22}
		};

		matrix = new int[][]{
				{0, 0, 0},
				{0, 0, 0},
				{0, 0, 0}
		};

	}

	public void setupMatrixIcon() {
		class IconMouseListener extends MouseAdapter {
			private int i;
			private int j;

			public IconMouseListener(int i, int j) {
				this.i = i;
				this.j = j;
			}

			@Override
			public void mouseClicked(MouseEvent mouseEvent) {
				if (inGame && enabledRole) {
					if (matrix[i][j] == 0) {
						// empty
						matrixIcon[i][j].setIcon(((role == 1) ? xIcon : oIcon));
						matrix[i][j] = (role == 1) ? 1 : -1;
						ClientTTT.this.status = ClientTTT.this.controller.updateResult();

						// disable
						enableRole(false);
						try {
							moveTurn(i, j);
						} catch (IOException | InterruptedException e) {
							e.printStackTrace();
						}
					}
				}

				ClientTTT.this.updateResult();
			}

			@Override
			public void mouseEntered(MouseEvent mouseEvent) {
				if (inGame && enabledRole) {
					if (matrix[i][j] == 0) {
						// only when empty to be hovered
						matrixIcon[i][j].setIcon(hoveredIcon);
					}
				}
			}

			@Override
			public void mouseExited(MouseEvent mouseEvent) {
				if (inGame && enabledRole) {
					if (matrix[i][j] == 0) {
						// only when empty to be hovered
						matrixIcon[i][j].setIcon(emptyIcon);
					}
				}
			}

		}


		for (int i = 0; i < matrix.length; i++) {
			for (int j = 0; j < matrix[i].length; j++) {
				IconMouseListener listener = new IconMouseListener(i, j);
				matrixIcon[i][j].addMouseListener(listener);
			}
		}
	}

	// operational functions-------------------------------------
	// call by other player when they set their own turn
	public void moveTurn(int i, int j) throws IOException, InterruptedException {
		String message = "player," + ((role == 1)?"x":"o") + ",turn," + i + "," + j;
		// must run the add message in runnable
		ClientTTT.this.connection.moveTurn(i, j);

		//ClientTTT.this.connection.addMessage(message);
	}

	public synchronized void setMatrixPoint(int i, int j, int team) {
		if ((matrix[i][j] != 0) || role == 0) {
			System.err.println(String.format("Error to write at: %d %d for role %d\n", i, j, team));
		} else {
			//write to the matrix
			enableRole(true); // your turn
			matrix[i][j] = team;
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					matrixIcon[i][j].setIcon((team == 1) ? xIcon : oIcon);
				}
			});
			updateResult();
			//enableRole(false);
		}
	}

	public void updateResult(){
		status = controller.updateResult();

		if (ClientTTT.this.status != GameControl.GameStatus.DRAW) {
			enterGame(false);

			// debuging
			if (Main.DEBUG)
				System.out.printf(String.format("\nWin = %s", (ClientTTT.this.status == GameControl.GameStatus.XWIN) ? "X" : "O"));
		}
	}

	public void enableRole(boolean enabled) {
		this.enabledRole = enabled;
		if (this.enabledRole) {
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					statusLabel.setText("Your turn");
				}
			});
		} else {
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					statusLabel.setText("Other's turn");
				}
			});
		}
		if (Main.DEBUG){
			System.out.printf("\nenable = %s ingame = %s", this.enabledRole, inGame);
		}
	}

	public void enterGame(boolean game) {
		this.inGame = game;
		if (!inGame) {
			enableRole(false);
			// out of game
			// print the result
			SwingUtilities.invokeLater(new Runnable() {
				@Override
				public void run() {
					String result = String.format("%s win.", (ClientTTT.this.status == GameControl.GameStatus.XWIN) ? "X" : "O");
					statusLabel.setText(result);
				}
			});
		}
	}

	public void setRole(int i) throws IOException, InterruptedException {
		this.role = i;
		if (role == -1){
			// start the game from here
			enterGame(true);
			ClientTTT.this.connection.addMessage("player,o,game,start");
		}
		SwingUtilities.invokeLater(new Runnable() {
			@Override
			public void run() {
				userRoleLabel.setText("You're " + ((role == 1)?"X":"O"));
			}
		});
	}

	public int getRole(){
		return this.role;
	}

}
