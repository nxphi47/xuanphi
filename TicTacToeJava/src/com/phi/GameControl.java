package com.phi;

/**
 * Created by nxphi on 9/12/16.
 */

public class GameControl {
	public enum GameStatus {OWIN, DRAW, XWIN};
	private int[][] matrix;
	private GameStatus status;


	public GameControl(int[][] matrix){
		setMatrix(matrix);
		status = GameStatus.DRAW;
	}

	public GameStatus updateResult(){
		// check horizontal
		for (int i = 0; i < 3; i++){
			if (matrix[i][0] == matrix[i][1] && matrix[i][1] == matrix[i][2]){
				// equal values
				GameStatus val =checkValue(i, 0);
				if (val != GameStatus.DRAW){
					status = val;
					return val;
				}
			}
		}
		// check verical
		for (int i = 0; i < 3; i++){
			if (matrix[0][i] == matrix[1][i] && matrix[1][i] == matrix[2][i]){
				GameStatus val =checkValue(0, i);
				if (val != GameStatus.DRAW){
					status = val;
					return val;
				}
			}
		}
		// check northwest - southeast
		if (matrix[0][0] == matrix[1][1] && matrix[1][1] == matrix[2][2]){
			GameStatus val =checkValue(0, 0);
			if (val != GameStatus.DRAW){
				status = val;
				return val;
			}
		}
		// check northeast - southwest
		if (matrix[0][2] == matrix[1][1] && matrix[1][1] == matrix[2][0]){
			GameStatus val =checkValue(0, 2);
			if (val != GameStatus.DRAW){
				status = val;
				return val;
			}
		}

		return status;
	}

	public GameStatus checkValue(int i, int j){
		if (matrix[i][j] == 1){
			// x win
			return GameStatus.XWIN;
		}
		else if (matrix[i][j] == -1){
			return GameStatus.OWIN;
		}
		else {
			return GameStatus.DRAW;
		}
	}

	public int[][] getMatrix() {
		return matrix;
	}

	public void setMatrix(int[][] matrix) {
		this.matrix = matrix;
	}




}
