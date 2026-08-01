#include "cube.h"
#include "search.h"
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

void summarize(const char* name, const std::vector<double>& times, int trials) {
    std::cout << name << ": " << times.size() << "/" << trials << " solved.";
    if (!times.empty()) {
        double sum = 0, mx = 0;
        for (double v : times) { sum += v; if (v > mx) mx = v; }
        std::cout << " avg " << std::fixed << std::setprecision(1) << (sum / times.size())
                  << " ms, max " << mx << " ms.";
    }
    std::cout << "\n";
}

int main() {
    const int SCRAMBLE_LEN = 8;
    const int TRIALS = 5;
    const long long BFS_NODE_LIMIT = 5000000; // roughly ~850 MB worth of nodes

    std::cout << "Benchmarking BFS / DFS / IDDFS on " << TRIALS
              << " random " << SCRAMBLE_LEN << "-move scrambles.\n\n";

    std::vector<double> dfsTimes, iddfsTimes, bfsTimes;

    for (int t = 1; t <= TRIALS; ++t) {
        ScrambleResult sc = generateScramble(SCRAMBLE_LEN);
        std::cout << "Scramble " << t << "/" << TRIALS << ": ";
        printMoves(sc.moves);
        std::cout << "\n";

        {
            long long nodes = 0;
            auto start = Clock::now();
            auto result = solveDFS(sc.cube, SCRAMBLE_LEN, nodes);
            double ms = elapsedMs(start, Clock::now());
            if (result) {
                dfsTimes.push_back(ms);
                Cube check = sc.cube;
                for (auto& m : *result) check.move(m.face, m.turns);
                std::cout << "  DFS:    " << std::fixed << std::setprecision(1) << ms
                          << " ms, " << nodes << " nodes, " << result->size()
                          << " moves, verify: " << (check.isSolved() ? "OK" : "BROKEN") << "\n";
            } else {
                std::cout << "  DFS:    no solution within depth " << SCRAMBLE_LEN
                          << " (" << nodes << " nodes, " << ms << " ms)\n";
            }
        }
        {
            long long nodes = 0;
            auto start = Clock::now();
            auto result = solveIDDFS(sc.cube, SCRAMBLE_LEN, nodes);
            double ms = elapsedMs(start, Clock::now());
            if (result) {
                iddfsTimes.push_back(ms);
                Cube check = sc.cube;
                for (auto& m : *result) check.move(m.face, m.turns);
                std::cout << "  IDDFS:  " << std::fixed << std::setprecision(1) << ms
                          << " ms, " << nodes << " nodes, " << result->size()
                          << " moves, verify: " << (check.isSolved() ? "OK" : "BROKEN") << "\n";
            } else {
                std::cout << "  IDDFS:  no solution within depth " << SCRAMBLE_LEN
                          << " (" << nodes << " nodes, " << ms << " ms)\n";
            }
        }
        {
            long long nodes = 0;
            auto start = Clock::now();
            auto result = solveBFS(sc.cube, SCRAMBLE_LEN, nodes, BFS_NODE_LIMIT);
            double ms = elapsedMs(start, Clock::now());
            if (result) {
                bfsTimes.push_back(ms);
                Cube check = sc.cube;
                for (auto& m : *result) check.move(m.face, m.turns);
                std::cout << "  BFS:    " << std::fixed << std::setprecision(1) << ms
                          << " ms, " << nodes << " nodes, " << result->size()
                          << " moves, verify: " << (check.isSolved() ? "OK" : "BROKEN") << "\n";
            } else {
                std::cout << "  BFS:    hit node limit (" << BFS_NODE_LIMIT
                          << ") before finding a solution (" << ms << " ms)\n";
            }
        }
        std::cout << "\n";
    }

    std::cout << "=== Summary ===\n";
    summarize("DFS  ", dfsTimes, TRIALS);
    summarize("IDDFS", iddfsTimes, TRIALS);
    summarize("BFS  ", bfsTimes, TRIALS);

    return 0;
}