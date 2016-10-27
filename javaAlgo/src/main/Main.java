package main;

import dataStructure.*;

import java.util.*;

public class Main {
	public static long hanoiTimes = 0;


	public static void main(String[] args) throws Exception {
		graphFunc();
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
		void function1(Node node);
		void function2(Node prev, Node target);
	}

	public static class MethodSet {
		public void print(Integer i){
			System.out.printf("%d ", i);
		}
		public void printSQ(Integer i){
			System.out.printf("%d ",i * i);
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

	public static void graphFunc() throws Exception {
		Graph<String> graph = new Graph<>(10);
		graph.addNode("A");
		graph.addNode("B");
		graph.addNode("C");
		graph.addNode("D");
		graph.addNode("E");
		graph.addNode("F");
		graph.addNode("G");
		graph.addNode("H");

		graph.addPath("A", "C");
		graph.addPath("B", "D");
		graph.addPath("D", "C");
		graph.addPath("D", "E");
		graph.addPath("A", "F");
		graph.addPath("A", "G");
		graph.addPath("F", "H");
		graph.addPath("G", "H");
		graph.printGraph();

		System.out.printf("\n");
		ArrayList<Node<String>> list = graph.depthFirstSearch("C", new Method() {
			@Override
			public void function1(Node node) {
				return;
			}

			@Override
			public void function2(Node prev, Node target) {
				return;
			}
		});
		System.out.printf("\nsequence depth: %s", list);

		// shorted part
		HashMap<String, Integer> lengths = new HashMap<>();
		list = graph.breadthFirstSearch("C", new Method() {
			boolean lock = false;
			@Override
			public void function1(Node node) {
				if (!lock){
					lengths.put((String) node.getValue(), 0);
					lock = !lock;
				}
			}

			@Override
			public void function2(Node prev, Node target) {
				if (!lengths.containsKey(target.getValue())){

					lengths.put((String) target.getValue(), lengths.get(prev.getValue()) + 1);
				}
			}
		});


		System.out.printf("\nsequence breadth: %s", list);
		System.out.printf("\nShorted part: %s", lengths);
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