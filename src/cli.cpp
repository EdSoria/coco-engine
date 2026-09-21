#include "puzzle/board.hpp"
#include "puzzle/move.hpp"
#include "puzzle/solver.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

void print_board(const puzzle::Board& board) {
    const int size = board.size();
    const auto& tiles = board.tiles();

    for (int row = 0; row < size; ++row) {
        for (int column = 0; column < size; ++column) {
            std::cout << tiles[static_cast<std::size_t>(row * size + column)];
            if (column + 1 < size) {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }
}

void print_states(const std::vector<puzzle::Board>& states) {
    for (std::size_t index = 0; index < states.size(); ++index) {
        std::cout << "State " << index << ":\n";
        print_board(states[index]);
    }
}

}  // namespace

int main() {
    try {
        int size = 0;
        std::cout << "Board size (2-5): ";
        if (!(std::cin >> size)) {
            std::cerr << "Invalid board size.\n";
            return 1;
        }

        const std::size_t tile_count = static_cast<std::size_t>(size * size);
        std::vector<int> initial_tiles(tile_count);
        std::vector<int> goal_tiles(tile_count);

        std::cout << "Initial board (" << tile_count << " numbers): ";
        for (int& tile : initial_tiles) {
            if (!(std::cin >> tile)) {
                std::cerr << "Invalid initial board.\n";
                return 1;
            }
        }

        std::cout << "Goal board (" << tile_count << " numbers): ";
        for (int& tile : goal_tiles) {
            if (!(std::cin >> tile)) {
                std::cerr << "Invalid goal board.\n";
                return 1;
            }
        }

        const puzzle::Board initial(size, std::move(initial_tiles));
        const puzzle::Board goal(size, std::move(goal_tiles));
        const puzzle::SolveResult result = puzzle::solve(initial, goal);

        switch (result.status) {
            case puzzle::SolveStatus::Solved:
                std::cout << "Status: solved\n";
                break;
            case puzzle::SolveStatus::Unsolvable:
                std::cout << "Status: unsolvable\n";
                break;
            case puzzle::SolveStatus::NodeLimitReached:
                std::cout << "Status: node limit reached\n";
                break;
        }

        std::cout << "Moves (" << result.moves.size() << "):";
        for (const puzzle::Move move : result.moves) {
            std::cout << ' ' << puzzle::to_string(move);
        }
        std::cout << "\n\n";

        std::cout << "Expanded nodes: " << result.stats.nodes_expanded << '\n';
        std::cout << "Elapsed time (ms): " << result.stats.elapsed_ms << '\n';
        std::cout << "Path length: " << result.stats.path_length << "\n\n";

        if (!result.states.empty()) {
            print_states(result.states);
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
