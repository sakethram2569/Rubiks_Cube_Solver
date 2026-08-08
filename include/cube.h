#ifndef CUBE_H
#define CUBE_H

#include <array>
#include <iostream>

class Cube {
public:
    static const int NUM_CORNERS = 8;
    static const int NUM_EDGES = 12;

    enum Corner { URF, UFL, ULB, UBR, DFR, DLF, DBL, DRB };
    enum Edge { UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR };
    enum Face { U, D, L, R, F, B };

    Cube();

    bool operator==(const Cube& other) const;
    bool operator!=(const Cube& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Cube& c);

    bool isSolved() const;
    void move(Face face, int turns);

    std::array<int, NUM_CORNERS> cornerPerm() const { return cp; }
    std::array<int, NUM_CORNERS> cornerOrient() const { return co; }

    // Added for Phase 7 -- cross-checking BitCube's bit-packed edge state
    // against this array-based implementation requires reading edges too,
    // not just corners. Read-only, doesn't touch move()/isSolved() at all.
    std::array<int, NUM_EDGES> edgePerm() const { return ep; }
    std::array<int, NUM_EDGES> edgeOrient() const { return eo; }

private:
    std::array<int, NUM_CORNERS> cp;
    std::array<int, NUM_CORNERS> co;
    std::array<int, NUM_EDGES> ep;
    std::array<int, NUM_EDGES> eo;
};

#endif