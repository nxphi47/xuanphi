package main;

/**
 * Created by fi on 9/19/2017.
 * print all valid combinations on n-pairs parentheses
 */
public class ValidParentheses {
	public ValidParentheses() {
		System.out.printf("Start here\n");
//		System.out.printf("%s", recur(3));
		String[] result = recur(2);
		for (String str: result) {
			System.out.printf("%s\n", str);
		}
	}

	public String[] recur(int n) {
		if (n == 0) {
			return new String[]{""};
		}
		else if (n == 1) {
			return new String[]{"()"};
		}
		else {
			String[] subs = recur(n - 1);
			String[] vals = new String[subs.length * 3];

			for (int i = 0; i < subs.length; i++) {
				vals[i * 3] = String.format("%s%s%s", "()", subs[i], "");
				vals[i * 3 + 1] = String.format("%s%s%s", "(", subs[i], ")");
				vals[i * 3 + 2] = String.format("%s%s%s", "", subs[i], "()");
			}
			return vals;
		}
	}
}
