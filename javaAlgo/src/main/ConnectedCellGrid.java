package main;

import java.util.Arrays;
import java.util.Random;
import java.util.Scanner;

/**
 * Created by nxphi on 01/11/2016.
 */
public class ConnectedCellGrid {
	public int n = 0;
	public int m = 0;
	public int currentIndex = 0;

	public ConnectedCellGrid(boolean isRandom){
		System.out.printf("%d",cellGrid(isRandom));
	}

	public String randomGridCells(){
		StringBuilder stringBuilder = new StringBuilder(100000);
		Random random = new Random();
		int n = 10;
		int m = 10;
		stringBuilder.append(String.format("%d\n%d\n", n, m));
		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				stringBuilder.append(String.format("%d ", random.nextInt(2)));
			}
			stringBuilder.append("\n");
		}
		return stringBuilder.toString();
	}

	public int cellGrid(boolean isRandom){
		Scanner scanner = isRandom?(new Scanner(randomGridCells())):(new Scanner(System.in));
		n = scanner.nextInt();
		m = scanner.nextInt();
		int[][] mainGrid = new int[n][m];
		int[][] regGrid = new int[n][m];
		int[][] marker = new int[n][m];
		for (int i = 0 ; i < n; i++){
			Arrays.fill(regGrid[i], 0);
			Arrays.fill(marker[i], 0);
		}

		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				mainGrid[i][j] = scanner.nextInt();
			}
		}
		for (int i = 0; i <n; i++){
			for (int j = 0; j < m; j++){
				if (marker[i][j] == 0 && mainGrid[i][j] == 1){
					// not marked and =1
					currentIndex++;
					// grow it
					grow(mainGrid, marker, regGrid, i, j);
				}
			}
		}
		System.out.printf("\nAfter regrid:\n");
		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				System.out.printf("%d ", regGrid[i][j]);
			}
			System.out.printf("\n");
		}

		int[] regCount = new int[currentIndex + 1];
		Arrays.fill(regCount, 0);
		for (int i = 0; i < n; i++){
			for (int j = 0; j < m; j++){
				if (regGrid[i][j] != 0){
					regCount[regGrid[i][j]]++;
				}
			}
		}
		int max = 0;
		for (int k = 1; k < regCount.length; k++){
			if (regCount[k] > max){
				max = regCount[k];
			}
		}
		return max;
	}

	public int grow(int[][] main, int[][] marker, int[][] reg, int i, int j){
		marker[i][j] = 1;
		reg[i][j] = currentIndex;
		// north west
		if (i - 1 >= 0 && j - 1 >= 0 && main[i - 1][j - 1] == 1 && marker[i - 1][j - 1] == 0){
			grow(main, marker, reg, i - 1, j - 1);
		}
		// north
		if (i - 1 >= 0 && main[i - 1][j] == 1 && marker[i - 1][j] == 0){
			grow(main, marker, reg, i - 1, j);
		}
		// north east
		if (i - 1 >= 0 && j + 1 < m && main[i - 1][j + 1] == 1 && marker[i - 1][j + 1] ==0){
			grow(main, marker, reg, i - 1, j + 1);
		}
		// east
		if (j + 1 < m && main[i][j + 1] == 1 && marker[i][j + 1] == 0){
			grow(main, marker, reg, i, j + 1);
		}
		// south ease
		if (i + 1 < n && j + 1 < m && main[i + 1][j + 1] == 1 && marker[i + 1][j + 1] ==0){
			grow(main, marker, reg, i + 1, j + 1);
		}
		// south
		if (i + 1 < n && main[i + 1][j] == 1 && marker[i + 1][j] == 0){
			grow(main, marker, reg, i + 1, j);
		}
		// south west
		if (i + 1 < n && j - 1 >= 0 && main[i + 1][j - 1] == 1 && marker[i + 1][j - 1] == 0){
			grow(main, marker, reg, i + 1, j - 1);
		}
		// west
		if (j - 1 >= 0 && main[i][j - 1] == 1 && marker[i][j - 1] == 0){
			grow(main, marker, reg, i,j - 1);
		}

		return 0;
	}
}
