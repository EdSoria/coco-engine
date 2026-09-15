#include "puzzle/solvability.hpp"

#include <vector>

namespace puzzle {

bool is_solvable(const Board& initial, const Board& goal) {
    if (!initial.is_valid() || !goal.is_valid() || initial.size() != goal.size()) {
        return false;
    }

    const int total = initial.size() * initial.size();
    std::vector<int> goal_positions(static_cast<std::size_t>(total));
    for (int index = 0; index < total; ++index) {
        goal_positions[static_cast<std::size_t>(goal.tiles()[static_cast<std::size_t>(index)])] = index;
    }

    std::vector<int> ordered_tiles;
    ordered_tiles.reserve(static_cast<std::size_t>(total - 1));
    for (const int tile : initial.tiles()) {
        if (tile != 0) {
            ordered_tiles.push_back(goal_positions[static_cast<std::size_t>(tile)]);
        }
    }

    int inversions = 0;
    for (std::size_t left = 0; left < ordered_tiles.size(); ++left) {
        for (std::size_t right = left + 1; right < ordered_tiles.size(); ++right) {
            if (ordered_tiles[left] > ordered_tiles[right]) {
                ++inversions;
            }
        }
    }

    if (initial.size() % 2 == 1) {
        return inversions % 2 == 0;
    }

    const auto blank_row_from_bottom = [](const Board& board) {
        return board.size() - static_cast<int>(board.blank_index()) / board.size();
    };

    return (inversions + blank_row_from_bottom(initial)) % 2 ==
        blank_row_from_bottom(goal) % 2;
}

}  // namespace puzzle
