package main;
import java.math.BigInteger;
import java.util.*;

/**
 * Created by nxphi on 31/10/2016.
 * https://www.hackerrank.com/challenges/short-palindrome
 */
public class ShortPalindrome {
	public static final int randomSize = 1000000;
	public static final long limitOutput = 1000000007;

	public String randomInput(int size){

		StringBuilder builder = new StringBuilder(10000000);
		Random random = new Random();
		for (int i = 0; i < size; i++){
			builder.append((char) (random.nextInt(26) + 97));
		}
		return builder.toString();
	}

	public ShortPalindrome(boolean isRandom){
		Scanner scanner = isRandom?(new Scanner(randomInput(randomSize))):(new Scanner(System.in));
		String str = scanner.nextLine();
//		System.out.printf("\nString generated\n");
		BigInteger numPali = new BigInteger(String.valueOf(0));

		ArrayList<Integer>[] listIndex = new ArrayList[26];
		for (int i = 0; i < 26; i++){
			int finalI = i + 97;
			int index = str.indexOf((char) finalI);
			listIndex[i] = new ArrayList<>(1000000);
			while (index != -1) {
				listIndex[i].add(index);
				index = str.indexOf((char) finalI, index+1);
			}
			if (listIndex[i].size() >= 4){
				// return combination of big number after mod 4
				numPali = numPali.add(combinationBig(listIndex[i].size(), 4));
			}
			if (listIndex[i].size() != 0) {
//				System.out.printf("\nsize %d = %s", i, listIndex[i]);
			}
		}
//		System.out.printf("\nFirst check = %s", numPali);
//
//		String dupString;
//		ArrayList<Integer> listI = new ArrayList<>(1000000);
//		ArrayList<Integer> listJ = new ArrayList<>(1000000);
//		for (int i = 0; i < 26; i++) {
//			for (int j = i + 1; j < 26; j++) {
//				dupString = str.replaceAll("[^" + (char) (i + 97) + "" + (char) (j + 97) + "]", "");
//				int currentIndex = i;
//				int count = 0;
//				for (int k = 0; k < dupString.length(); k++) {
//					if (dupString.charAt(k) - 97 == i) {
//						count++;
//					} else {
//						listI.add(count);
//						count = 0;
//					}
//				}
//				for (int k = 0; k < dupString.length(); k++) {
//					if (dupString.charAt(k) - 97 == j) {
//						count++;
//					} else {
//						listJ.add(count);
//						count = 0;
//					}
//				}
////				System.out.printf("\nlist i = %s, list j = %s", listI.size(), listJ.size());
//			}
//		}
//
//		System.out.printf("\nOld algo start!");
		int firstLeft;
		int firstRight;
		String leftString;
		String rightString;
		int count;
		for (int i = 0; i < 26; i++) {
			for (int j = 0; j < 26; j++) {
				if (i == j) {
					continue;
				}
				if (listIndex[i].size() == 0 || listIndex[j].size() == 0) {
					continue;
				}
				// first character vs second character, not viceversia
				// Method = find any two combination for left and right and find the combination bounded inside
//				System.out.printf("\ni = %s j = %s", i, j);

				// FIXME: too many combination for left and right
				for (int left = 0; left < listIndex[i].size(); left++) {
					for (int right = left + 1; right < listIndex[i].size(); right++) {
						// find total listIndex[j] between left and right
						firstLeft = listIndex[i].get(left);
						firstRight = listIndex[i].get(right);
						count = 0;
						// FIXME: change to find index first and last
						// FIXME: put as O(n)
						leftString = str.substring(0, firstLeft);
						rightString = str.substring(firstRight + 1);
//						System.out.printf("\nFinish divide string");
						int lastLeftIndex = leftString.lastIndexOf((char) (j + 97));
						int firstRightIndex = rightString.indexOf((char) (j + 97));
//						System.out.printf("\nFinish find index and lastindex");
						long secondLeftNum = listIndex[j].indexOf(lastLeftIndex) + 1;
						long secondRightNum = listIndex[j].size() - listIndex[j].indexOf(firstRightIndex + (str.length() - rightString.length()));

						if (lastLeftIndex < 0 || firstRightIndex < 0 || secondLeftNum < 0 || secondRightNum < 0){
							continue;
						}
//						System.out.printf("\n %s %s ||| %d %d %s %s", leftString, rightString, lastLeftIndex, firstRightIndex, secondLeftNum, secondRightNum);
//						System.out.printf("\n first %d second %d left = %d right = %d and %d", i,j, left, right, count);
						numPali = numPali.add(BigInteger.valueOf(secondLeftNum * secondRightNum));
//						System.out.printf("\nFinish find all adding");
					}
				}
			}
		}
		numPali = numPali.mod(new BigInteger(String.valueOf(limitOutput)));
		System.out.printf("%s", numPali);
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
