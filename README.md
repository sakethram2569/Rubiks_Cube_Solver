# Rubik's Cube Solver

A 3x3 Rubik's Cube solver built from scratch in C++, implementing BFS, DFS,
IDDFS, and Korf's IDA* algorithm with an admissible corner pattern database
heuristic.

## Algorithms implemented

- **State representation**: cubie-level model — permutation + orientation
  arrays for 8 corners and 12 edges — rather than facelet-level. This is
  what makes pattern databases tractable (see below).
- **Legal move generation**: all 18 face turns (6 faces x {CW, 180, CCW}),
  with move pruning (no immediate same-face repeat, canonical ordering for
  commuting opposite-face pairs) to cut the effective branching factor.
- **Naive search**: breadth-first search, depth-limited depth-first search,
  and iterative deepening DFS.
- **Corner pattern database**: full breadth-first traversal of the
  88,179,840-state corner-only subgroup (8! permutations x 3^7 orientations),
  computed once and cached to disk.
- **IDA\***: iterative-deepening A* search using the corner PDB as an
  admissible heuristic (f = g + h), guaranteeing shortest solutions.

## Build

```bash
mkdir build && cd build
cmake .. -G "Ninja"
cmake --build .
./cube_solver             # quick 5-scramble smoke test
./cube_solver --benchmark # full evidence-generating benchmark suite
```

The corner pattern database (`corner_pdb.bin`, ~88MB) is built automatically
on first run and cached at the project root — every run after that loads it
in under a second instead of rebuilding.

## Why cubie-level, not facelet-level

A facelet-level model (54 stickers) makes illegal states easy to represent
by accident and doesn't naturally decompose into independent subproblems.
The cubie-level model — tracking *which* piece occupies each of 8 corner
slots and 12 edge slots, plus each piece's orientation — makes the group
structure explicit. That's what lets Phase 4 relax the problem to "corners
only" and get a valid admissible heuristic: ignoring edges is a legal
relaxation of the cubie-level model in a way it isn't for facelets.

## Why the corner PDB is an admissible heuristic

Solving *only* the corners is strictly easier than solving the whole cube —
any full solution is also a valid corner solution, so the corner-only
distance can never exceed the true distance. That inequality
(`h(state) <= true_distance(state)`) is the definition of admissibility,
and it's what guarantees IDA* finds the shortest solution rather than just
*a* solution.

## Complexity notes

| Component | Time | Space |
|---|---|---|
| `Cube::move()` | O(1) — fixed 20-element array update | O(1) |
| Naive BFS | O(b^d) | O(b^d) — stores every node visited |
| Naive DFS / IDDFS | O(b^d) worst case | O(d) — just the current path |
| Corner PDB build | O(N), N = 88,179,840, one-time | O(N), ~88MB on disk |
| IDA\* | O(b^d) worst case, but heuristic pruning reduces the effective branching factor sharply in practice | O(d) |

where `b` is the effective branching factor after move pruning (~13-15) and
`d` is solution depth.

## Benchmark results

Measured with `--benchmark` (Release build, `-O2`), full log in
`benchmark_results.txt`.

**8-move scrambles:**

| Algorithm | Solved | Avg time | Max time | Avg nodes |
|---|---|---|---|---|
| DFS (naive) | 5/5 | 35.6 s | 58.8 s | 382M |
| IDDFS (naive) | 5/5 | 46.5 s | 68.1 s | 502M |
| BFS (naive) | 0/5 | — (hit 5M node cap every time) | — | 374,574 |
| **IDA\* + corner PDB** | **20/20** | **5.0 ms** | **27.3 ms** | **~30K** |

**13-move scrambles:**

| Algorithm | Solved | Avg time | Max time | Avg nodes |
|---|---|---|---|---|
| **IDA\* + corner PDB** | **10/10** | **86.1 s** | **271.9 s** | **~425M** |

At 8 moves, the corner heuristic cuts node exploration by roughly **12,700x**
versus naive DFS (382M -> ~30K nodes), a ~7,100x wall-clock speedup. At 13
moves the same single-heuristic approach is far less effective — the corner
PDB is blind to the edges, and at this depth there are states where the
corners look nearly solved while the edges are still badly scrambled,
causing IDA* to explore large amounts of the tree before its threshold
climbs high enough. BFS never completes an 8-move solve within a 5M-node
cap — its memory requirement (every node in the frontier, not just the
current path) makes it impractical at this depth without a heuristic.

## Known limitations

- The single corner-PDB heuristic doesn't scale cleanly to 13-move
  scrambles — solve times there range from ~8.6s to ~272s in testing, a
  >30x spread, because the heuristic is blind to edge state entirely.
- A disjoint edge pattern database (splitting the 12 edges into groups,
  same BFS technique as the corner table) combined via `max(cornerDist,
  edgeDist)` — still provably admissible — would tighten this heuristic
  and is the natural next step if 13-move performance needs to improve.
- BFS's node cap is a practical safety limit, not a fundamental one — with
  enough RAM it would eventually solve 8-move scrambles, just not on
  typical hardware.

## Project structure

```
include/
  cube.h        -- Cube class: state, operators, move()
  movetables.h  -- shared move-delta tables (corners + edges)
  search.h      -- scramble generation, BFS/DFS/IDDFS/IDA*
  pdb.h         -- corner pattern database build/load/query
src/
  cube.cpp
  search.cpp
  pdb.cpp
  main.cpp      -- quick smoke test + --benchmark suite
```