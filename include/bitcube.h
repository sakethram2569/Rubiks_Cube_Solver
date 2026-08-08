#ifndef BITCUBE_H
#define BITCUBE_H

#include <array>
#include <cstdint>

// A from-scratch bit-packed reimplementation of the same cubie-level model
// as Cube (include/cube.h), built for Phase 7. Cube stores corner/edge
// permutation and orientation as four separate std::array members; every
// move() call reads and writes all four arrays (40 int slots, ~160 bytes)
// through memory. BitCube stores the entire state in two uint64_t words
// instead -- 8 corners packed 5 bits each (3 for permutation, 2 for
// orientation) into one 64-bit word, and 12 edges packed 5 bits each
// (4 for permutation, 1 for orientation) into another. A move becomes
// register-level bit-shifting instead of array copying.
//
// Whether that's actually faster in practice, and by how much, is exactly
// what the Phase 7 benchmark in main.cpp measures -- this class makes no
// performance claim on its own.
class BitCube {
public:
    static const int NUM_CORNERS = 8;
    static const int NUM_EDGES = 12;
    enum Face { U, D, L, R, F, B };

    BitCube(); // solved cube

    bool operator==(const BitCube& other) const;
    bool operator!=(const BitCube& other) const;

    bool isSolved() const;
    void move(Face face, int turns);

    // Unpacked accessors -- only used for cross-checking against Cube,
    // never on the hot path.
    std::array<int, NUM_CORNERS> cornerPerm() const;
    std::array<int, NUM_CORNERS> cornerOrient() const;
    std::array<int, NUM_EDGES> edgePerm() const;
    std::array<int, NUM_EDGES> edgeOrient() const;

private:
    uint64_t cornerState; // 8 x 5 bits: [orient(2 bits) | perm(3 bits)]
    uint64_t edgeState;   // 12 x 5 bits: [orient(1 bit) | perm(4 bits)]
};

#endif