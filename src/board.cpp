#include "puzzle/board.hpp"

#include <utility>

namespace puzzle {

Board::Board(int size, std::vector<int> tiles) : size_(size), tiles_(std::move(tiles)) {}

int Board::size() const { return size_; }
const std::vector<int>& Board::tiles() const { return tiles_; }

// TODO: implement validation, moves, solvability and random generation.

}  // namespace puzzle
