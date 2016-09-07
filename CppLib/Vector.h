//
// Created by phi on 01/07/16.
//

#ifndef CPPLIB_VECTOR_H
#define CPPLIB_VECTOR_H

#include <iostream>

using namespace std;

// primatively it is a doubly linkedlist
template<typename type>
struct Node {
	type val;
	Node *next;
	Node *prev;
};

struct Coor {
	short X;
	short Y;
};

enum Direction {
	UP, RIGHT, DOWN, LEFT
};

template<class T>
class Vector {
public:
	Vector() {
		vectorSize = 0;
	}

	Vector(T &start) {
		Vector();
		head = new Node<T>();
		head->val = start;
		head->prev = NULL;
		head->next = tail;
		tail = head;
		//cout << "start = " << head->val << endl;
		vectorSize = 1;
	}

	Vector(T *arrayStart, int size) {
		Vector();
		head = new Node<T>();
		head->val = *arrayStart;
		head->next = head->prev = NULL;
		Node<T> *temp = head;
		if (size == 1) {
			tail = head;
			return;
		}
		for (int i = 1; i < size - 1; ++i) {
			Node<T> *newNode = new Node<T>();
			newNode->val = *(arrayStart + i);
			newNode->prev = temp;
			newNode->next = NULL;
			temp->next = newNode;

			temp = newNode;
			//cout << "part = " << head->next->val << endl;
		}
		tail = new Node<T>();
		tail->val = *(arrayStart + size - 1);
		tail->next = NULL;
		tail->prev = temp;

		vectorSize = size;
	}

	// appending function
	Vector *append(T &newOne) {
		if (vectorSize == 0) {
			head = new Node<T>();
			head->val = newOne;
			head->prev = NULL;
			head->next = tail;
			tail = head;
			//cout << "start = " << head->val << endl;
			vectorSize = 1;
			return this;
		}

		Node<T> *newNode = new Node<T>();
		newNode->val = newOne;
		newNode->next = NULL;
		newNode->prev = tail;
		tail->next = newNode;
		//cout << "tail = " << tail->val << " prev = " << tail->prev->val << endl;
		//cout << "couse here = " << tail->prev->prev->next->val << endl;
		//cout << "\nfrom 2nd ";
		Node<T> *temp = head;
		while (temp != NULL) {
			//cout << " " << temp->val;
			temp = temp->next;
		}
		tail = newNode;
		vectorSize++;
		return this;
	}


	// poping and shifting function
	// remember to put exception
	T &pop() {
		//return ref;
		if (vectorSize == 1) {
			T ref = tail->val;
			delete head;
			head = tail = NULL;
			vectorSize = 0;
			return ref;
		}
		else if (vectorSize == 0) {
			fprintf(stderr, "\nvector size = 0, failed to pop");
			exit(1);
		}
		else {
			T ref = tail->val;
			Node<Coor> * tailRef = tail;
			tail = tail->prev;
			delete tailRef;
			vectorSize--;
			return ref;
		}
	}

	T &shift() {
		//return ref;
		if (vectorSize == 1) {
			T ref = head->val;
			delete head;
			head = tail = NULL;
			vectorSize = 0;
			return ref;
		}
		else if (vectorSize == 0) {
			//fprintf(stderr, "\nVector size = 0, failed to shift");
			exit(1);
		}
		else {
			T ref = head->val;
			Node<Coor> *headRef = head;
			head = head->next;
			delete headRef;
			vectorSize--;
			return ref;
		}
	}

	// get function to return reference of index
	T &get(int index) {
		Node<T> *tempHead = head;
		if (index < (vectorSize + 1) * -1 || index > vectorSize) {
			fprintf(stderr, "\nVector failed to insert, index out of bound");
			exit(1);
		}
		if (index < 0) { index += vectorSize; }

		if (index == 0) {
			return head->val;
		} else if (index == vectorSize - 1) {
			return tail->val;
		}
		else {
			// from the top down
			for (int i = 0; i < index; ++i) {
				tempHead = tempHead->next;
			}
			return tempHead->val;
		}
	}

	//insert, accept negative index but not exceed the size
	Vector *insert(int index, T &newOne) {
		if (index < (vectorSize + 1) * -1 || index > vectorSize) {
			fprintf(stderr, "\nVector failed to insert, index out of bound");
			exit(1);
		}
		if (index < 0) { index += vectorSize; }

		if (index == vectorSize) {
			append(newOne);
			return this;
		}

		if (index == 0) {
			Node<T> *newNode = new Node<T>();
			newNode->val = newOne;
			newNode->prev = NULL;
			newNode->next = head;
			head->prev = newNode;
			head = newNode;
			vectorSize++;
			return this;
		}


		Node<T> *temp = head;
		for (int i = 0; i < index; ++i) {
			temp = temp->next;
		}
		// it is insert before temp
		Node<T> *newNode = new Node<T>();
		newNode->val = newOne;
		newNode->next = temp;
		newNode->prev = temp->prev;
		newNode->prev->next = newNode;
		temp->prev = newNode;
		vectorSize++;
		return this;
	}

	int size() {
		return vectorSize;
	}

private:
	Node<T> *head;
	Node<T> *tail;
	int vectorSize;
};


#endif //CPPLIB_VECTOR_H
