#pragma once

#include "puzzle/board.hpp"

#include <cstddef>
#include <vector>

namespace puzzle {

struct SolveStats {
    std::size_t nodes_expanded = 0;
    double elapsed_ms = 0.0;
};

struct SolveResult {
    bool solvable = false;
    std::vector<Move> moves;
    std::vector<Board> states;
    SolveStats stats;
};

SolveResult solve(
    const Board& initial,
    const Board& goal,
    std::size_t max_nodes = 250000
);

}  // namespace puzzle
