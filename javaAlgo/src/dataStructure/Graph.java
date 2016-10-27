package dataStructure;

import main.*;
import java.util.*;

/**
 * Created by nxphi47 on 10/27/16.
 */
public class Graph<T extends Comparable<T>> {
	public static final int INF = 100000;
	private int sizeNode = 0;
	private int sizeEdge = 0;
	private boolean isDirected;
	private boolean isWeighted;
	private ArrayList<Edge<T>> edgeList;
	private LinkedListCustom<Node<T>> array;

	public int getSizeNode() {
		sizeNode = getArray().size();
		return sizeNode;
	}

	public void setSizeNode(int sizeNode) {
		this.sizeNode = sizeNode;
	}

	public LinkedListCustom<Node<T>> getArray() {
		return array;
	}

	public void setArray(LinkedListCustom<Node<T>> array) {
		this.array = array;
	}

	public ArrayList<Edge<T>> getEdgeList() {
		return edgeList;
	}

	public void setEdgeList(ArrayList<Edge<T>> edgeList) {
		this.edgeList = edgeList;
	}

	public int getSizeEdge() {
		sizeEdge = isDirected?getEdgeList().size():getEdgeList().size()/2;
		return sizeEdge;
	}

	public void setSizeEdge(int sizeEdge) {
		this.sizeEdge = sizeEdge;
	}

	public boolean isDirected() {
		return isDirected;
	}

	public void setDirected(boolean directed) {
		isDirected = directed;
	}

	public boolean isWeighted() {
		return isWeighted;
	}

	public void setWeighted(boolean weighted) {
		isWeighted = weighted;
	}

	// main methods
	public class Edge<T extends Comparable<T>> {
		public T first;
		public T second;
		public int weight;
		public Edge(T f, T s, int wt){
			first = f;
			second = s;
			weight = wt;
		}
		public Edge(T f, T s) {this(f, s, 1);}
	}


	public Graph(boolean isDirected, boolean isWeighted){
		sizeNode = 0;
		sizeEdge = 0;
		this.isDirected = isDirected;
		this.isWeighted = isWeighted;
		array = new LinkedListCustom<>();
		edgeList = new ArrayList<>();
	}

	public int containsNode(T val){
		for (int i = 0; i < getSizeNode(); i++){
			if (getArray().get(i).getValue().equals(val)){
				return i;
			}
		}
		return -1;
	}

	public int containsEdge(T f, T s){
		for (Edge<T> edge: getEdgeList()){
			if (edge.first.equals(f) && edge.second.equals(s)){
				return getEdgeList().indexOf(edge);
			}
			if (!isDirected && edge.first.equals(s) && edge.second.equals(f)){
				return getEdgeList().indexOf(edge);
			}
		}
		return -1;
	}

	public Graph<T> addNode(T val){
		getArray().add(new Node<T>(val));
		return this;
	}
	public Graph<T> addNode(Node<T> node){
		getArray().add(node);
		return this;
	}
	public Node<T> getNodeByIndex(int index){return getArray().get(index);}
	public Node<T> getNodeByNode(Node<T> no){return getArray().get(this.containsNode(no.getValue()));}
	public Edge<T> getEdge(T f, T s) {return getEdgeList().get(containsEdge(f, s));}

	public Graph<T> addEdge(T val1, T val2, int wt) throws Exception{
		int indexVal1 = this.containsNode(val1);
		int indexVal2 = this.containsNode(val2);
		if (indexVal1 != -1 && indexVal2 != -1){
			Node<T> srcNode = getNodeByIndex(indexVal1);
			Node<T> temp = srcNode;
			Node<T> descNode = getNodeByIndex(indexVal2);
			while (temp.getRight()!=null){temp = temp.getRight();}
			temp.setRight(new Node(descNode.getValue()));
			getEdgeList().add(new Edge<T>(srcNode.getValue(), descNode.getValue(), wt));
			if (!isDirected){
				temp = descNode;
				while (temp.getRight() != null){temp = temp.getRight();}
				temp.setRight(new Node(srcNode.getValue()));
				getEdgeList().add(new Edge<T>(descNode.getValue(), srcNode.getValue(), wt));
			}
		}
		else {
			String val1Error = (this.containsNode(val1) == -1)?val1.toString():"";
			String val2Error = (this.containsNode(val2) == -1)?val2.toString():"";
			throw new Exception(String.format("There is no value: %s %s\n", val1Error, val2Error));

		}
		return this;
	}

	public Graph<T> addEdge(T v1,T v2) throws Exception{return this.addEdge(v1,  v2, 1);}

	public void printGraph(){
		System.out.printf("\nGraph %s %s:", isDirected?"Directed":"undirected", isWeighted?"weighted":"unWeighted");
		for (int i = 0; i < getSizeNode(); i ++){
			System.out.printf("\nNode %s -> ", getNodeByIndex(i));
			Node<T> temp = getNodeByIndex(i);
			while (temp.getRight() != null){
				temp = temp.getRight();
				System.out.printf("[%s %s] ", temp, isWeighted?getEdge(getNodeByIndex(i).getValue(), temp.getValue()).weight:"");
			}
		}

	}

