#include "cube.h"

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