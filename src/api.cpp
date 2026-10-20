#include "puzzle/api.hpp"

#include "puzzle/goals.hpp"
#include "puzzle/move.hpp"
#include "puzzle/random_board.hpp"
#include "puzzle/solvability.hpp"
#include "puzzle/solver.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace puzzle {

namespace {

using Json = nlohmann::json;

constexpr int minimum_size = 2;
constexpr int maximum_size = 5;

bool is_allowed_origin(const std::string& origin) {
	return origin == "null" ||
		origin == "http://localhost" ||
		origin.rfind("http://localhost:", 0) == 0 ||
		origin == "http://127.0.0.1" ||
		origin.rfind("http://127.0.0.1:", 0) == 0;
}

void set_cors_headers(
	const httplib::Request& request,
	httplib::Response& response
) {
	if (!request.has_header("Origin")) {
		return;
	}

	const std::string origin = request.get_header_value("Origin");
	if (!is_allowed_origin(origin)) {
		return;
	}

	response.set_header("Access-Control-Allow-Origin", origin);
	response.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
	response.set_header("Access-Control-Allow-Headers", "Content-Type");
	response.set_header("Access-Control-Max-Age", "600");
	response.set_header("Vary", "Origin");
}

void set_json_response(
	httplib::Response& response,
	const Json& body,
	int status = 200
) {
	response.status = status;
	response.set_content(body.dump(), "application/json");
}

void set_error_response(
	httplib::Response& response,
	const std::string& code,
	const std::string& message,
	int status = 400
) {
	set_json_response(response, Json{{"error", code}, {"message", message}}, status);
}

int parse_size(const std::string& value) {
	std::size_t parsed_length = 0;
	int size = 0;
	try {
		size = std::stoi(value, &parsed_length);
	} catch (const std::exception&) {
		throw std::invalid_argument("size must be an integer between 2 and 5");
	}

	if (parsed_length != value.size() || size < minimum_size || size > maximum_size) {
		throw std::invalid_argument("size must be an integer between 2 and 5");
	}
	return size;
}

std::size_t parse_max_nodes(const Json& body) {
	if (!body.contains("max_nodes")) {
		return 250000;
	}

	const Json& value = body.at("max_nodes");
	if (!value.is_number_integer() && !value.is_number_unsigned()) {
		throw std::invalid_argument("max_nodes must be a non-negative integer");
	}

	if (value.is_number_integer() && value.get<std::int64_t>() < 0) {
		throw std::invalid_argument("max_nodes must be a non-negative integer");
	}

	const std::uint64_t parsed = value.get<std::uint64_t>();
	if (parsed > std::numeric_limits<std::size_t>::max()) {
		throw std::invalid_argument("max_nodes is too large");
	}
	return static_cast<std::size_t>(parsed);
}

Move parse_move(const std::string& value) {
	if (value == "UP") {
		return Move::Up;
	}
	if (value == "DOWN") {
		return Move::Down;
	}
	if (value == "LEFT") {
		return Move::Left;
	}
	if (value == "RIGHT") {
		return Move::Right;
	}

	throw std::invalid_argument("move must be UP, DOWN, LEFT or RIGHT");
}

Json board_to_json(const Board& board, const std::string& field = "board") {
	return Json{{"size", board.size()}, {field, board.tiles()}};
}

Json states_to_json(const std::vector<Board>& states) {
	Json result = Json::array();
	for (const Board& state : states) {
		result.push_back(state.tiles());
	}

	return result;
}

Board board_from_json(const Json& body, const std::string& field = "board") {
	if (!body.contains("size") || !body.at("size").is_number_integer()) {
		throw std::invalid_argument("size must be an integer between 2 and 5");
	}

	const int size = body.at("size").get<int>();
	if (size < minimum_size || size > maximum_size) {
		throw std::invalid_argument("size must be an integer between 2 and 5");
	}
	if (!body.contains(field) || !body.at(field).is_array()) {
		throw std::invalid_argument(field + " must be an array");
	}

	try {
		return Board(size, body.at(field).get<std::vector<int>>());
	} catch (const Json::exception&) {
		throw std::invalid_argument(field + " must contain integer tiles");
	} catch (const std::invalid_argument&) {
		throw std::invalid_argument(
			field + " must contain each number from 0 to N*N-1 exactly once"
		);
	}
}

Json parse_json_body(const httplib::Request& request) {
	if (request.body.empty()) {
		throw std::invalid_argument("request body is required");
	}

	try {
		return Json::parse(request.body);
	} catch (const Json::parse_error&) {
		throw std::invalid_argument("request body must be valid JSON");
	}
}

Json goal_types_to_json() {
	Json types = Json::array();
	for (const GoalType type : available_goal_types()) {
		types.push_back(to_string(type));
	}
	return Json{{"types", types}};
}

}  // namespace

