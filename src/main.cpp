#include "cube.h"
#include <iostream>

void testOrderFour(Cube::Face face, const char* name) {
    Cube c;
    for (int i = 0; i < 4; ++i) c.move(face, 1);
    std::cout << name << " applied 4 times returns to solved: "
              << (c.isSolved() ? "PASS" : "FAIL") << "\n";
}

int main() {
    // Test 1: any single quarter turn, applied 4 times, must return to solved
    testOrderFour(Cube::U, "U");
    testOrderFour(Cube::D, "D");
    testOrderFour(Cube::L, "L");
    testOrderFour(Cube::R, "R");
    testOrderFour(Cube::F, "F");
    testOrderFour(Cube::B, "B");

    // Test 2: the famous "sexy move" (R U R' U') has order 6
    Cube c3;
    for (int i = 0; i < 6; ++i) {
        c3.move(Cube::R, 1);
        c3.move(Cube::U, 1);
        c3.move(Cube::R, 3); // R'
        c3.move(Cube::U, 3); // U'
    }
    std::cout << "(R U R' U') x6 returns to solved: "
              << (c3.isSolved() ? "PASS" : "FAIL") << "\n";

    // Test 3: scramble, then apply the exact inverse sequence
    Cube c4;
    c4.move(Cube::R, 1);
    c4.move(Cube::U, 2);
    c4.move(Cube::F, 3);
    c4.move(Cube::L, 1);
    // inverse: reverse order, and invert each turn (1<->3, 2 stays 2)
    c4.move(Cube::L, 3);
    c4.move(Cube::F, 1);
    c4.move(Cube::U, 2);
    c4.move(Cube::R, 3);
    std::cout << "Scramble + exact inverse returns to solved: "
              << (c4.isSolved() ? "PASS" : "FAIL") << "\n";

    return 0;
}