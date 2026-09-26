#include "puzzle/api.hpp"
#include "puzzle/goals.hpp"
#include "puzzle/metrics.hpp"
#include "puzzle/random_board.hpp"
#include "puzzle/solvability.hpp"
#include "puzzle/solver.hpp"

#include <catch2/catch_test_macros.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <stdexcept>
#include <thread>

namespace {

using Json = nlohmann::json;

class ApiFixture {
public:
    ApiFixture() {
        puzzle::register_routes(server_);
        port_ = server_.bind_to_any_port("127.0.0.1");
        REQUIRE(port_ > 0);

        server_thread_ = std::thread([this]() {
            server_.listen_after_bind();
        });
    }

    ~ApiFixture() {
        server_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

    httplib::Client client() const {
        return httplib::Client("127.0.0.1", port_);
    }

private:
    httplib::Server server_;
    std::thread server_thread_;
    int port_ = 0;
};

httplib::Result post_json(
    httplib::Client& client,
    const std::string& path,
    const Json& body
) {
    return client.Post(path, body.dump(), "application/json");
}

}  // namespace

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
    REQUIRE(result.moves.empty());
    REQUIRE(result.states == std::vector<puzzle::Board>{goal});
    REQUIRE(result.stats.nodes_expanded == 0);
}

TEST_CASE("solver returns the optimal one-move solution") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 7, 0, 8});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
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
    REQUIRE(result.moves.empty());
    REQUIRE(result.states.empty());
    REQUIRE(result.stats.nodes_expanded == 0);
}

TEST_CASE("solver reports the node limit before finding a solution") {
    const puzzle::Board goal = puzzle::make_goal(3);
    const puzzle::Board initial(3, {1, 2, 3, 4, 5, 6, 0, 7, 8});
    const puzzle::SolveResult result = puzzle::solve(initial, goal, 0);

    REQUIRE(result.status == puzzle::SolveStatus::NodeLimitReached);
    REQUIRE(result.moves.empty());
    REQUIRE(result.states.empty());
}

TEST_CASE("solver supports a custom goal") {
    const puzzle::Board goal(2, {0, 1, 2, 3});
    const puzzle::Board initial(2, {1, 0, 2, 3});
    const puzzle::SolveResult result = puzzle::solve(initial, goal);

    REQUIRE(result.status == puzzle::SolveStatus::Solved);
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

TEST_CASE("goal endpoints return the configured goal data") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();

    const auto goal_types = client.Get("/api/puzzle/goal-types");
    REQUIRE(goal_types);
    REQUIRE(goal_types->status == 200);
    const Json goal_types_body = Json::parse(goal_types->body);
    REQUIRE(goal_types_body.at("types") == Json({
        "ordered",
        "spiral",
        "columns",
        "snake",
        "reverse"
    }));

    const auto goal = client.Get("/api/puzzle/goal?size=3&type=ordered");
    REQUIRE(goal);
    REQUIRE(goal->status == 200);
    const Json goal_body = Json::parse(goal->body);
    REQUIRE(goal_body.at("size") == 3);
    REQUIRE(goal_body.at("goal") == Json({1, 2, 3, 4, 5, 6, 7, 8, 0}));
}

TEST_CASE("new and move endpoints return valid boards") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();
    const Json goal = {1, 2, 3, 4, 5, 6, 7, 8, 0};

    const auto new_board = post_json(client, "/api/puzzle/new", Json{
        {"size", 3},
        {"goal", goal}
    });
    REQUIRE(new_board);
    REQUIRE(new_board->status == 200);
    const Json new_body = Json::parse(new_board->body);
    REQUIRE(new_body.at("size") == 3);
    REQUIRE(new_body.at("board").size() == 9);

    const auto moved = post_json(client, "/api/puzzle/move", Json{
        {"size", 3},
        {"board", {1, 2, 3, 4, 5, 6, 7, 0, 8}},
        {"move", "RIGHT"}
    });
    REQUIRE(moved);
    REQUIRE(moved->status == 200);
    const Json moved_body = Json::parse(moved->body);
    REQUIRE(moved_body.at("board") == goal);
}

TEST_CASE("validate and solve endpoints expose puzzle results") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();
    const Json goal = {1, 2, 3, 4, 5, 6, 7, 8, 0};
    const Json initial = {1, 2, 3, 4, 5, 6, 7, 0, 8};

    const auto validation = post_json(client, "/api/puzzle/validate", Json{
        {"size", 3},
        {"initial", initial},
        {"goal", goal}
    });
    REQUIRE(validation);
    REQUIRE(validation->status == 200);
    REQUIRE(Json::parse(validation->body).at("solvable") == true);

    const auto solution = post_json(client, "/api/puzzle/solve", Json{
        {"size", 3},
        {"initial", initial},
        {"goal", goal}
    });
    REQUIRE(solution);
    REQUIRE(solution->status == 200);
    const Json solution_body = Json::parse(solution->body);
    REQUIRE(solution_body.at("solvable") == true);
    REQUIRE(solution_body.at("moves") == Json({"RIGHT"}));
    REQUIRE(solution_body.at("states").size() == 2);
    REQUIRE(solution_body.at("stats").at("path_length") == 1);
}

TEST_CASE("HTTP endpoints reject invalid input consistently") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();

    const auto invalid_goal = client.Get("/api/puzzle/goal?size=1");
    REQUIRE(invalid_goal);
    REQUIRE(invalid_goal->status == 400);
    REQUIRE(Json::parse(invalid_goal->body).at("error") == "invalid_goal");

    const auto invalid_move = post_json(client, "/api/puzzle/move", Json{
        {"size", 3},
        {"board", {1, 2, 3}},
        {"move", "RIGHT"}
    });
    REQUIRE(invalid_move);
    REQUIRE(invalid_move->status == 400);
    REQUIRE(Json::parse(invalid_move->body).at("error") == "invalid_move");
}

TEST_CASE("HTTP endpoints support local browser CORS") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();

    httplib::Headers preflight_headers = {
        {"Origin", "http://localhost:5173"},
        {"Access-Control-Request-Method", "POST"},
        {"Access-Control-Request-Headers", "Content-Type"}
    };
    const auto preflight = client.Options(
        "/api/puzzle/solve",
        preflight_headers
    );
    REQUIRE(preflight);
    REQUIRE(preflight->status == 204);
    REQUIRE(preflight->get_header_value("Access-Control-Allow-Origin") == "http://localhost:5173");
    REQUIRE(preflight->get_header_value("Access-Control-Allow-Methods") == "GET, POST, OPTIONS");
    REQUIRE(preflight->get_header_value("Access-Control-Allow-Headers") == "Content-Type");

    httplib::Headers request_headers = {
        {"Origin", "http://localhost:5173"}
    };
    const auto response = client.Get(
        "/api/puzzle/goal-types",
        request_headers
    );
    REQUIRE(response);
    REQUIRE(response->status == 200);
    REQUIRE(response->get_header_value("Access-Control-Allow-Origin") == "http://localhost:5173");
}

TEST_CASE("HTTP endpoints do not authorize unrelated origins") {
    ApiFixture fixture;
    httplib::Client client = fixture.client();
    const httplib::Headers headers = {
        {"Origin", "https://untrusted.example"}
    };

    const auto response = client.Get("/api/puzzle/goal-types", headers);
    REQUIRE(response);
    REQUIRE(response->status == 200);
    REQUIRE(response->get_header_value("Access-Control-Allow-Origin").empty());
}
