#include "cube.h"

namespace {
    struct MoveDef {
        std::array<int, 8> cp;
        std::array<int, 8> co;
        std::array<int, 12> ep;
        std::array<int, 12> eo;
    };

    // Verified corner/edge permutation + orientation-delta tables for one
    // clockwise quarter turn of each face.
    const MoveDef baseMoves[6] = {
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
}

Cube::Cube() {
    for (int i = 0; i < NUM_CORNERS; ++i) {
        cp[i] = i;
        co[i] = 0;
    }
    for (int i = 0; i < NUM_EDGES; ++i) {
        ep[i] = i;
        eo[i] = 0;
    }
}

bool Cube::operator==(const Cube& other) const {
    return cp == other.cp && co == other.co &&
           ep == other.ep && eo == other.eo;
}

bool Cube::operator!=(const Cube& other) const {
    return !(*this == other);
}

bool Cube::isSolved() const {
    for (int i = 0; i < NUM_CORNERS; ++i) {
        if (cp[i] != i || co[i] != 0) return false;
    }
    for (int i = 0; i < NUM_EDGES; ++i) {
        if (ep[i] != i || eo[i] != 0) return false;
    }
    return true;
}

void Cube::move(Face face, int turns) {
    const MoveDef& m = baseMoves[face];

    for (int t = 0; t < turns; ++t) {
        std::array<int, NUM_CORNERS> newCp;
        std::array<int, NUM_CORNERS> newCo;
        std::array<int, NUM_EDGES> newEp;
        std::array<int, NUM_EDGES> newEo;

        for (int i = 0; i < NUM_CORNERS; ++i) {
            newCp[i] = cp[m.cp[i]];
            newCo[i] = (co[m.cp[i]] + m.co[i]) % 3;
        }
        for (int i = 0; i < NUM_EDGES; ++i) {
            newEp[i] = ep[m.ep[i]];
            newEo[i] = (eo[m.ep[i]] + m.eo[i]) % 2;
        }

        cp = newCp;
        co = newCo;
        ep = newEp;
        eo = newEo;
    }
}

std::ostream& operator<<(std::ostream& os, const Cube& c) {
    os << "Corners (perm/orient): ";
    for (int i = 0; i < Cube::NUM_CORNERS; ++i)
        os << c.cp[i] << "/" << c.co[i] << " ";
    os << "\nEdges   (perm/orient): ";
    for (int i = 0; i < Cube::NUM_EDGES; ++i)
        os << c.ep[i] << "/" << c.eo[i] << " ";
    os << "\n";
    return os;
}