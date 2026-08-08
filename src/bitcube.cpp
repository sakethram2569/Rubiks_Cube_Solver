#include "bitcube.h"
#include "movetables.h"

namespace {
    constexpr int CBITS = 5;
    constexpr int EBITS = 5;
    constexpr uint64_t CMASK = 0x1F;
    constexpr uint64_t EMASK = 0x1F;

    inline uint64_t packCorner(int perm, int orient) {
        return static_cast<uint64_t>(perm) | (static_cast<uint64_t>(orient) << 3);
    }
    inline int cPerm(uint64_t field) { return static_cast<int>(field & 0x7); }
    inline int cOrient(uint64_t field) { return static_cast<int>((field >> 3) & 0x3); }

    inline uint64_t packEdge(int perm, int orient) {
        return static_cast<uint64_t>(perm) | (static_cast<uint64_t>(orient) << 4);
    }
    inline int ePerm(uint64_t field) { return static_cast<int>(field & 0xF); }
    inline int eOrient(uint64_t field) { return static_cast<int>((field >> 4) & 0x1); }

    // Same permutation + orientation-delta data as Cube::move(), applied
    // via bit extraction/insertion on a packed word instead of writing
    // into a second std::array.
    uint64_t cornerQuarterTurn(uint64_t state, const MoveDef& m) {
        uint64_t result = 0;
        for (int i = 0; i < 8; ++i) {
            uint64_t field = (state >> (CBITS * m.cp[i])) & CMASK;
            int perm = cPerm(field);
            int orient = (cOrient(field) + m.co[i]) % 3;
            result |= packCorner(perm, orient) << (CBITS * i);
        }
        return result;
    }

    uint64_t edgeQuarterTurn(uint64_t state, const MoveDef& m) {
        uint64_t result = 0;
        for (int i = 0; i < 12; ++i) {
            uint64_t field = (state >> (EBITS * m.ep[i])) & EMASK;
            int perm = ePerm(field);
            int orient = (eOrient(field) + m.eo[i]) % 2;
            result |= packEdge(perm, orient) << (EBITS * i);
        }
        return result;
    }
}

BitCube::BitCube() {
    cornerState = 0;
    for (int i = 0; i < NUM_CORNERS; ++i)
        cornerState |= packCorner(i, 0) << (CBITS * i);

    edgeState = 0;
    for (int i = 0; i < NUM_EDGES; ++i)
        edgeState |= packEdge(i, 0) << (EBITS * i);
}

bool BitCube::operator==(const BitCube& other) const {
    return cornerState == other.cornerState && edgeState == other.edgeState;
}

bool BitCube::operator!=(const BitCube& other) const {
    return !(*this == other);
}

bool BitCube::isSolved() const {
    static const BitCube solved;
    return cornerState == solved.cornerState && edgeState == solved.edgeState;
}

void BitCube::move(Face face, int turns) {
    const MoveDef& m = baseMoves[face];
    for (int t = 0; t < turns; ++t) {
        cornerState = cornerQuarterTurn(cornerState, m);
        edgeState = edgeQuarterTurn(edgeState, m);
    }
}

std::array<int, BitCube::NUM_CORNERS> BitCube::cornerPerm() const {
    std::array<int, NUM_CORNERS> result;
    for (int i = 0; i < NUM_CORNERS; ++i)
        result[i] = cPerm((cornerState >> (CBITS * i)) & CMASK);
    return result;
}

std::array<int, BitCube::NUM_CORNERS> BitCube::cornerOrient() const {
    std::array<int, NUM_CORNERS> result;
    for (int i = 0; i < NUM_CORNERS; ++i)
        result[i] = cOrient((cornerState >> (CBITS * i)) & CMASK);
    return result;
}

std::array<int, BitCube::NUM_EDGES> BitCube::edgePerm() const {
    std::array<int, NUM_EDGES> result;
    for (int i = 0; i < NUM_EDGES; ++i)
        result[i] = ePerm((edgeState >> (EBITS * i)) & EMASK);
    return result;
}

std::array<int, BitCube::NUM_EDGES> BitCube::edgeOrient() const {
    std::array<int, NUM_EDGES> result;
    for (int i = 0; i < NUM_EDGES; ++i)
        result[i] = eOrient((edgeState >> (EBITS * i)) & EMASK);
    return result;
}