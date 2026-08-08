#include "cube.h"
#include "bitcube.h"
#include "bitcube2.h"
#include "search.h"
#include "pdb.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <utility>

using Clock = std::chrono::steady_clock;

double elapsedMs(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::string movesToString(const std::vector<Move>& moves) {
    std::string s;
    for (size_t i = 0; i < moves.size(); ++i) {
        s += moveToString(moves[i]);
        if (i + 1 < moves.size()) s += " ";
    }
    return s;
}

struct AlgoResult {
    bool solved;
    double ms;
    long long nodes;
    int solutionLen;
    bool verified;
};

AlgoResult verify(const Cube& scrambled, bool solved, double ms, long long nodes,
                   const std::optional<std::vector<Move>>& result) {
    if (!solved) return {false, ms, nodes, -1, false};
    Cube check = scrambled;
    for (auto& m : *result) check.move(m.face, m.turns);
    return {true, ms, nodes, static_cast<int>(result->size()), check.isSolved()};
}

AlgoResult runDFS(const Cube& c, int maxDepth) {
    long long nodes = 0;
    auto start = Clock::now();
    auto result = solveDFS(c, maxDepth, nodes);
    double ms = elapsedMs(start, Clock::now());
    return verify(c, result.has_value(), ms, nodes, result);
}

AlgoResult runIDDFS(const Cube& c, int maxDepth) {
    long long nodes = 0;
    auto start = Clock::now();
    auto result = solveIDDFS(c, maxDepth, nodes);
    double ms = elapsedMs(start, Clock::now());
    return verify(c, result.has_value(), ms, nodes, result);
}

AlgoResult runBFS(const Cube& c, int maxDepth, long long nodeLimit) {
    long long nodes = 0;
    auto start = Clock::now();
    auto result = solveBFS(c, maxDepth, nodes, nodeLimit);
    double ms = elapsedMs(start, Clock::now());
    return verify(c, result.has_value(), ms, nodes, result);
}

AlgoResult runIDAStar(const Cube& c, const std::vector<uint8_t>& pdb, long long nodeLimit) {
    long long nodes = 0;
    auto start = Clock::now();
    auto result = solveIDAStar(c, pdb, nodes, nodeLimit);
    double ms = elapsedMs(start, Clock::now());
    return verify(c, result.has_value(), ms, nodes, result);
}

void summarize(std::ostream& out, const std::string& name, const std::vector<double>& times, int trials) {
    out << name << ": " << times.size() << "/" << trials << " solved.";
    if (!times.empty()) {
        double sum = 0, mx = 0;
        for (double v : times) { sum += v; if (v > mx) mx = v; }
        out << " avg " << std::fixed << std::setprecision(1) << (sum / times.size())
            << " ms, max " << mx << " ms.";
    }
    out << "\n";
}

void logAlgo(std::ofstream& log, const std::string& name, const AlgoResult& r) {
    if (!r.solved) {
        log << "  " << name << ": FAILED/hit limit, " << std::fixed << std::setprecision(1)
            << r.ms << " ms, " << r.nodes << " nodes\n";
        return;
    }
    log << "  " << name << ": solved, " << std::fixed << std::setprecision(1) << r.ms
        << " ms, " << r.nodes << " nodes, " << r.solutionLen << " moves, verify: "
        << (r.verified ? "OK" : "*** BROKEN ***") << "\n";
}

void runFullBenchmark(const std::string& outPath) {
    std::vector<uint8_t> pdb = getCornerPDB("../corner_pdb.bin");

    std::ofstream log(outPath);
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    log << "Rubik's Cube Solver -- benchmark log\n";
    log << "Generated: " << std::ctime(&t);
    log << "==========================================\n\n";

    const int NAIVE_TRIALS = 5;
    const int IDA_TRIALS_8 = 20;
    const int IDA_TRIALS_13 = 10;
    const long long BFS_NODE_LIMIT = 5000000;
    const long long IDA_NODE_LIMIT = 2000000000LL;

    log << "--- Naive search baseline: BFS / DFS / IDDFS, 8-move scrambles ---\n";
    std::vector<double> dfsT, iddfsT, bfsT;
    for (int i = 1; i <= NAIVE_TRIALS; ++i) {
        ScrambleResult sc = generateScramble(8);
        std::cout << "Naive baseline trial " << i << "/" << NAIVE_TRIALS << "...\n";
        log << "Scramble " << i << ": " << movesToString(sc.moves) << "\n";

        auto d = runDFS(sc.cube, 8);
        logAlgo(log, "DFS  ", d);
        if (d.solved && d.verified) dfsT.push_back(d.ms);

        auto id = runIDDFS(sc.cube, 8);
        logAlgo(log, "IDDFS", id);
        if (id.solved && id.verified) iddfsT.push_back(id.ms);

        auto b = runBFS(sc.cube, 8, BFS_NODE_LIMIT);
        logAlgo(log, "BFS  ", b);
        if (b.solved && b.verified) bfsT.push_back(b.ms);
    }
    log << "\n";
    summarize(log, "DFS  ", dfsT, NAIVE_TRIALS);
    summarize(log, "IDDFS", iddfsT, NAIVE_TRIALS);
    summarize(log, "BFS  ", bfsT, NAIVE_TRIALS);

    struct DepthConfig { int depth; int trials; };
    std::vector<DepthConfig> configs = {{8, IDA_TRIALS_8}, {13, IDA_TRIALS_13}};

    for (auto& cfg : configs) {
        log << "\n--- IDA* with corner PDB, " << cfg.depth << "-move scrambles (" << cfg.trials << " trials) ---\n";
        std::vector<double> times;
        int broken = 0;
        for (int i = 1; i <= cfg.trials; ++i) {
            ScrambleResult sc = generateScramble(cfg.depth);
            std::cout << "  Starting IDA* " << cfg.depth << "-move trial " << i << "/" << cfg.trials << "...\n";
            auto r = runIDAStar(sc.cube, pdb, IDA_NODE_LIMIT);

            log << "  trial " << i << ":";
            if (r.solved) {
                log << " solved, " << std::fixed << std::setprecision(1) << r.ms << " ms, "
                    << r.nodes << " nodes, " << r.solutionLen << " moves, verify: "
                    << (r.verified ? "OK" : "*** BROKEN ***") << "\n";
                std::cout << "  IDA* " << cfg.depth << "-move trial " << i << "/" << cfg.trials
                          << ": " << r.ms << " ms, verify " << (r.verified ? "OK" : "BROKEN") << "\n";
                if (r.verified) times.push_back(r.ms);
                else ++broken;
            } else {
                log << " HIT NODE CAP without finding a solution, " << r.ms << " ms elapsed\n";
                std::cout << "  IDA* " << cfg.depth << "-move trial " << i << "/" << cfg.trials
                          << ": hit node cap after " << r.ms << " ms\n";
            }
        }
        log << "\n";
        summarize(log, "IDA* (" + std::to_string(cfg.depth) + "-move)", times, cfg.trials);
        if (broken > 0) {
            log << "  WARNING: " << broken << " trial(s) returned an unverifiable solution -- do not trust this data.\n";
        }
    }

    log.close();
    std::cout << "\nFull benchmark log written to " << outPath << "\n";
}

void runPhase7() {
    std::cout << "\n=== Phase 7: BitCube correctness + raw throughput ===\n\n";

    const int CORRECTNESS_TRIALS = 200;
    const int SCRAMBLE_LEN = 20;
    bool allMatch = true;

    for (int t = 0; t < CORRECTNESS_TRIALS && allMatch; ++t) {
        ScrambleResult sc = generateScramble(SCRAMBLE_LEN);
        Cube refCube;
        BitCube testCube;

        for (const auto& mv : sc.moves) {
            refCube.move(mv.face, mv.turns);
            testCube.move(static_cast<BitCube::Face>(static_cast<int>(mv.face)), mv.turns);

            if (refCube.cornerPerm() != testCube.cornerPerm() ||
                refCube.cornerOrient() != testCube.cornerOrient() ||
                refCube.edgePerm() != testCube.edgePerm() ||
                refCube.edgeOrient() != testCube.edgeOrient() ||
                refCube.isSolved() != testCube.isSolved()) {
                allMatch = false;
                std::cout << "  MISMATCH on trial " << t << " after move "
                          << moveToString(mv) << " -- BitCube diverged from Cube.\n";
                break;
            }
        }
    }

    std::cout << "Correctness (" << CORRECTNESS_TRIALS << " scrambles x " << SCRAMBLE_LEN
              << " moves, checked after every single move): "
              << (allMatch ? "OK -- BitCube matches Cube exactly" : "FAILED") << "\n";

    if (!allMatch) {
        std::cout << "Stopping here -- do not trust throughput numbers from a broken implementation.\n";
        return;
    }

    const long long NUM_MOVES = 100000000;

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> faceDist(0, 5);
    std::uniform_int_distribution<int> turnDist(1, 3);
    std::vector<std::pair<int, int>> sequence;
    sequence.reserve(NUM_MOVES);
    for (long long i = 0; i < NUM_MOVES; ++i) {
        sequence.push_back({faceDist(rng), turnDist(rng)});
    }

    Cube arrCube;
    auto arrStart = Clock::now();
    for (auto& mv : sequence) arrCube.move(static_cast<Cube::Face>(mv.first), mv.second);
    double arrMs = elapsedMs(arrStart, Clock::now());

    BitCube bitCubeBench;
    auto bitStart = Clock::now();
    for (auto& mv : sequence) bitCubeBench.move(static_cast<BitCube::Face>(mv.first), mv.second);
    double bitMs = elapsedMs(bitStart, Clock::now());

    std::cout << "\n" << NUM_MOVES << " single quarter-turns applied to each representation:\n";
    std::cout << "  Cube (array-based):   " << std::fixed << std::setprecision(1) << arrMs
              << " ms (" << std::setprecision(2) << (arrMs * 1e6 / NUM_MOVES) << " ns/move)\n";
    std::cout << "  BitCube (bit-packed): " << std::setprecision(1) << bitMs
              << " ms (" << std::setprecision(2) << (bitMs * 1e6 / NUM_MOVES) << " ns/move)\n";
    std::cout << "  Speedup: " << (arrMs / bitMs) << "x\n";
}

void runPhase7b() {
    std::cout << "\n=== Phase 7b: BitCubeV2 (table-lookup moves) correctness + throughput ===\n\n";

    const int CORRECTNESS_TRIALS = 200;
    const int SCRAMBLE_LEN = 20;
    bool allMatch = true;

    for (int t = 0; t < CORRECTNESS_TRIALS && allMatch; ++t) {
        ScrambleResult sc = generateScramble(SCRAMBLE_LEN);
        Cube refCube;
        BitCubeV2 testCube;

        for (const auto& mv : sc.moves) {
            refCube.move(mv.face, mv.turns);
            testCube.move(static_cast<BitCubeV2::Face>(static_cast<int>(mv.face)), mv.turns);

            if (refCube.cornerPerm() != testCube.cornerPerm() ||
                refCube.cornerOrient() != testCube.cornerOrient() ||
                refCube.edgePerm() != testCube.edgePerm() ||
                refCube.edgeOrient() != testCube.edgeOrient() ||
                refCube.isSolved() != testCube.isSolved()) {
                allMatch = false;
                std::cout << "  MISMATCH on trial " << t << " after move "
                          << moveToString(mv) << " -- BitCubeV2 diverged from Cube.\n";
                break;
            }
        }
    }

    std::cout << "Correctness vs Cube (" << CORRECTNESS_TRIALS << " scrambles x " << SCRAMBLE_LEN
              << " moves, checked after every single move): "
              << (allMatch ? "OK -- BitCubeV2 matches Cube exactly" : "FAILED") << "\n";

    if (!allMatch) {
        std::cout << "Stopping here -- do not trust throughput numbers from a broken implementation.\n";
        return;
    }

    const long long NUM_MOVES = 100000000;

    std::mt19937 rng(12345); // same seed as Phase 7 -- identical move sequence across all three representations
    std::uniform_int_distribution<int> faceDist(0, 5);
    std::uniform_int_distribution<int> turnDist(1, 3);
    std::vector<std::pair<int, int>> sequence;
    sequence.reserve(NUM_MOVES);
    for (long long i = 0; i < NUM_MOVES; ++i) {
        sequence.push_back({faceDist(rng), turnDist(rng)});
    }

    Cube arrCube;
    auto arrStart = Clock::now();
    for (auto& mv : sequence) arrCube.move(static_cast<Cube::Face>(mv.first), mv.second);
    double arrMs = elapsedMs(arrStart, Clock::now());

    BitCube v1Cube;
    auto v1Start = Clock::now();
    for (auto& mv : sequence) v1Cube.move(static_cast<BitCube::Face>(mv.first), mv.second);
    double v1Ms = elapsedMs(v1Start, Clock::now());

    BitCubeV2 v2Cube;
    auto v2Start = Clock::now();
    for (auto& mv : sequence) v2Cube.move(static_cast<BitCubeV2::Face>(mv.first), mv.second);
    double v2Ms = elapsedMs(v2Start, Clock::now());

    std::cout << "\n" << NUM_MOVES << " single quarter-turns, same move sequence, all three representations:\n";
    std::cout << "  Cube      (array-based):        " << std::fixed << std::setprecision(1) << arrMs
              << " ms (" << std::setprecision(2) << (arrMs * 1e6 / NUM_MOVES) << " ns/move)\n";
    std::cout << "  BitCube   (bit-packed, Phase 7): " << std::setprecision(1) << v1Ms
              << " ms (" << std::setprecision(2) << (v1Ms * 1e6 / NUM_MOVES) << " ns/move)\n";
    std::cout << "  BitCubeV2 (table lookup, 7b):    " << std::setprecision(1) << v2Ms
              << " ms (" << std::setprecision(2) << (v2Ms * 1e6 / NUM_MOVES) << " ns/move)\n\n";
    std::cout << "  BitCubeV2 vs Cube:    " << std::setprecision(2) << (arrMs / v2Ms) << "x\n";
    std::cout << "  BitCubeV2 vs BitCube: " << (v1Ms / v2Ms) << "x\n";
}

int main(int argc, char** argv) {
    bool fullBenchmark = false;
    bool phase7 = false;
    bool phase7b = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--benchmark") fullBenchmark = true;
        if (arg == "--phase7") phase7 = true;
        if (arg == "--phase7b") phase7b = true;
    }

    if (phase7b) { runPhase7b(); return 0; }
    if (phase7) { runPhase7(); return 0; }

    if (fullBenchmark) {
        runFullBenchmark("../benchmark_results.txt");
        return 0;
    }

    std::vector<uint8_t> pdb = getCornerPDB("../corner_pdb.bin");

    std::cout << "Quick check (5 scrambles at 8 moves, IDA*):\n";
    for (int i = 1; i <= 5; ++i) {
        ScrambleResult sc = generateScramble(8);
        auto r = runIDAStar(sc.cube, pdb, 2000000000LL);
        std::cout << "  " << movesToString(sc.moves) << " -> "
                  << (r.solved ? "solved" : "FAILED") << ", "
                  << std::fixed << std::setprecision(1) << r.ms << " ms, "
                  << r.solutionLen << " moves, verify: "
                  << (r.verified ? "OK" : "*** BROKEN ***") << "\n";
    }
    std::cout << "\nRun with --benchmark, --phase7, or --phase7b.\n";

    return 0;
}