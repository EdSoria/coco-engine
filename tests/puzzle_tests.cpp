// TODO: add Catch2 test cases for board generation, validation and solving.
#include "puzzle/goals.hpp"
#include "puzzle/metrics.hpp"
#include "puzzle/random_board.hpp"
#include "puzzle/solvability.hpp"
#include "puzzle/solver.hpp"

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

TEST_CASE("solver reports an already solved board") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::SolveResult result = puzzle::solve(goal, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
    REQUIRE(result.solvable);
    REQUIRE(result.moves.empty());
    REQUIRE(result.states == std::vector<puzzle::Board>{goal});
    REQUIRE(result.stats.nodes_expanded == 0);
}

TEST_CASE("solver returns the optimal one-move solution") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 7, 0, 8});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
    REQUIRE(result.solvable);
    REQUIRE(result.moves == std::vector<puzzle::Move>{puzzle::Move::Right});
    REQUIRE(result.states.size() == result.moves.size() + 1);
    REQUIRE(result.states.front() == initial);
    REQUIRE(result.states.back() == goal);
    REQUIRE(result.stats.path_length == 1);
}

TEST_CASE("solver reports an unsolvable board without searching") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 8, 7, 0});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Unsolvable);
    REQUIRE_FALSE(result.solvable);
    REQUIRE(result.moves.empty());
    REQUIRE(result.states.empty());
    REQUIRE(result.stats.nodes_expanded == 0);
}

TEST_CASE("solver reports the node limit before finding a solution") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 0, 7, 8});
    const puzzle::SolveResult result = puzzle::solve(initial, goal, 0);

    REQUIRE(result.status == puzzle::SolveStatus::NodeLimitReached);
    REQUIRE_FALSE(result.solvable);
    REQUIRE(result.moves.empty());
    REQUIRE(result.states.empty());
}

TEST_CASE("solver supports a custom goal") {
    const puzzle::Board goal(2, {0, 1, 2, 3});
    const puzzle::Board initial(2, {1, 0, 2, 3});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
    REQUIRE(result.solvable);
    REQUIRE(result.moves == std::vector<puzzle::Move>{puzzle::Move::Left});
    REQUIRE(result.states.back() == goal);
}

TEST_CASE("solver preserves the full path for multiple moves") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 0, 7, 8});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
    REQUIRE(result.moves == std::vector<puzzle::Move>{puzzle::Move::Right, puzzle::Move::Right});
    REQUIRE(result.states.size() == 3);
    REQUIRE(result.states.front() == initial);
    REQUIRE(result.states.back() == goal);
    REQUIRE(result.stats.path_length == 2);
}
