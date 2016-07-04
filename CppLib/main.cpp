#include <iostream>
// main is use for testing
#include "Vector.h"

using namespace std;

// CPPLIB is where C++ library is personally develop and may be implemented in other project
// perhaps something we may come into memory situation like arduino, practive memory saving


int main() {
	cout << "Hello, World!" << endl;
	Vector<int> *ptr = new Vector<int>();
	delete ptr;
	int x = 2;
	ptr = new Vector<int>(x);
	cout << "hi: " << ptr->get(0);


	return 0;
}