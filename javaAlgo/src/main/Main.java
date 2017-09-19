package main;

import dataStructure.*;

import java.util.*;
import java.util.regex.Matcher;

public class Main {
	public static long hanoiTimes = 0;

	public static void main(String[] args) throws Exception {


		ValidParentheses validParentheses = new ValidParentheses();

//		rungeKutaMultiple(3, 3, 2, 0.1, 3);
	}


	public static double funcRungeKuta(double x, double y){
		return 1/x + 2 * y;
	}

	public static double funcSys1(double x, double y1, double y2){

		return y2;
	}
	public static double funcSys2(double x, double y1, double y2){

//		return 1 - 3 * y1 / x + 2 * y2 / x;
//		return (2 * x * y2 + 1 - y1) / x;
//		return (y2 - x * y1 + 3 * x) / (x * 2);
		return (2 - x * y2 + 4 * y1) / x;
	}


	public static double eulerMultiple(double x, double y1, double y2, double h, double iteration){
		double nextY1;
		double nextY2;
		for (int i = 0; i < iteration; i ++){
			System.out.printf("%d%12f%12f%12f\n", i, x, y1, y2);
			nextY1 = y1 + h * funcSys1(x, y1, y2);
			nextY2 = y2 + h * funcSys2(x, y1, y2);
			y1 = nextY1;
			y2 = nextY2;
			x += h;
		}
		return y1;
	}

	public static double eulerImprovedMultiple(double x, double y1, double y2, double h, double iteration){
		double nextY1;
		double nextY2;
		double a1, a2;
		double b1, b2;
		for (int i = 0; i < iteration; i ++){
			a1 = h * funcSys1(x, y1, y2);
			a2 = h * funcSys2(x, y1, y2);
			b1 = h * funcSys1(x + h, y1 + a1, y2 + a2);
			b2 = h * funcSys2(x + h, y1 + a1, y2 + a2);
			System.out.printf("%d %12f %12f %12f %12f %12f %12f %12f\n", i, x, y1, y2, a1, a2, b1, b2);
			nextY1 = y1 + 0.5 * (a1 + b1);
			nextY2 = y2 + 0.5 * (a2 + b2);
			y1 = nextY1;
			y2 = nextY2;
			x += h;
		}
		return y1;
	}

	public static double rungeKutaMultiple(double x, double y1, double y2, double h, double iteration){
		double nextY1;
		double nextY2;
		double a1, a2;
		double b1, b2;
		double c1, c2;
		double d1, d2;
		for (int i = 0; i < iteration; i ++){
			a1 = h * funcSys1(x, y1, y2);
			a2 = h * funcSys2(x, y1, y2);
			b1 = h * funcSys1(x + h / 2, y1 + a1 / 2, y2 + a2 / 2);
			b2 = h * funcSys2(x + h / 2, y1 + a1 / 2, y2 + a2 / 2);
			c1 = h * funcSys1(x + h / 2, y1 + b1 / 2, y2 + b2 / 2);
			c2 = h * funcSys2(x + h / 2, y1 + b1 / 2, y2 + b2 / 2);
			d1 = h * funcSys1(x + h, y1 + c1, y2 + c2);
			d2 = h * funcSys2(x + h, y1 + c1, y2 + c2);
			System.out.printf("%d %12f %12f %12f %12f %12f %12f %12f %12f %12f %12f %12f\n", i, x, y1, y2, a1, a2, b1, b2, c1, c2, d1, d2);
			nextY1 = y1 + (a1 + 2 * b1 + 2 * c1 + d1) / 6;
			nextY2 = y2 + (a2 + 2 * b2 + 2 * c2 + d2) / 6;
			y1 = nextY1;
			y2 = nextY2;
			x += h;
		}
		return y1;
	}



	public static double rungeKutaSingle(double x, double y, double h, int iteration){
//		double x = 1;
//		double h = 0.1;
//		double y = 4;
		double a,b,c,d;
		System.out.printf("xn ---- yn ------ a ------- b ------c -------d\n");
		for (int i = 0; i < iteration; i++){
			a = h * funcRungeKuta(x, y);
			b = h * funcRungeKuta(x + h / 2, y + a / 2);
			c = h * funcRungeKuta(x + h / 2, y + b / 2);
			d = h * funcRungeKuta(x + h, y + c);
			System.out.printf("%12f%12f%12f%12f%12f%12f\n", x, y, a, b, c, d);
			y += (a + 2 * b + 2 * c + d)/6;
			x += h;
		}
		return y;
	}





