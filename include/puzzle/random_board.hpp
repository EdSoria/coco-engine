#pragma once

#include "puzzle/board.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace puzzle {

struct RandomBoardOptions {
    std::size_t max_attempts = 100;
    int minimum_manhattan = -1;
    int minimum_misplaced = -1;
    std::optional<std::uint32_t> seed;
};

// Creates a solvable board for goal, rejecting candidates that are too close to it.
Board make_random_board(
    const Board& goal,
    const RandomBoardOptions& options = {}
);

}  // namespace puzzle
