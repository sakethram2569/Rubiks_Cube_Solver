#include "cube.h"
#include "search.h"
#include "pdb.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>

using Clock = std::chrono::steady_clock;

double elapsedMs(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void printMoves(const std::vector<Move>& moves) {
    for (size_t i = 0; i < moves.size(); ++i) {
        std::cout << moveToString(moves[i]);
        if (i + 1 < moves.size()) std::cout << " ";
    }
}

void runBenchmark(const std::vector<uint8_t>& pdb, int scrambleLen, int trials) {
    std::cout << "\n=== " << scrambleLen << "-move scrambles (" << trials << " trials) ===\n";
    std::vector<double> times;

    for (int t = 1; t <= trials; ++t) {
        ScrambleResult sc = generateScramble(scrambleLen);
        std::cout << "Scramble " << t << "/" << trials << ": ";
        printMoves(sc.moves);
        std::cout << "\n";

        long long nodes = 0;
        auto start = Clock::now();
        auto result = solveIDAStar(sc.cube, pdb, nodes);
        double ms = elapsedMs(start, Clock::now());

        if (result) {
            times.push_back(ms);
            Cube check = sc.cube;
            for (auto& m : *result) check.move(m.face, m.turns);
            std::cout << "  IDA*: " << std::fixed << std::setprecision(1) << ms
                      << " ms, " << nodes << " nodes, " << result->size()
                      << " moves, verify: " << (check.isSolved() ? "OK" : "BROKEN") << "\n";
        } else {
            std::cout << "  IDA*: no solution found (" << ms << " ms, " << nodes << " nodes)\n";
        }
    }

    if (!times.empty()) {
        double sum = 0, mx = 0;
        for (double v : times) { sum += v; if (v > mx) mx = v; }
        std::cout << scrambleLen << "-move summary: " << times.size() << "/" << trials
                  << " solved. avg " << std::fixed << std::setprecision(1) << (sum / times.size())
                  << " ms, max " << mx << " ms.\n";
    }
}

int main() {
    std::vector<uint8_t> pdb = getCornerPDB("../corner_pdb.bin");

    runBenchmark(pdb, 8, 5);
    runBenchmark(pdb, 13, 5);

    return 0;
}