#include <iostream>
// main is use for testing
#include "Vector.h"
using namespace std;



int main() {
    cout << "Hello, World!" << endl;
    string x1 = "hello";
    string x2 = "world";
    string x3 = "phi";
    string x4 = "vy";
    Vector<string> vector;
    vector.append(x1);
    vector.append(x2);
    vector.append(x3);
    vector.insert(vector.size(), x4);

    cout << "\nsize " << vector.size() << endl;

    for (int i = 0; i < vector.size(); ++i) {
        cout << "val = " << vector.get(i) << endl;
    }


    return 0;
}