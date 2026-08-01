#ifndef MOVETABLES_H
#define MOVETABLES_H

#include <array>

// Corner/edge permutation + orientation-delta tables for one clockwise
// quarter turn of each of the 6 faces (U, D, L, R, F, B in that order).
// This is the same verified data used since Phase 2 -- pulled out into its
// own header so both Cube::move() and the pattern database builder use the
// exact same source of truth instead of two copies that could drift apart.
struct MoveDef {
    std::array<int, 8> cp;
    std::array<int, 8> co;
    std::array<int, 12> ep;
    std::array<int, 12> eo;
};

inline const MoveDef baseMoves[6] = {
    // U
    {{3,0,1,2,4,5,6,7}, {0,0,0,0,0,0,0,0}, {3,0,1,2,4,5,6,7,8,9,10,11}, {0,0,0,0,0,0,0,0,0,0,0,0}},
    // D
    {{0,1,2,3,5,6,7,4}, {0,0,0,0,0,0,0,0}, {0,1,2,3,5,6,7,4,8,9,10,11}, {0,0,0,0,0,0,0,0,0,0,0,0}},
    // L
    {{0,2,6,3,4,1,5,7}, {0,1,2,0,0,2,1,0}, {0,1,10,3,4,5,9,7,8,2,6,11}, {0,0,0,0,0,0,0,0,0,0,0,0}},
    // R
    {{4,1,2,0,7,5,6,3}, {2,0,0,1,1,0,0,2}, {8,1,2,3,11,5,6,7,4,9,10,0}, {0,0,0,0,0,0,0,0,0,0,0,0}},
    // F
    {{1,5,2,3,0,4,6,7}, {1,2,0,0,2,1,0,0}, {0,9,2,3,4,8,6,7,1,5,10,11}, {0,1,0,0,0,1,0,0,1,1,0,0}},
    // B
    {{0,1,3,7,4,5,2,6}, {0,0,1,2,0,0,2,1}, {0,1,2,11,4,5,6,10,8,9,3,7}, {0,0,0,1,0,0,0,1,0,0,1,1}}
};

#endif