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

// Generates a scramble of the given length. Uses move pruning (no immediate
// same-face repeat, canonical ordering for same-axis opposite faces) so the
// scramble can't be trivially shortened by an obvious cancellation.
ScrambleResult generateScramble(int length);

// Fixed-depth-limited DFS. Returns the FIRST solution it stumbles into at
// depth <= maxDepth, in depth-first traversal order — NOT guaranteed to be
// the shortest solution, just "a" solution within the depth bound.
std::optional<std::vector<Move>> solveDFS(Cube start, int maxDepth, long long& nodesVisited);

// Iterative deepening DFS: runs solveDFS-style search at depth = 0, 1, 2, ...
// until something is found. Because every shallower depth is fully ruled out
// first, the depth at which it succeeds is guaranteed minimal — same
// optimality guarantee as BFS, but with DFS's O(depth) memory footprint.
std::optional<std::vector<Move>> solveIDDFS(Cube start, int maxDepthCap, long long& nodesVisited);

// True breadth-first search. Guarantees the shortest solution, but stores
// every node it visits in memory. nodeLimit caps that growth — if it's hit
// before a solution is found, this returns nullopt instead of exhausting RAM.
std::optional<std::vector<Move>> solveBFS(Cube start, int maxDepth, long long& nodesVisited, long long nodeLimit);

// IDA*: iterative deepening on f = g + h instead of raw depth, using the
// corner pattern database as an admissible heuristic h. Guarantees the
// shortest solution (like BFS/IDDFS) but the heuristic prunes far more of
// the tree than plain depth pruning does, and memory stays O(depth) like
// DFS — no node-storage blowup like BFS has.
std::optional<std::vector<Move>> solveIDAStar(Cube start, const std::vector<uint8_t>& pdb, long long& nodesVisited);

#endif