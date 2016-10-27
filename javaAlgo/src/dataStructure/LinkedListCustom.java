package dataStructure;

import java.util.Comparator;
import java.util.LinkedList;

/**
 * Created by nxphi47 on 10/26/16.
 */
public class LinkedListCustom<T extends Comparable<T>> extends LinkedList<Comparable<T>> {

	@Override
	public boolean contains(Object o) {
		T obj = (T) o;
		for (int i = 0; i < this.size(); i++){
			if (this.get(i).equals(o)){
				return true;
			}
		}
		return false;
	}
}
