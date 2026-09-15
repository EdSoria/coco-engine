#pragma once

#include "puzzle/board.hpp"

namespace puzzle {

// Checks whether initial can reach goal using legal moves.
bool is_solvable(const Board& initial, const Board& goal);

}  // namespace puzzle
