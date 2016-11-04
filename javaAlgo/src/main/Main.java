package main;

import dataStructure.*;

import java.util.*;

public class Main {
	public static long hanoiTimes = 0;


	public static void main(String[] args) throws Exception {
//		ConnectedCellGrid cellGrid = new ConnectedCellGrid(true);
		Pairs pairs = new Pairs(false);
	}

	public static int appointRegion(int[][] mainArr, int[][] regArr, int[][] marker, int i, int j){

		return 0;
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