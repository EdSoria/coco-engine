#include "puzzle/random_board.hpp"

#include "puzzle/metrics.hpp"
#include "puzzle/solvability.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace puzzle {

namespace {

struct QualityThresholds {
    int minimum_manhattan;
    int minimum_misplaced;
};

QualityThresholds default_thresholds(int size) {
    switch (size) {
        case 2: return {4, 3};
        case 3: return {10, 6};
        case 4: return {22, 11};
        case 5: return {30, 18};
        default: throw std::invalid_argument("size must be between 2 and 5");
    }
}

std::mt19937& default_generator() {
    static std::random_device device;
    static std::mt19937 generator(device());
    return generator;
}

void swap_first_two_non_zero_tiles(std::vector<int>& tiles) {
    auto first = tiles.end();
    auto second = tiles.end();

    for (auto iterator = tiles.begin(); iterator != tiles.end(); ++iterator) {
        if (*iterator == 0) {
            continue;
        }
        if (first == tiles.end()) {
            first = iterator;
        } else {
            second = iterator;
            break;
        }
    }

    if (first == tiles.end() || second == tiles.end()) {
        throw std::logic_error("board must contain at least two non-zero tiles");
    }

    std::iter_swap(first, second);
}

}  // namespace

Board make_random_board(const Board& goal, const RandomBoardOptions& options) {
    if (!goal.is_valid()) {
        throw std::invalid_argument("goal must be a valid board");
    }
    if (options.max_attempts == 0) {
        throw std::invalid_argument("max_attempts must be greater than zero");
    }
    if (options.minimum_manhattan < -1 || options.minimum_misplaced < -1) {
        throw std::invalid_argument("quality thresholds must be non-negative or -1");
    }

    const QualityThresholds defaults = default_thresholds(goal.size());
    const int minimum_manhattan = options.minimum_manhattan >= 0
        ? options.minimum_manhattan
        : defaults.minimum_manhattan;
    const int minimum_misplaced = options.minimum_misplaced >= 0
        ? options.minimum_misplaced
        : defaults.minimum_misplaced;

    std::mt19937 seeded_generator;
    std::mt19937* generator = &default_generator();
    if (options.seed.has_value()) {
        seeded_generator.seed(*options.seed);
        generator = &seeded_generator;
    }
    std::vector<int> tiles = goal.tiles();
    const Positions goal_positions = get_positions(goal);
    const Neighbor fallback_neighbor = goal.neighbors().front();
    Board fallback = fallback_neighbor.board;

    for (std::size_t attempt = 0; attempt < options.max_attempts; ++attempt) {
        std::shuffle(tiles.begin(), tiles.end(), *generator);
        if (!is_solvable(Board(goal.size(), tiles), goal)) {
            swap_first_two_non_zero_tiles(tiles);
        }

        Board candidate(goal.size(), tiles);
        if (!is_solvable(candidate, goal)) {
            throw std::logic_error("parity correction failed");
        }
        if (candidate != goal) {
            fallback = candidate;
        }

        if (candidate != goal &&
            manhattan_distance(candidate, goal_positions) >= minimum_manhattan &&
            misplaced_tiles(candidate, goal) >= minimum_misplaced) {
            return candidate;
        }
    }

    return fallback;
}

}  // namespace puzzle
