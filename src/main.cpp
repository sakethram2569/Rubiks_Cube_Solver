#include "cube.h"
#include <iostream>

int main() {
    Cube c1;
    Cube c2;

    std::cout << "c1:\n" << c1;
    std::cout << "Is c1 solved? " << (c1.isSolved() ? "yes" : "no") << "\n";
    std::cout << "c1 == c2? " << (c1 == c2 ? "yes" : "no") << "\n";

    return 0;
}