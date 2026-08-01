#ifndef CUBE_H
#define CUBE_H

#include <array>
#include <iostream>

class Cube {
public:
    static const int NUM_CORNERS = 8;
    static const int NUM_EDGES = 12;

    // Singmaster notation: 8 corner slots, 12 edge slots
    enum Corner { URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB };
    enum Edge { UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR };

    // The six faces. A "move" is a face + how many clockwise quarter turns:
    // 1 = clockwise, 2 = 180 degrees, 3 = counterclockwise
    enum Face { U, D, L, R, F, B };

    Cube(); // builds a solved cube

    bool operator==(const Cube& other) const;
    bool operator!=(const Cube& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Cube& c);

    bool isSolved() const;
    void move(Face face, int turns);

private:
    std::array<int, NUM_CORNERS> cp; // corner permutation: which piece is at slot i
    std::array<int, NUM_CORNERS> co; // corner orientation: 0, 1, or 2
    std::array<int, NUM_EDGES> ep;   // edge permutation
    std::array<int, NUM_EDGES> eo;   // edge orientation: 0 or 1
};

#endif