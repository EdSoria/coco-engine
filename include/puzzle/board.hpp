#pragma once

#include "puzzle/move.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace puzzle {

struct Neighbor;

class Board {
public:
    Board(int size, std::vector<int> tiles);

    int size() const noexcept;
    const std::vector<int>& tiles() const noexcept;
    bool is_valid() const;

    std::size_t blank_index() const;
    Board moved(Move move) const;
    std::vector<Neighbor> neighbors() const;

    std::string key() const;
    bool operator==(const Board& other) const noexcept;
    bool operator!=(const Board& other) const noexcept;

private:
    int size_;
    std::vector<int> tiles_;
};

struct Neighbor {
    Move move;
    Board board;
};

}  // namespace puzzle
