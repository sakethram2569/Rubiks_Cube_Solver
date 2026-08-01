#include "cube.h"
#include "movetables.h"

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