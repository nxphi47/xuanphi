package main;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;
import java.util.Scanner;

/**
 * Created by nxphi on 01/11/2016.
 * https://www.hackerrank.com/challenges/pairs
 */
public class Pairs {
	public int n; // N integers 2 - 10^5
	public int k; // k integers 0 - 10^9

	public Pairs(boolean isRandom){
//		System.out.printf("\n%s", randomInput());
		Scanner scanner = isRandom?(new Scanner(randomInput())):(new Scanner(System.in));
		System.out.printf("finish randomize");
		n = scanner.nextInt();
		k = scanner.nextInt();
		ArrayList<Integer> list = new ArrayList<>(n);
		for (int i = 0; i < n; i++){
			list.add(scanner.nextInt());
		}
		Collections.sort(list);
		if (k == 0){
			System.out.printf("%s", combinationBig(n, 2));
			return;
		}
		// if not = 0, must find pairs
		int pairs = 0;
		int current = 0;
		for (int i = 0; i < n; i++){
			int firstIndex = Collections.binarySearch(list, list.get(i) + k);
			if (firstIndex != -1 && firstIndex > i) {
				pairs++;
			}
		}
		System.out.printf("\n%d", pairs);
	}

	public String randomInput(){
		StringBuilder builder = new StringBuilder(10000000);
		int n = 10000;
		int k = 123143;
		int rand = 0;
		Random random = new Random();
		builder.append(String.format("%d %d\n", n, k));
		for (int i = 0; i < n; i++){
			do {
				rand = random.nextInt(Integer.MAX_VALUE);
			} while (builder.indexOf(String.valueOf(rand)) != -1);
			builder.append(String.format("%d ", rand));
		}

		return builder.toString();
	}

	public static BigInteger combinationBig(int n, int k){
		return permutationBig(n, k).divide(factorial(k));
	}


	public static BigInteger permutationBig (int n, int k){
		BigInteger val = new BigInteger(String.valueOf(n));
		for (int i = n - 1; i > n - k; i--){
			val = val.multiply(new BigInteger(String.valueOf(i)));
		}
		return val;
	}

	public static BigInteger factorial(int n) {
		BigInteger factorial = BigInteger.valueOf(1);
		for (int i = 1; i <= n; i++) {
			factorial = factorial.multiply(BigInteger.valueOf(i));
		}
		return factorial;
	}
}