	// searching
	public ArrayList<Node<T>> breadthFirstSearch(T firstNode, Main.Method method){
		ArrayList<Node<T>> visitedList = new ArrayList<>(getSizeNode());
		if (containsNode(firstNode) == -1){
			System.err.printf("\nERROR: no %s in the graph.\n", firstNode);
			return visitedList;
		}
		Queue<Node<T>> nodeQueue = new LinkedList<>();
		nodeQueue.offer(getNodeByIndex(containsNode(firstNode)));
		Node<T> visited;
		while (!nodeQueue.isEmpty()){
			visited = nodeQueue.poll();
			visited = getNodeByNode(visited); //get realnode
			visitedList.add(visited);
			// do the visited
			if (!method.visitNow(visited)) {break;}

			Node<T> temp = visited;
			while (temp.getRight() != null) {
				temp = temp.getRight();
				if (!visitedList.contains(temp) && !nodeQueue.contains(temp)) {
					if (!method.visitAfter(visited, getNodeByNode(temp))) {break;}
					nodeQueue.offer(temp);
				}
			}
		}
		return visitedList;
	}

	public ArrayList<Node<T>> depthFirstSearch(T firstNode, Main.Method method){
		ArrayList<Node<T>> visitedList = new ArrayList<>(getSizeNode());
		if (containsNode(firstNode) == -1){
			System.err.printf("\nERROR: no %s in the graph.\n", firstNode);
			return visitedList;
		}
		Stack<Node<T>> stack = new Stack<>();
		stack.push(getNodeByIndex(containsNode(firstNode)));
		Node<T> visited;
		while (!stack.isEmpty()){
			visited = stack.pop();
			visited = getNodeByNode(visited); //get realnode
			visitedList.add(visited);
			// do the visited
			if (!method.visitNow(visited)) {break;}

			Node<T> temp = visited;
			while (temp.getRight() != null) {
				temp = temp.getRight();
				if (!visitedList.contains(temp) && !stack.contains(temp)) {
					if (!method.visitAfter(visited, getNodeByNode(temp))) {
						break;
					}
					stack.push(temp);
				}
			}
		}
		return visitedList;
	}

	public Graph<T> shortedPath(T startVal, Graph<T> spanningTree, Main.Method method) throws Exception{
		int sizeNode = getSizeNode();
		int sizeEdge = getSizeEdge();
		LinkedListCustom<T> addedNodes = new LinkedListCustom<T>();
		// marking all nodes if INF
		for (Node<T> node : getArray()){
			node.setMarker(INF);
		}
		// add the first node to spt, addedNodes, and update marker for adjacent
		Node<T> firstNode = getNodeByIndex(containsNode(startVal));
		firstNode.setMarker(0);
		spanningTree.addNode((Node<T>) firstNode.clone());
		addedNodes.add(firstNode.getValue());
		Node<T> temp = firstNode;
		while (temp.getRight() != null){
			temp = temp.getRight();
			getNodeByNode(temp).setMarker(getEdge(firstNode.getValue(), temp.getValue()).weight);
			getNodeByNode(temp).setLeft(firstNode);
		}


		while (spanningTree.getSizeEdge() < getSizeEdge() - 1){
			//find the min marked node
			int index = -1;
			int wt = INF;
			for (Node<T> node: getArray()){
				if (!addedNodes.contains(node.getValue()) && node.getMarker() < wt){
					wt = node.getMarker();
					index = getArray().indexOf(node);
				}
			}
			if (index == -1) { break;}
			// add toBeadded to spt and addedNodes
			Node<T> toBeAdded = getArray().get(index);
			spanningTree.addNode((Node<T>) toBeAdded.clone());
			//spanningTree.printGraph();
			Edge<T> edgeTobeAdded = getEdge(toBeAdded.getLeft().getValue(), toBeAdded.getValue());
			spanningTree.addEdge(toBeAdded.getLeft().getValue(), toBeAdded.getValue(), edgeTobeAdded.weight);
			addedNodes.add(toBeAdded.getValue());

			// update marker for the rest of nodes
			for (Node<T> restNode: getArray()){
				int newEdgeIndex = containsEdge(toBeAdded.getValue(), restNode.getValue());
				if (!addedNodes.contains(restNode.getValue()) && newEdgeIndex != -1){
					// update marker
					if (toBeAdded.getMarker() + getEdgeList().get(newEdgeIndex).weight < restNode.getMarker()){
						restNode.setMarker(toBeAdded.getMarker() + getEdgeList().get(newEdgeIndex).weight);
						restNode.setLeft(toBeAdded);
					}
				}
			}

		}


		return spanningTree;
	}
}
