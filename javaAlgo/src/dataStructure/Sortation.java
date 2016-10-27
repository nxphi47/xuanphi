package dataStructure;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by nxphi47 on 10/26/16.
 */
public class Sortation {
	public static void printArray(Integer[] arr){
		System.out.printf("\narray: ");
		for (Integer x: arr){
			System.out.printf("%d ", x);
		}
	}

	// for merge sort

	public static void merge(Integer[] arr, int start, int mid, int end){
		ArrayList<Integer> list = new ArrayList<>(Arrays.asList(arr));
		List<Integer> list1 = new ArrayList<>(list.subList(start, mid));
		List<Integer> list2 = new ArrayList<>(list.subList(mid, end + 1));

		int index = start;
		while (!list1.isEmpty() && !list2.isEmpty()){
			arr[index++] = (list1.get(0) < list2.get(0))?list1.remove(0):list2.remove(0);
		}
		while (!list1.isEmpty()){
			arr[index++] = list1.remove(0);
		}
		while (!list2.isEmpty()){
			arr[index++] = list2.remove(0);
		}
	}
	public static void mergeSort(Integer[] arr, int start, int end){
		if (start == end){
			return;
		}
		else {
			int mid = (start + end)/2;
			mergeSort(arr, start, mid);
			mergeSort(arr, mid + 1, end);
			merge(arr, start, mid + 1, end);
		}
	}

	// heap sort for min heap, descending order [max heap is for ascending)
	public static void swap(Integer[] arr, int i, int j){
		int temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
	}
	public static void siftdown(Integer[] arr, int i, int n){
		if (i*2 + 1 <= n){
			int toSwap = i*2 + 1; // left side
			if (toSwap + 1 <= n) {
				toSwap = (arr[toSwap] < arr[toSwap + 1])?toSwap:toSwap + 1; // find the min
			}
			if (arr[i] > arr[toSwap]){
				swap(arr, i, toSwap);
				siftdown(arr, toSwap, n);
			}
		}
	}
	public static void heapify(Integer[] arr){
		int start = (int) (Math.ceil((double) (arr.length-1)/2) - 1);
		for (int i = start; i >= 0; i--){
			siftdown(arr, i, arr.length - 1);
		}
	}
	public static void heapSort(Integer[] arr){
		//heapsort in descending
		heapify(arr);

		int n = arr.length - 1;
		while (n > 0){
			swap(arr, 0, n);
			n--;
			siftdown(arr, 0, n);
		}
	}

	// partitiona and quicksort
	public static int partition(Integer[] arr, int start, int end){
		// end must include the last index
		int val = arr[start];
		int h = start;
		for (int k = start + 1; k <= end; k++){
			if (arr[k] < val){
				h++;
				swap(arr, h, k);
			}
		}
		swap(arr, start, h);
		return h;
	}
	public static void quickSort(Integer[] arr, int start, int end){
		if (start > end){
			return;
		}
		else {
			int mid = partition(arr, start, end);
			printArray(arr);
			quickSort(arr, start, mid-1);
			quickSort(arr, mid + 1, end);
		}
	}


}
