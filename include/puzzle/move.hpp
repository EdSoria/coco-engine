#pragma once

#include <string>

namespace puzzle {

enum class Move {
    Up,
    Down,
    Left,
    Right
};

std::string to_string(Move move);
Move inverse(Move move);

}  // namespace puzzle
