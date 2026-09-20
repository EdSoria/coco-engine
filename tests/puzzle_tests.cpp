// TODO: add Catch2 test cases for board generation, validation and solving.
#include "puzzle/goals.hpp"
#include "puzzle/metrics.hpp"
#include "puzzle/random_board.hpp"
#include "puzzle/solvability.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <stdexcept>

TEST_CASE("random boards are valid and solvable for every supported size") {
    for (int size = 2; size <= 5; ++size) {
        const puzzle::Board goal = puzzle::make_goal(size);
        puzzle::RandomBoardOptions options;
        options.seed = static_cast<std::uint32_t>(size);

        const puzzle::Board board = puzzle::make_random_board(goal, options);

        REQUIRE(board.is_valid());
        REQUIRE(puzzle::is_solvable(board, goal));
        REQUIRE(board != goal);
    }
}

TEST_CASE("random board generation is reproducible with a seed") {
    const puzzle::Board goal = puzzle::make_goal(3);
    puzzle::RandomBoardOptions first_options;
    first_options.seed = 12345;
    puzzle::RandomBoardOptions second_options;
    second_options.seed = 12345;

    const puzzle::Board first = puzzle::make_random_board(goal, first_options);
    const puzzle::Board second = puzzle::make_random_board(goal, second_options);

    REQUIRE(first == second);
}

TEST_CASE("random board generation rejects invalid options") {
    const puzzle::Board goal = puzzle::make_goal(3);
    puzzle::RandomBoardOptions options;
    options.max_attempts = 0;

    REQUIRE_THROWS_AS(
        puzzle::make_random_board(goal, options),
        std::invalid_argument
    );
}

TEST_CASE("board metrics work with custom goals") {
    const puzzle::Board goal(3, {0, 1, 2, 3, 4, 5, 6, 7, 8});
    const puzzle::Board board(3, {1, 0, 2, 3, 4, 5, 6, 7, 8});
    const puzzle::Positions goal_positions = puzzle::get_positions(goal);

    REQUIRE(puzzle::manhattan_distance(goal, goal_positions) == 0);
    REQUIRE(puzzle::manhattan_distance(board, goal_positions) == 1);
    REQUIRE(puzzle::manhattan_distance(board, goal) == 1);
    REQUIRE(puzzle::misplaced_tiles(board, goal) == 1);
}
