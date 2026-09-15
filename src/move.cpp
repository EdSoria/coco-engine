#include "puzzle/move.hpp"

#include <stdexcept>

namespace puzzle {

std::string to_string(Move move) {
    switch (move) {
        case Move::Up: return "UP";
        case Move::Down: return "DOWN";
        case Move::Left: return "LEFT";
        case Move::Right: return "RIGHT";
    }
    throw std::invalid_argument("unknown move");
}

Move inverse(Move move) {
    switch (move) {
        case Move::Up: return Move::Down;
        case Move::Down: return Move::Up;
        case Move::Left: return Move::Right;
        case Move::Right: return Move::Left;
    }
    throw std::invalid_argument("unknown move");
}

}  // namespace puzzle
