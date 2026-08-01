#include "pdb.h"
#include "movetables.h"
#include <fstream>
#include <iostream>
#include <chrono>

namespace {
    // Ranks a permutation of {0..7} into [0, 8!) using the standard
    // Lehmer-code / factorial-number-system technique: for each position,
    // count how many later elements are smaller, then weight by (7-i)!.
    int permRank(const std::array<uint8_t, 8>& perm) {
        static const int fact[8] = {5040, 720, 120, 24, 6, 2, 1, 1}; // (7-i)!
        int rank = 0;
        for (int i = 0; i < 8; ++i) {
            int smaller = 0;
            for (int j = i + 1; j < 8; ++j) {
                if (perm[j] < perm[i]) ++smaller;
            }
            rank += smaller * fact[i];
        }
        return rank;
    }

    // Treats the first 7 corner orientations as a base-3 number. The 8th
    // is never independently indexed since it's always determined by the
    // other 7 (orientations must sum to 0 mod 3).
    int orientIndex(const std::array<uint8_t, 8>& co) {
        int idx = 0;
        for (int i = 0; i < 7; ++i) idx = idx * 3 + co[i];
        return idx;
    }

    long long cornerIndex(const std::array<uint8_t, 8>& cp, const std::array<uint8_t, 8>& co) {
        return static_cast<long long>(permRank(cp)) * NUM_CORNER_ORIENTS + orientIndex(co);
    }

    // Same math as Cube::move(), restricted to corners only -- deliberately
    // not going through a Cube object here, since we call this ~1.6 billion
    // times during the BFS and skipping the unused edge-array work matters.
    void applyCornerMove(std::array<uint8_t, 8>& cp, std::array<uint8_t, 8>& co, int face, int turns) {
        const MoveDef& m = baseMoves[face];
        for (int t = 0; t < turns; ++t) {
            std::array<uint8_t, 8> newCp, newCo;
            for (int i = 0; i < 8; ++i) {
                newCp[i] = cp[m.cp[i]];
                newCo[i] = static_cast<uint8_t>((co[m.cp[i]] + m.co[i]) % 3);
            }
            cp = newCp;
            co = newCo;
        }
    }

    struct CornerState {
        std::array<uint8_t, 8> cp;
        std::array<uint8_t, 8> co;
    };
}

std::vector<uint8_t> buildCornerPDB() {
    std::vector<uint8_t> dist(static_cast<size_t>(NUM_CORNER_STATES), 0xFF);

    std::array<uint8_t, 8> solvedCp = {0, 1, 2, 3, 4, 5, 6, 7};
    std::array<uint8_t, 8> solvedCo = {0, 0, 0, 0, 0, 0, 0, 0};
    dist[cornerIndex(solvedCp, solvedCo)] = 0;

    std::vector<CornerState> currentLevel;
    currentLevel.push_back({solvedCp, solvedCo});

    int depth = 0;
    long long filled = 1;

    while (!currentLevel.empty()) {
        std::vector<CornerState> nextLevel;
        for (const auto& s : currentLevel) {
            for (int f = 0; f < 6; ++f) {
                for (int turns = 1; turns <= 3; ++turns) {
                    std::array<uint8_t, 8> ncp = s.cp;
                    std::array<uint8_t, 8> nco = s.co;
                    applyCornerMove(ncp, nco, f, turns);
                    long long idx = cornerIndex(ncp, nco);
                    if (dist[idx] == 0xFF) {
                        dist[idx] = static_cast<uint8_t>(depth + 1);
                        nextLevel.push_back({ncp, nco});
                        ++filled;
                    }
                }
            }
        }
        ++depth;
        currentLevel = std::move(nextLevel);
    }

    std::cout << "  corner BFS finished: " << filled << " / " << NUM_CORNER_STATES
              << " states reached, max depth " << (depth - 1) << "\n";

    return dist;
}

bool loadCornerPDB(const std::string& path, std::vector<uint8_t>& dist) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    dist.resize(static_cast<size_t>(NUM_CORNER_STATES));
    in.read(reinterpret_cast<char*>(dist.data()), NUM_CORNER_STATES);

    if (in.gcount() != static_cast<std::streamsize>(NUM_CORNER_STATES)) {
        dist.clear();
        return false;
    }
    return true;
}

void saveCornerPDB(const std::string& path, const std::vector<uint8_t>& dist) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(dist.data()), static_cast<std::streamsize>(dist.size()));
}

std::vector<uint8_t> getCornerPDB(const std::string& path) {
    std::vector<uint8_t> dist;
    if (loadCornerPDB(path, dist)) {
        std::cout << "Loaded corner pattern database from " << path << "\n";
        return dist;
    }

    std::cout << "Building corner pattern database (" << NUM_CORNER_STATES
              << " states) -- this only happens once, it's cached to disk after.\n";
    auto start = std::chrono::steady_clock::now();
    dist = buildCornerPDB();
    auto end = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << "  built in " << seconds << " s. Saving to " << path << "...\n";
    saveCornerPDB(path, dist);
    return dist;
}

int cornerDistance(const std::vector<uint8_t>& pdb, const Cube& c) {
    auto fullCp = c.cornerPerm();
    auto fullCo = c.cornerOrient();
    std::array<uint8_t, 8> cp, co;
    for (int i = 0; i < 8; ++i) {
        cp[i] = static_cast<uint8_t>(fullCp[i]);
        co[i] = static_cast<uint8_t>(fullCo[i]);
    }
    return pdb[cornerIndex(cp, co)];
}