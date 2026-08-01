#ifndef PDB_H
#define PDB_H

#include "cube.h"
#include <vector>
#include <cstdint>
#include <string>

constexpr int NUM_CORNER_PERMS = 40320;   // 8!
constexpr int NUM_CORNER_ORIENTS = 2187;  // 3^7 -- the 8th corner's orientation is determined by the other 7
constexpr long long NUM_CORNER_STATES = static_cast<long long>(NUM_CORNER_PERMS) * NUM_CORNER_ORIENTS; // 88,179,840

// Builds the full corner pattern database via breadth-first search from the
// solved state, tracking corners only (edges are ignored entirely). The
// result: dist[i] = minimum number of moves to solve JUST the corners for
// the state indexed by i. Because ignoring edges is a relaxation, this is
// always <= the true full-cube solve distance -- which is what makes it a
// valid (admissible) heuristic for IDA* in Phase 5.
std::vector<uint8_t> buildCornerPDB();

bool loadCornerPDB(const std::string& path, std::vector<uint8_t>& dist);
void saveCornerPDB(const std::string& path, const std::vector<uint8_t>& dist);

// Loads the PDB from path if it exists and is the right size; otherwise
// builds it from scratch and saves it there for next time.
std::vector<uint8_t> getCornerPDB(const std::string& path);

// Looks up the corner-only distance for a given full cube state.
int cornerDistance(const std::vector<uint8_t>& pdb, const Cube& c);

#endif