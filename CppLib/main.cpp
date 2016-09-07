#include <iostream>
// main is use for testing
#include "Vector.h"

using namespace std;

// CPPLIB is where C++ library is personally develop and may be implemented in other project
// perhaps something we may come into memory situation like arduino, practive memory saving


int main() {
	Coor x = {0,3};
	Coor y = {1,4};
	Coor z = {4,5};
	Vector<Coor> vector;
	vector.append(x)->append(y);
	cout << vector.get(0).X << " " << vector.get(0).Y << endl;
	cout << vector.get(1).X << " " << vector.get(1).Y << endl;
	cout << vector.get(2).X << " " << vector.get(2).Y << endl;

	return 0;
}