	public static int powerRecursive(int i, int n){
		if (n == 0){
			return 1;
		}
		else if (n%2 == 0){
			// even
			int x = powerRecursive(i, n/2);
			return x * x;
		}
		else {
			int x = powerRecursive(i, (n-1)/2);
			return i*x*x;
		}

	}

	public interface Method {
		boolean visitNow(Node node); // to be visited right now
		boolean visitAfter(Node prev, Node target); // put to list to be visited
	}

	public static class MethodSet {
		public boolean print(Integer i){
			System.out.printf("%d ", i);
			return true;
		}
		public boolean printSQ(Integer i){
			System.out.printf("%d ",i * i);
			return true;
		}
	}


	public static void hashTableFunc(){
		Map<String, Integer> hashTable = new HashMap<>(20);
		Scanner scanner = new Scanner(System.in);
		String line = scanner.nextLine();
		String[] words = line.split(" ");

		for (String w: words){
			if (hashTable.containsKey(w)){
				hashTable.put(w, hashTable.get(w)+1);
			}
			else {
				hashTable.put(w, 1);
			}
		}

		// sort the key
		SortedSet<String> set = new TreeSet<>(hashTable.keySet());
		for (String k: set){
			System.out.printf("%s: %d\n", k, hashTable.get(k));
		}

	}

	public static <T extends Comparable<T>> void printStackRecursive(Stack<T> stack){
		if (stack.isEmpty()){
			return;
		}
		else {
			T temp = stack.pop();
			// print top first
			System.out.printf(" %s", String.valueOf(temp));
			printStackRecursive(stack);
			stack.push(temp);
		}

		//System.out.printf("\nStack: (top)");

	}

	public static void graphWeighted() throws Exception{
		Graph<String> graph = new Graph<>(false, true);
		graph.addNode("A");
		graph.addNode("B");
		graph.addNode("C");
		graph.addNode("D");
		graph.addNode("E");
		graph.addNode("F");

		// add edge with weight
		graph.addEdge("A", "B", 80);
		graph.addEdge("A", "C", 40);
		graph.addEdge("A", "E", 60);
		graph.addEdge("B", "D", 100);
		graph.addEdge("C", "D", 20);
		graph.addEdge("C", "E", 120);
		graph.addEdge("C", "F", 60);
		graph.addEdge("D", "F", 120);
		graph.addEdge("E", "F", 40);

		graph.printGraph();

		Graph<String> spt = new Graph<>(false, true);
		graph.shortedPath("A", spt, null);
		spt.printGraph();
		//System.out.printf(String.valueOf(graph.getSizeEdge(false)));
	}

	public static void graphFunc() throws Exception {
		Graph<String> graph = new Graph<>(false, false);
		graph.addNode("A");
		graph.addNode("B");
		graph.addNode("C");
		graph.addNode("D");
		graph.addNode("E");
		graph.addNode("F");
		graph.addNode("G");
		graph.addNode("H");

		graph.addEdge("A", "C");
		graph.addEdge("B", "D");
		graph.addEdge("D", "C");
		graph.addEdge("D", "E");
		graph.addEdge("A", "F");
		graph.addEdge("A", "G");
		graph.addEdge("F", "H");
		graph.addEdge("G", "H");
		graph.printGraph();

		System.out.printf("\n");
		System.out.printf("%s %s\n", graph.getSizeNode(), graph.getSizeEdge());
		System.out.printf("%s %s\n", graph.getNodeByIndex(2), graph.getNodeByNode(new Node<>("C")));

		ArrayList<Node<String>> list = graph.breadthFirstSearch("C", new Method() {
			@Override
			public boolean visitNow(Node node) {
				return true;
			}

			@Override
			public boolean visitAfter(Node prev, Node target) {
				return true;
			}
		});

		System.out.printf("\nsequence Breadth: %s", list);
		list = graph.depthFirstSearch("A", new Method() {
			@Override
			public boolean visitNow(Node node) {
				return true;
			}

			@Override
			public boolean visitAfter(Node prev, Node target) {
				return true;
			}
		})	;
		System.out.printf("\nsequence breadth: %s", list);
	}

	public static void printList(List list, String name){
		System.out.printf("\n%s: ", name);
		for (Object o: list){
			System.out.printf(" %s", o.toString());
		}
		System.out.printf("\n");
	}

	public static void solveTowerHanoi(int size, int source, int temp, int desc){
		if (size == 1){
			hanoiTimes++;
			return;
		}
		solveTowerHanoi(size - 1, source, desc,temp);
		hanoiTimes++;
		solveTowerHanoi(size - 1, temp, source, desc);

	}
}