package dataStructure;

import java.security.PublicKey;
import java.util.Comparator;

/**
 * Created by nxphi47 on 10/26/16.
 */
public class Node<T extends Comparable<T> > implements Comparable<Node> {
	private T value;
	private int index;
	private T secValue;
	private Node left;
	private Node right;
	private Node top;

	public Node(T value, T secValue, Node left, Node right, Node top, int index){
		this.value = value;
		this.secValue = secValue;
		this.left = left;
		this.right = right;
		this.top = top;
		this.index = index;
	}

	@Override
	public boolean equals(Object obj) {
		if (obj instanceof Node && ((Node) obj).getValue() != null){
			//System.out.printf("\nSame type");
			return value.equals(((Node) obj).getValue());
		}
		else {
			return false;
		}
	}

	@Override
	public String toString() {
		return getValue().toString();
	}

	public Node(T value, Node left,Node right, Node top){
		this(value, null, left,right, top, 0);
	}

	public Node(T value, Node left, Node right){
		this(value, left, right, null);
	}

	public Node(T value, Node next){
		this(value, null, next);
	}

	public Node(T value){
		this(value, null);
	}

	public Node(T value, int index){
		this(value, null,null, null, null, index);
	}

	public Node(){
		this(null);
	}

	@Override
	public int compareTo(Node o) {
		return value.compareTo((T) o.getValue());
	}

	public T getValue() {
		return value;
	}

	public void setValue(T value) {
		this.value = value;
	}

	public int getIndex() {
		return index;
	}

	public void setIndex(int index) {
		this.index = index;
	}

	public T getSecValue() {
		return secValue;
	}

	public void setSecValue(T secValue) {
		this.secValue = secValue;
	}

	public Node getLeft() {
		return left;
	}

	public void setLeft(Node left) {
		this.left = left;
	}

	public Node getRight() {
		return right;
	}

	public void setRight(Node right) {
		this.right = right;
	}


	public Node getTop() {
		return top;
	}

	public void setTop(Node top) {
		this.top = top;
	}
}
