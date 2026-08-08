#include "bitcube2.h"
#include "movetables.h"

namespace {
    constexpr int CBITS = 5;
    constexpr int EBITS = 5;
    constexpr uint64_t FMASK = 0x1F;

    inline uint64_t packCorner(int perm, int orient) {
        return static_cast<uint64_t>(perm) | (static_cast<uint64_t>(orient) << 3);
    }
    inline int cPerm(uint64_t f) { return static_cast<int>(f & 0x7); }
    inline int cOrient(uint64_t f) { return static_cast<int>((f >> 3) & 0x3); }

    inline uint64_t packEdge(int perm, int orient) {
        return static_cast<uint64_t>(perm) | (static_cast<uint64_t>(orient) << 4);
    }
    inline int ePerm(uint64_t f) { return static_cast<int>(f & 0xF); }
    inline int eOrient(uint64_t f) { return static_cast<int>((f >> 4) & 0x1); }

    // cornerMove[face][destSlot][rawSourceValue] = the exact bits to OR into
    // the result -- already reoriented AND already shifted into destSlot's
    // position. cornerSrc[face][destSlot] = which slot to read those raw
    // bits from. Built once, from the same baseMoves[] data Cube::move()
    // and BitCube::move() both already use, so table content traces back
    // to the one already-verified source of truth -- not re-derived geometry.
    struct Tables {
        uint64_t cornerMove[6][8][32];
        uint8_t cornerSrc[6][8];
        uint64_t edgeMove[6][12][32];
        uint8_t edgeSrc[6][12];

        Tables() {
            for (int face = 0; face < 6; ++face) {
                const MoveDef& m = baseMoves[face];

                for (int dest = 0; dest < 8; ++dest) {
                    cornerSrc[face][dest] = static_cast<uint8_t>(m.cp[dest]);
                    for (int raw = 0; raw < 32; ++raw) {
                        int perm = cPerm(raw);
                        int orient = (cOrient(raw) + m.co[dest]) % 3;
                        cornerMove[face][dest][raw] = packCorner(perm, orient) << (CBITS * dest);
                    }
                }

                for (int dest = 0; dest < 12; ++dest) {
                    edgeSrc[face][dest] = static_cast<uint8_t>(m.ep[dest]);
                    for (int raw = 0; raw < 32; ++raw) {
                        int perm = ePerm(raw);
                        int orient = (eOrient(raw) + m.eo[dest]) % 2;
                        edgeMove[face][dest][raw] = packEdge(perm, orient) << (EBITS * dest);
                    }
                }
            }
        }
    };

    const Tables& tables() {
        static const Tables t; // built once, first time move() is ever called
        return t;
    }
}

BitCubeV2::BitCubeV2() {
    cornerState = 0;
    for (int i = 0; i < NUM_CORNERS; ++i)
        cornerState |= packCorner(i, 0) << (CBITS * i);

    edgeState = 0;
    for (int i = 0; i < NUM_EDGES; ++i)
        edgeState |= packEdge(i, 0) << (EBITS * i);
}

bool BitCubeV2::operator==(const BitCubeV2& other) const {
    return cornerState == other.cornerState && edgeState == other.edgeState;
}

bool BitCubeV2::operator!=(const BitCubeV2& other) const {
    return !(*this == other);
}

bool BitCubeV2::isSolved() const {
    static const BitCubeV2 solved;
    return cornerState == solved.cornerState && edgeState == solved.edgeState;
}

void BitCubeV2::move(Face face, int turns) {
    const Tables& tb = tables();
    for (int t = 0; t < turns; ++t) {
        uint64_t newCorner = 0;
        for (int dest = 0; dest < 8; ++dest) {
            uint64_t raw = (cornerState >> (CBITS * tb.cornerSrc[face][dest])) & FMASK;
            newCorner |= tb.cornerMove[face][dest][raw];
        }
        uint64_t newEdge = 0;
        for (int dest = 0; dest < 12; ++dest) {
            uint64_t raw = (edgeState >> (EBITS * tb.edgeSrc[face][dest])) & FMASK;
            newEdge |= tb.edgeMove[face][dest][raw];
        }
        cornerState = newCorner;
        edgeState = newEdge;
    }
}

std::array<int, BitCubeV2::NUM_CORNERS> BitCubeV2::cornerPerm() const {
    std::array<int, NUM_CORNERS> r;
    for (int i = 0; i < NUM_CORNERS; ++i) r[i] = cPerm((cornerState >> (CBITS * i)) & FMASK);
    return r;
}
std::array<int, BitCubeV2::NUM_CORNERS> BitCubeV2::cornerOrient() const {
    std::array<int, NUM_CORNERS> r;
    for (int i = 0; i < NUM_CORNERS; ++i) r[i] = cOrient((cornerState >> (CBITS * i)) & FMASK);
    return r;
}
std::array<int, BitCubeV2::NUM_EDGES> BitCubeV2::edgePerm() const {
    std::array<int, NUM_EDGES> r;
    for (int i = 0; i < NUM_EDGES; ++i) r[i] = ePerm((edgeState >> (EBITS * i)) & FMASK);
    return r;
}
std::array<int, BitCubeV2::NUM_EDGES> BitCubeV2::edgeOrient() const {
    std::array<int, NUM_EDGES> r;
    for (int i = 0; i < NUM_EDGES; ++i) r[i] = eOrient((edgeState >> (EBITS * i)) & FMASK);
    return r;
}