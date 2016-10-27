package main;

import dataStructure.*;

import java.util.*;

public class Main {
	public static long hanoiTimes = 0;


	public static void main(String[] args) throws Exception {

		graphWeighted();
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

	public static void graphTopoSort() throws Exception {
		Graph_back<String> graphBack = new Graph_back<>(8);
		graphBack.addNode("A");
		graphBack.addNode("B");
		graphBack.addNode("C");
		graphBack.addNode("E");
		graphBack.addNode("F");
		graphBack.addNode("G");
		graphBack.addNode("H");
		graphBack.addNode("D");
		// edge
		graphBack.addEdge("A", "E", true);
		graphBack.addEdge("A", "D", true);
		graphBack.addEdge("B", "E", true);
		graphBack.addEdge("C", "A", true);
		graphBack.addEdge("C", "B", true);
		graphBack.addEdge("C", "G", true);
		graphBack.addEdge("C", "F", true);
		graphBack.addEdge("D", "F", true);
		graphBack.addEdge("D", "E", true);
		graphBack.addEdge("E", "F", true);
		graphBack.addEdge("F", "H", true);

		graphBack.printGraph();
		System.out.printf("\nToposort: %s", graphBack.topologicalSort());
	}

	public static void graphTopoSort(int test) throws Exception {
		Graph_back<Integer> graphBack = new Graph_back<>(13);
		for (int i = 13;i >= 5; i--){
			graphBack.addNode(i);
		}
		for (int i = 1; i <5; i++){
			graphBack.addNode(i);
		}
		// add end;
		graphBack.addEdge(1, 2, true);
		graphBack.addEdge(1, 3, true);
		graphBack.addEdge(2, 4, true);
		graphBack.addEdge(2, 5, true);
		graphBack.addEdge(3, 6, true);
		graphBack.addEdge(3, 7, true);
		graphBack.addEdge(4, 8, true);
		graphBack.addEdge(5, 9, true);
		graphBack.addEdge(5, 10, true);
		graphBack.addEdge(6, 10, true);
		graphBack.addEdge(7, 12, true);
		graphBack.addEdge(8, 11, true);
		graphBack.addEdge(9, 11, true);
		graphBack.addEdge(10, 12, true);
		graphBack.addEdge(11, 13, true);
		graphBack.addEdge(12, 13, true);

		// pring graphBack
		graphBack.printGraph();

		System.out.printf("\nToposort: %s", graphBack.topologicalSort());
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