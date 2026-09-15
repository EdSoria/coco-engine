#pragma once

#include <vector>

namespace puzzle {

class Board {
public:
    Board(int size, std::vector<int> tiles);

    int size() const;
    const std::vector<int>& tiles() const;

private:
    int size_;
    std::vector<int> tiles_;
};

// TODO: declare board validation, moves, solvability and random generation.

}  // namespace puzzle
