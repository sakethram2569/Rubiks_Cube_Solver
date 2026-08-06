#include "cube.h"
#include "search.h"
#include "pdb.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <fstream>
#include <string>

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

int main(int argc, char** argv) {
    bool fullBenchmark = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--benchmark") fullBenchmark = true;
    }

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
    std::cout << "\nRun with --benchmark for the full evidence log used for the CV claims.\n";

    return 0;
}