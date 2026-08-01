#include "cube.h"
#include "search.h"
#include "pdb.h"
#include <iostream>

int main() {
    std::vector<uint8_t> pdb = getCornerPDB("../corner_pdb.bin");

    long long filled = 0;
    int maxDist = 0;
    for (uint8_t d : pdb) {
        if (d != 0xFF) {
            ++filled;
            if (d > maxDist) maxDist = d;
        }
    }
    std::cout << "\nPDB stats: " << filled << " / " << NUM_CORNER_STATES
              << " states filled, max distance = " << maxDist << "\n";
    if (filled != NUM_CORNER_STATES) {
        std::cout << "WARNING: not every corner state was reached -- "
                     "something is likely wrong with the move tables or indexing.\n";
    }

    std::cout << "\n--- Sanity checks ---\n";
    Cube solved;
    int solvedDist = cornerDistance(pdb, solved);
    std::cout << "Solved cube distance: " << solvedDist
               << (solvedDist == 0 ? "  OK" : "  FAIL") << "\n";

    bool allSingleMovesOne = true;
    for (int f = 0; f < 6; ++f) {
        for (int t = 1; t <= 3; ++t) {
            Cube c;
            c.move(static_cast<Cube::Face>(f), t);
            int d = cornerDistance(pdb, c);
            if (d != 1) {
                allSingleMovesOne = false;
                std::cout << "  Face " << f << " turns " << t << " -> distance " << d << " (expected 1)\n";
            }
        }
    }
    std::cout << "All 18 single moves have corner-distance 1: "
              << (allSingleMovesOne ? "OK" : "FAIL") << "\n";

    std::cout << "\n--- Admissibility check (corner distance must never exceed scramble length) ---\n";
    bool admissible = true;
    for (int trial = 0; trial < 20; ++trial) {
        int len = 4 + (trial % 9); // scramble lengths 4..12
        ScrambleResult sc = generateScramble(len);
        int h = cornerDistance(pdb, sc.cube);
        bool ok = (h <= len);
        if (!ok) admissible = false;
        std::cout << "  scramble length " << len << ", corner-distance " << h
                  << (ok ? "  OK" : "  FAIL (heuristic overestimated!)") << "\n";
    }
    std::cout << "Admissibility across 20 random scrambles: " << (admissible ? "OK" : "FAIL") << "\n";

    return 0;
}