#pragma once

#include "puzzle/board.hpp"

#include <cstddef>
#include <vector>

namespace puzzle {

using Positions = std::vector<std::size_t>;

Positions get_positions(const Board& goal);

int manhattan_distance(
    const Board& board,
    const Positions& goal_positions
);

int manhattan_distance(
    const Board& board,
    const Board& goal
);

int misplaced_tiles(const Board& board, const Board& goal);

}  // namespace puzzle
