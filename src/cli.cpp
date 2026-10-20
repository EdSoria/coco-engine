#include "puzzle/board.hpp"
#include "puzzle/move.hpp"
#include "puzzle/solver.hpp"

#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
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

int main(int argc, char* argv[]) {
    try {
        std::size_t max_nodes = 250000;
        if (argc > 2) {
            std::cerr << "Usage: puzzle_cli.exe [max_nodes]\n";
            return 1;
        }
        if (argc == 2) {
            std::size_t parsed_length = 0;
            try {
                const unsigned long long parsed = std::stoull(argv[1], &parsed_length);
                if (parsed_length != std::string(argv[1]).size()) {
                    throw std::invalid_argument("invalid node limit");
                }
                if (parsed > std::numeric_limits<std::size_t>::max()) {
                    throw std::invalid_argument("node limit is too large");
                }
                max_nodes = static_cast<std::size_t>(parsed);
            } catch (const std::exception&) {
                std::cerr << "Invalid max_nodes. Use a non-negative integer.\n";
                return 1;
            }
        }

        int size = 0;
        std::cout << "Board size (2-5): ";
        if (!(std::cin >> size)) {
            std::cerr << "Invalid board size.\n";
            return 1;
        }
        if (size < 2 || size > 5) {
            std::cerr << "Invalid board size. Choose a size between 2 and 5.\n";
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
        const puzzle::SolveResult result = puzzle::solve(initial, goal, max_nodes);

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
