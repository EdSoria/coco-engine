#include "puzzle/metrics.hpp"

#include <cstdlib>
#include <stdexcept>

namespace puzzle {

Positions get_positions(const Board& goal) {
    if (!goal.is_valid()) {
        throw std::invalid_argument("goal must be a valid board");
    }

    Positions result(goal.tiles().size());
    for (std::size_t index = 0; index < goal.tiles().size(); ++index) {
        result[static_cast<std::size_t>(goal.tiles()[index])] = index;
    }
    return result;
}

int manhattan_distance(const Board& board, const Positions& goal_positions) {
    if (!board.is_valid() || goal_positions.size() != board.tiles().size()) {
        throw std::invalid_argument("board and positions must have matching valid dimensions");
    }

    int distance = 0;
    for (std::size_t index = 0; index < board.tiles().size(); ++index) {
        const int tile = board.tiles()[index];
        if (tile == 0) {
            continue;
        }

        const std::size_t goal_index = goal_positions[static_cast<std::size_t>(tile)];
        distance += std::abs(
            static_cast<int>(index / static_cast<std::size_t>(board.size())) -
            static_cast<int>(goal_index / static_cast<std::size_t>(board.size()))
        );
        distance += std::abs(
            static_cast<int>(index % static_cast<std::size_t>(board.size())) -
            static_cast<int>(goal_index % static_cast<std::size_t>(board.size()))
        );
    }
    return distance;
}

int manhattan_distance(const Board& board, const Board& goal) {
    if (!goal.is_valid() || board.size() != goal.size()) {
        throw std::invalid_argument("board and goal must have matching valid dimensions");
    }
    return manhattan_distance(board, get_positions(goal));
}

int misplaced_tiles(const Board& board, const Board& goal) {
    if (!board.is_valid() || !goal.is_valid() || board.size() != goal.size()) {
        throw std::invalid_argument("board and goal must have matching valid dimensions");
    }

    int misplaced = 0;
    for (std::size_t index = 0; index < board.tiles().size(); ++index) {
        if (board.tiles()[index] != 0 && board.tiles()[index] != goal.tiles()[index]) {
            ++misplaced;
        }
    }
    return misplaced;
}

}  // namespace puzzle
