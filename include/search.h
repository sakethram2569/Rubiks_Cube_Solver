#ifndef SEARCH_H
#define SEARCH_H

#include "cube.h"
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

struct Move {
    Cube::Face face;
    int turns; // 1 = clockwise, 2 = 180, 3 = counterclockwise
};

std::string moveToString(const Move& m);

struct ScrambleResult {
    Cube cube;
    std::vector<Move> moves;
};

ScrambleResult generateScramble(int length);

std::optional<std::vector<Move>> solveDFS(Cube start, int maxDepth, long long& nodesVisited);

std::optional<std::vector<Move>> solveIDDFS(Cube start, int maxDepthCap, long long& nodesVisited);

std::optional<std::vector<Move>> solveBFS(Cube start, int maxDepth, long long& nodesVisited, long long nodeLimit);

// IDA* using the corner PDB as an admissible heuristic. nodeLimit caps total
// node expansions across the whole search (all threshold iterations combined)
// -- if exceeded before a solution is found, returns nullopt instead of
// running indefinitely on a pathological case. Prints a short progress line
// every 5 million nodes so a long-running trial doesn't look frozen.
std::optional<std::vector<Move>> solveIDAStar(Cube start, const std::vector<uint8_t>& pdb,
                                               long long& nodesVisited, long long nodeLimit);

#endif