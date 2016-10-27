package dataStructure;

import main.Main;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Stack;

/**
 * Created by nxphi47 on 10/26/16.
 */
// this graph is implement on unweighted graph
public class Graph<T extends Comparable<T>> {
	private int sizeNode = 0;
	private int sizeEdge = 0;
	private ArrayList<Node<T>> array;

	public Graph(int size){
		sizeNode = size;
		array = new ArrayList<>(size);
	}

	public Graph(){
		this(10);
	}

	public int contains(T val){
		Node<T> node = new Node<T>(val);
		for (int i = 0; i < array.size(); i++){
			if (array.get(i).equals(node)){
				return i;
			}
		}
		return -1;
	}

	public Graph addNode(T val){
		array.add(new Node<T>(val));
		return this;
	}

	public Graph addPath(T val1, T val2, boolean isDirected) throws Exception{
//		System.out.printf("\nAdd part: %s %s %s", val1, val2, String.valueOf(isDirected));
		int indexVal1 = this.contains(val1);
		int indexVal2 = this.contains(val2);
		if (indexVal1 != -1 && indexVal2 != -1){
			Node srcNode = array.get(indexVal1);
			Node temp = srcNode;
			Node descNode = array.get(indexVal2);
			while (temp.getRight() != null){
				temp = temp.getRight();
			}
			temp.setRight(new Node(descNode.getValue()));
			if (!isDirected) {
				temp = descNode;
				while (temp.getRight() != null){
					temp = temp.getRight();
				}
				temp.setRight(new Node(srcNode.getValue()));
			}
		}
		else {
			String val1Error = (this.contains(val1) == -1)?val1.toString():"";
			String val2Error = (this.contains(val2) == -1)?val2.toString():"";
			throw new Exception(String.format("There is no value: %s %s\n", val1Error, val2Error));

		}
		return this;
	}

	// default is undirected
	public Graph addPath(T val1, T val2) throws Exception {
		return this.addPath(val1, val2, false);
	}

	public void printGraph(){
		System.out.printf("\nGraph: ");
		for (int i = 0; i < array.size(); i++){
			System.out.printf("\nNode %s -> ", array.get(i));
			Node temp = array.get(i);
			while (temp.getRight() != null) {
				System.out.printf("%s ", temp.getRight());
				temp = temp.getRight();
			}
		}
	}

	// FIXME: The primary node of array is main, the linked node is not node!
	public ArrayList<Node<T>> breadthFirstSearch(T firstnode, Main.Method method){
		ArrayList<Node<T>> visitedList = new ArrayList<>();
		if (this.contains(firstnode) == -1){
			System.err.printf("\nERROR: no %s in the graph.\n", firstnode);
			return visitedList;
		}
		Queue<Node<T>> nodeQueue = new LinkedList<>();
		nodeQueue.offer(array.get(this.contains(firstnode)));
		Node<T> visited = null;

		while (!nodeQueue.isEmpty()){
			visited = nodeQueue.poll();
			// get the real visited
			// because the one added is the linked node, not the primary node
			visited = array.get(this.contains(visited.getValue()));
			visitedList.add(visited);
			// do the visited
			method.function1(visited);
			Node<T> temp = visited;
			while (temp.getRight() != null){
				temp = temp.getRight();
				if (!visitedList.contains(temp) && !nodeQueue.contains(temp)){
					// not in visited list
					method.function2(visited, temp);
					nodeQueue.offer(temp);
				}
			}
		}
		return visitedList;
	}

	public ArrayList<Node<T>> depthFirstSearch(T firstNode, Main.Method method){
		ArrayList<Node<T>> visitedList = new ArrayList<>();
		if (this.contains(firstNode) == -1){
			System.err.printf("\nERROR: node: %s not in graph!\n", firstNode);
			return visitedList;
		}
		Stack<Node<T>> stack = new Stack<>();
		stack.push(array.get(this.contains(firstNode)));
		Node<T> visited = null;
		while (!stack.isEmpty()){
			visited = stack.pop();
			visited = array.get(this.contains(visited.getValue()));
			// visit and add to visited
			visitedList.add(visited);
			method.function1(visited);
			Node<T> temp = visited;
			while (temp.getRight() != null){
				temp = temp.getRight();
				if (!visitedList.contains(temp) && !stack.contains(temp)){
					// add to stack
					method.function2(visited,temp);
					stack.push(temp);
				}
			}
		}

		return visitedList;
	}
}
