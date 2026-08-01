#include "search.h"
#include <random>
#include <queue>
#include <algorithm>

namespace {
    int axisOf(Cube::Face f) { return static_cast<int>(f) / 2; }

    // Move pruning shared by scramble generation, DFS, and BFS:
    // - never repeat the same face twice in a row (redundant: U U = U2, already covered by turns)
    // - for opposite faces on the same axis (U/D, L/R, F/B), which commute,
    //   only allow the canonical order (lower enum value first) so "U D" and
    //   "D U" aren't both explored as distinct branches for the same result
    bool allowedMove(Cube::Face face, Cube::Face prevFace, bool hasPrev) {
        if (!hasPrev) return true;
        if (face == prevFace) return false;
        if (axisOf(face) == axisOf(prevFace) && face < prevFace) return false;
        return true;
    }

    struct BFSNode {
        Cube state;
        int parent;
        Cube::Face face;
        int turns;
    };

    bool dfsRecurse(Cube& current, int depth, int maxDepth,
                     Cube::Face prevFace, bool hasPrev,
                     std::vector<Move>& path, long long& nodesVisited) {
        ++nodesVisited;
        if (current.isSolved()) return true;
        if (depth == maxDepth) return false;

        for (int f = 0; f < 6; ++f) {
            Cube::Face face = static_cast<Cube::Face>(f);
            if (!allowedMove(face, prevFace, hasPrev)) continue;
            for (int turns = 1; turns <= 3; ++turns) {
                current.move(face, turns);
                path.push_back({face, turns});
                if (dfsRecurse(current, depth + 1, maxDepth, face, true, path, nodesVisited))
                    return true;
                path.pop_back();
                current.move(face, 4 - turns); // undo: inverse of turns 1/2/3 is 3/2/1
            }
        }
        return false;
    }
}

std::string moveToString(const Move& m) {
    static const char* names[6] = {"U", "D", "L", "R", "F", "B"};
    std::string s = names[m.face];
    if (m.turns == 2) s += "2";
    else if (m.turns == 3) s += "'";
    return s;
}

ScrambleResult generateScramble(int length) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> faceDist(0, 5);
    std::uniform_int_distribution<int> turnDist(1, 3);

    Cube c;
    std::vector<Move> moves;
    Cube::Face prevFace = Cube::U;
    bool hasPrev = false;

    while (static_cast<int>(moves.size()) < length) {
        Cube::Face face = static_cast<Cube::Face>(faceDist(rng));
        if (!allowedMove(face, prevFace, hasPrev)) continue;
        int turns = turnDist(rng);
        c.move(face, turns);
        moves.push_back({face, turns});
        prevFace = face;
        hasPrev = true;
    }
    return {c, moves};
}

std::optional<std::vector<Move>> solveDFS(Cube start, int maxDepth, long long& nodesVisited) {
    nodesVisited = 0;
    std::vector<Move> path;
    if (dfsRecurse(start, 0, maxDepth, Cube::U, false, path, nodesVisited))
        return path;
    return std::nullopt;
}

std::optional<std::vector<Move>> solveIDDFS(Cube start, int maxDepthCap, long long& nodesVisited) {
    nodesVisited = 0;
    for (int depth = 0; depth <= maxDepthCap; ++depth) {
        Cube current = start;
        std::vector<Move> path;
        long long nodesThisDepth = 0;
        if (dfsRecurse(current, 0, depth, Cube::U, false, path, nodesThisDepth)) {
            nodesVisited += nodesThisDepth;
            return path;
        }
        nodesVisited += nodesThisDepth;
    }
    return std::nullopt;
}

std::optional<std::vector<Move>> solveBFS(Cube start, int maxDepth, long long& nodesVisited, long long nodeLimit) {
    nodesVisited = 0;
    if (start.isSolved()) return std::vector<Move>{};

    std::vector<BFSNode> nodes;
    nodes.push_back({start, -1, Cube::U, 0});

    std::queue<std::pair<int, int>> q; // (nodeIndex, depth)
    q.push({0, 0});

    while (!q.empty()) {
        auto [idx, depth] = q.front();
        q.pop();
        ++nodesVisited;

        if (static_cast<long long>(nodes.size()) > nodeLimit) {
            return std::nullopt; // hit the memory/node cap before finding a solution
        }
        if (depth == maxDepth) continue;

        Cube::Face prevFace = nodes[idx].face;
        bool hasPrev = (nodes[idx].parent != -1);

        for (int f = 0; f < 6; ++f) {
            Cube::Face face = static_cast<Cube::Face>(f);
            if (!allowedMove(face, prevFace, hasPrev)) continue;
            for (int turns = 1; turns <= 3; ++turns) {
                Cube next = nodes[idx].state;
                next.move(face, turns);
                nodes.push_back({next, idx, face, turns});
                int newIdx = static_cast<int>(nodes.size()) - 1;

                if (next.isSolved()) {
                    std::vector<Move> path;
                    int cur = newIdx;
                    while (nodes[cur].parent != -1) {
                        path.push_back({nodes[cur].face, nodes[cur].turns});
                        cur = nodes[cur].parent;
                    }
                    std::reverse(path.begin(), path.end());
                    return path;
                }
                if (static_cast<long long>(nodes.size()) > nodeLimit) {
                    return std::nullopt;
                }
                q.push({newIdx, depth + 1});
            }
        }
    }
    return std::nullopt;
}