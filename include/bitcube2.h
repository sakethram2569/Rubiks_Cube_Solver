#ifndef BITCUBE2_H
#define BITCUBE2_H

#include <array>
#include <cstdint>

// Phase 7b: same packed-state idea as BitCube, but move() is restructured
// around precomputed lookup tables instead of per-field arithmetic. Phase 7
// diagnosed BitCube's slowdown as (a) decode/recompute/repack cost per
// field and (b) a serialized OR-accumulator chain. This class targets (a)
// directly and leaves (b) exactly as it was in BitCube, so any difference
// in the result is attributable to the table-lookup change specifically,
// not a mix of several changes at once.
class BitCubeV2 {
public:
    static const int NUM_CORNERS = 8;
    static const int NUM_EDGES = 12;
    enum Face { U, D, L, R, F, B };

    BitCubeV2(); // solved cube

    bool operator==(const BitCubeV2& other) const;
    bool operator!=(const BitCubeV2& other) const;

    bool isSolved() const;
    void move(Face face, int turns);

    std::array<int, NUM_CORNERS> cornerPerm() const;
    std::array<int, NUM_CORNERS> cornerOrient() const;
    std::array<int, NUM_EDGES> edgePerm() const;
    std::array<int, NUM_EDGES> edgeOrient() const;

private:
    uint64_t cornerState;
    uint64_t edgeState;
};

#endif