void register_routes(httplib::Server& server) {
	server.set_post_routing_handler(
		[](const httplib::Request& request, httplib::Response& response) {
			set_cors_headers(request, response);
		}
	);

	server.Options("/api/puzzle/(.*)", [](const httplib::Request&, httplib::Response& response) {
		response.status = 204;
	});

	server.Get("/api/puzzle/goal-types", [](const httplib::Request&, httplib::Response& response) {
		set_json_response(response, goal_types_to_json());
	});

	server.Get("/api/puzzle/goal", [](const httplib::Request& request, httplib::Response& response) {
		try {
			if (!request.has_param("size")) {
				throw std::invalid_argument("size is required");
			}

			const int size = parse_size(request.get_param_value("size"));
			const std::string type = request.has_param("type")
				? request.get_param_value("type")
				: "ordered";
			
			set_json_response(response, board_to_json(make_goal(size, type), "goal"));
		} catch (const std::invalid_argument& error) {
			set_error_response(response, "invalid_goal", error.what());
		} catch (const std::exception& error) {
			set_error_response(response, "internal_error", error.what(), 500);
		}
	});

	server.Post("/api/puzzle/new", [](const httplib::Request& request, httplib::Response& response) {
		try {
			const Json body = parse_json_body(request);
			if (!body.contains("size") || !body.at("size").is_number_integer()) {
				throw std::invalid_argument("size must be an integer between 2 and 5");
			}

			const int size = body.at("size").get<int>();
			if (size < minimum_size || size > maximum_size) {
				throw std::invalid_argument("size must be an integer between 2 and 5");
			}
			
			const Board goal = body.contains("goal")
				? board_from_json(body, "goal")
				: make_goal(size);
			
			set_json_response(
				response,
				board_to_json(make_random_board(goal))
			);
		} catch (const std::invalid_argument& error) {
			set_error_response(response, "invalid_board", error.what());
		} catch (const std::exception& error) {
			set_error_response(response, "internal_error", error.what(), 500);
		}
	});

	server.Post("/api/puzzle/move", [](const httplib::Request& request, httplib::Response& response) {
		try {
			const Json body = parse_json_body(request);
			const Board board = board_from_json(body);
			if (!body.contains("move") || !body.at("move").is_string()) {
				throw std::invalid_argument("move must be UP, DOWN, LEFT or RIGHT");
			}

			const Move move = parse_move(body.at("move").get<std::string>());
			set_json_response(response, board_to_json(board.moved(move)));
		} catch (const std::invalid_argument& error) {
			set_error_response(response, "invalid_move", error.what());
		} catch (const std::exception& error) {
			set_error_response(response, "internal_error", error.what(), 500);
		}
	});

	server.Post("/api/puzzle/validate", [](const httplib::Request& request, httplib::Response& response) {
		try {
			const Json body = parse_json_body(request);
			const Board initial = board_from_json(body, "initial");
			const Board goal = board_from_json(body, "goal");
			if (initial.size() != goal.size()) {
				throw std::invalid_argument("initial and goal must have matching sizes");
			}
			
			const bool solvable = is_solvable(initial, goal);
			Json result{{"solvable", solvable}};
			if (!solvable) {
				result["reason"] = "different_parity";
			}
			
			set_json_response(response, result);
		} catch (const std::invalid_argument& error) {
			set_error_response(response, "invalid_board", error.what());
		} catch (const std::exception& error) {
			set_error_response(response, "internal_error", error.what(), 500);
		}
	});

	server.Post("/api/puzzle/solve", [](const httplib::Request& request, httplib::Response& response) {
		try {
			const Json body = parse_json_body(request);
			const Board initial = board_from_json(body, "initial");
			const Board goal = board_from_json(body, "goal");
			if (initial.size() != goal.size()) {
				throw std::invalid_argument("initial and goal must have matching sizes");
			}

			const SolveResult result = solve(initial, goal, parse_max_nodes(body));
			if (result.status == SolveStatus::Unsolvable) {
				set_json_response(response, Json{
					{"solvable", false},
					{"moves", Json::array()},
					{"states", Json::array()}
				});
				return;
			}
			
			if (result.status == SolveStatus::NodeLimitReached) {
				set_error_response(
					response,
					"timeout",
					"the search exceeded the node limit",
					408
				);
				return;
			}

			Json moves = Json::array();
			for (const Move move : result.moves) {
				moves.push_back(to_string(move));
			}
			
			set_json_response(response, Json{
				{"solvable", true},
				{"moves", moves},
				{"states", states_to_json(result.states)},
				{"stats", {
					{"nodes_expanded", result.stats.nodes_expanded},
					{"time_ms", result.stats.elapsed_ms},
					{"path_length", result.stats.path_length}
				}}
			});
		} catch (const std::invalid_argument& error) {
			set_error_response(response, "invalid_board", error.what());
		} catch (const std::exception& error) {
			set_error_response(response, "internal_error", error.what(), 500);
		}
	});
}

}  // namespace puzzle
