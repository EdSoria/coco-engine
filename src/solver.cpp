#include "puzzle/solver.hpp"
#include "puzzle/metrics.hpp"
#include "puzzle/solvability.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace puzzle {

namespace {

using NodeId = std::size_t;
constexpr NodeId no_parent = std::numeric_limits<NodeId>::max();

constexpr double heuristic_weight = 1.8;

// Custom hash for safely using std::vector<int> with std::unordered_map
struct VectorHash {
	std::size_t operator()(const std::vector<int>& vec) const noexcept {
		std::size_t hash = 1469598103934665603ULL;
		for (const int val : vec) {
			hash ^= static_cast<std::size_t>(val + 1);
			hash *= 1099511628211ULL;
		}
		return hash;
	}
};

// Fast Linear Conflict calculation
int compute_linear_conflicts(const std::vector<int>& tiles, int size, const Positions& goal_positions) {
	int conflicts = 0;

	for (int r = 0; r < size; ++r) {
		for (int c1 = 0; c1 < size; ++c1) {
			const int t1 = tiles[static_cast<std::size_t>(r * size + c1)];
			if (t1 == 0) continue;
			const std::size_t g1 = goal_positions[static_cast<std::size_t>(t1)];
			if (static_cast<int>(g1 / static_cast<std::size_t>(size)) != r) continue;
			const int gc1 = static_cast<int>(g1 % static_cast<std::size_t>(size));

			for (int c2 = c1 + 1; c2 < size; ++c2) {
				const int t2 = tiles[static_cast<std::size_t>(r * size + c2)];
				if (t2 == 0) continue;
				const std::size_t g2 = goal_positions[static_cast<std::size_t>(t2)];
				if (static_cast<int>(g2 / static_cast<std::size_t>(size)) != r) continue;
				const int gc2 = static_cast<int>(g2 % static_cast<std::size_t>(size));

				if (gc1 > gc2) conflicts += 2;
			}
		}
	}

	for (int c = 0; c < size; ++c) {
		for (int r1 = 0; r1 < size; ++r1) {
			const int t1 = tiles[static_cast<std::size_t>(r1 * size + c)];
			if (t1 == 0) continue;
			const std::size_t g1 = goal_positions[static_cast<std::size_t>(t1)];
			if (static_cast<int>(g1 % static_cast<std::size_t>(size)) != c) continue;
			const int gr1 = static_cast<int>(g1 / static_cast<std::size_t>(size));

			for (int r2 = r1 + 1; r2 < size; ++r2) {
				const int t2 = tiles[static_cast<std::size_t>(r2 * size + c)];
				if (t2 == 0) continue;
				const std::size_t g2 = goal_positions[static_cast<std::size_t>(t2)];
				if (static_cast<int>(g2 % static_cast<std::size_t>(size)) != c) continue;
				const int gr2 = static_cast<int>(g2 / static_cast<std::size_t>(size));

				if (gr1 > gr2) conflicts += 2;
			}
		}
	}

	return conflicts;
}

int compute_manhattan(const std::vector<int>& tiles, int size, const Positions& goal_positions) {
	int h = 0;
	const int total = size * size;
	for (int i = 0; i < total; ++i) {
		const int tile = tiles[static_cast<std::size_t>(i)];
		if (tile == 0) continue;
		const std::size_t g_idx = goal_positions[static_cast<std::size_t>(tile)];
		const int r = i / size;
		const int c = i % size;
		const int gr = static_cast<int>(g_idx / static_cast<std::size_t>(size));
		const int gc = static_cast<int>(g_idx % static_cast<std::size_t>(size));
		h += std::abs(r - gr) + std::abs(c - gc);
	}
	return h;
}


// 1. STANDARD A* (For N <= 3 boards)
struct Node {
	Board board;
	int g;
	int h;
	NodeId parent;
	Move move;

	int f() const noexcept {
		return g + h;
	}
};

struct QueueItem {
	NodeId node_id;
	int f;
};

struct QueueItemCompare {
	bool operator()(const QueueItem& left, const QueueItem& right) const noexcept {
		return left.f > right.f;
	}
};

using OpenQueue = std::priority_queue<
	QueueItem,
	std::vector<QueueItem>,
	QueueItemCompare
>;

void reconstruct_path(
	const std::vector<Node>& nodes,
	NodeId goal_id,
	std::vector<Move>& moves,
	std::vector<Board>& states
) {
	std::vector<Move> reversed_moves;
	std::vector<Board> reversed_states;

	for (NodeId current_id = goal_id;
		 current_id != no_parent;
		 current_id = nodes[current_id].parent) {
		const Node& current = nodes[current_id];
		reversed_states.push_back(current.board);

		if (current.parent != no_parent) {
			reversed_moves.push_back(current.move);
		}
	}

	moves.assign(reversed_moves.rbegin(), reversed_moves.rend());
	states.assign(reversed_states.rbegin(), reversed_states.rend());
}

void solve_astar(const Board& initial, const Board& goal, std::size_t max_nodes, SolveResult& result) {
	std::vector<Node> nodes;
	OpenQueue open;
	std::unordered_map<std::string, int> best_cost;
	const Positions goal_positions = get_positions(goal);

	nodes.push_back({
		initial,
		0,
		manhattan_distance(initial, goal_positions),
		no_parent,
		Move::Up
	});

	best_cost.emplace(initial.key(), 0);
	open.push({0, nodes.front().f()});

	NodeId goal_id = no_parent;
	while (!open.empty() && result.stats.nodes_expanded < max_nodes) {
		const QueueItem item = open.top();
		open.pop();

		const Node current = nodes[item.node_id];
		const auto current_cost = best_cost.find(current.board.key());
		if (current_cost != best_cost.end() && current.g > current_cost->second) {
			continue;
		}

		if (current.board == goal) {
			goal_id = item.node_id;
			break;
		}

		++result.stats.nodes_expanded;
		for (const Neighbor& neighbor : current.board.neighbors()) {
			const int next_cost = current.g + 1;
			const std::string key = neighbor.board.key();
			const auto known_cost = best_cost.find(key);
			if (known_cost != best_cost.end() && known_cost->second <= next_cost) {
				continue;
			}

			const NodeId node_id = nodes.size();
			nodes.push_back({
				neighbor.board,
				next_cost,
				manhattan_distance(neighbor.board, goal_positions),
				item.node_id,
				neighbor.move
			});

			best_cost[key] = next_cost;
			open.push({node_id, nodes.back().f()});
		}
	}

	if (goal_id == no_parent) {
		result.status = SolveStatus::NodeLimitReached;
		return;
	}

	result.status = SolveStatus::Solved;
	reconstruct_path(nodes, goal_id, result.moves, result.states);
	result.stats.path_length = result.moves.size();
}

// 2. ULTRA-LIGHT WEIGHTED A* (For N >= 4 boards)
struct FastNode {
	std::vector<int> tiles;
	int blank_pos;
	int g;
	int h;
	NodeId parent;
	Move move;

	double f() const noexcept {
		return static_cast<double>(g) + heuristic_weight * static_cast<double>(h);
	}
};

struct FastQueueItem {
	NodeId node_id;
	double f;
};

struct FastQueueCompare {
	bool operator()(const FastQueueItem& left, const FastQueueItem& right) const noexcept {
		return left.f > right.f;
	}
};

void build_solution_states(
	const Board& initial,
	const std::vector<Move>& moves,
	std::vector<Board>& states
) {
	states.clear();
	states.reserve(moves.size() + 1);
	states.push_back(initial);

	Board current = initial;
	for (const Move move : moves) {
		current = current.moved(move);
		states.push_back(current);
	}
}

void solve_weighted_astar(
	const Board& initial,
	const Board& goal,
	std::size_t max_nodes,
	SolveResult& result
) {
	const int size = initial.size();
	const Positions goal_positions = get_positions(goal);
	const std::vector<int> goal_tiles = goal.tiles();

	std::vector<FastNode> nodes;
	std::priority_queue<FastQueueItem, std::vector<FastQueueItem>, FastQueueCompare> open;
	std::unordered_map<std::vector<int>, int, VectorHash> best_cost;

	auto calc_h = [&](const std::vector<int>& t) {
		return compute_manhattan(t, size, goal_positions) +
			   compute_linear_conflicts(t, size, goal_positions);
	};

	const std::vector<int> init_tiles = initial.tiles();
	const int init_blank = static_cast<int>(initial.blank_index());
	const int init_h = calc_h(init_tiles);

	nodes.push_back({
		init_tiles,
		init_blank,
		0,
		init_h,
		no_parent,
		Move::Up
	});

	best_cost[init_tiles] = 0;
	open.push({0, nodes.front().f()});

	NodeId goal_id = no_parent;

	while (!open.empty() && result.stats.nodes_expanded < max_nodes) {
		const FastQueueItem item = open.top();
		open.pop();

		const NodeId curr_id = item.node_id;

		const std::vector<int> curr_tiles = nodes[curr_id].tiles;
		const int curr_blank = nodes[curr_id].blank_pos;
		const int curr_g = nodes[curr_id].g;

		const auto known = best_cost.find(curr_tiles);
		if (known != best_cost.end() && curr_g > known->second) {
			continue;
		}

		if (curr_tiles == goal_tiles) {
			goal_id = curr_id;
			break;
		}

		++result.stats.nodes_expanded;

		const int r = curr_blank / size;
		const int c = curr_blank % size;

		// Directions: Up, Down, Left, Right
		const std::array<int, 4> dr = {-1, 1, 0, 0};
		const std::array<int, 4> dc = {0, 0, -1, 1};
		const std::array<Move, 4> moves_enum = {Move::Up, Move::Down, Move::Left, Move::Right};

		for (int i = 0; i < 4; ++i) {
			const int nr = r + dr[i];
			const int nc = c + dc[i];

			if (nr >= 0 && nr < size && nc >= 0 && nc < size) {
				const int next_blank = nr * size + nc;
				std::vector<int> next_tiles = curr_tiles;
				std::swap(next_tiles[static_cast<std::size_t>(curr_blank)], next_tiles[static_cast<std::size_t>(next_blank)]);

				const int next_g = curr_g + 1;
				const auto known_cost = best_cost.find(next_tiles);

				if (known_cost != best_cost.end() && known_cost->second <= next_g) {
					continue;
				}

				const NodeId new_node_id = nodes.size();
				const int next_h = calc_h(next_tiles);

				nodes.push_back({
					std::move(next_tiles),
					next_blank,
					next_g,
					next_h,
					curr_id,
					moves_enum[i]
				});

				// Re-fetch the key of the newly added node
				best_cost[nodes.back().tiles] = next_g;
				open.push({new_node_id, nodes.back().f()});
			}
		}
	}

	if (goal_id == no_parent) {
		result.status = SolveStatus::NodeLimitReached;
		return;
	}

	result.status = SolveStatus::Solved;

	// Reconstruct the solution
	std::vector<Move> reversed_moves;
	for (NodeId current_id = goal_id;
		 current_id != no_parent;
		 current_id = nodes[current_id].parent) {
		if (nodes[current_id].parent != no_parent) {
			reversed_moves.push_back(nodes[current_id].move);
		}
	}

	result.moves.assign(reversed_moves.rbegin(), reversed_moves.rend());
	build_solution_states(initial, result.moves, result.states);
	result.stats.path_length = result.moves.size();
}

}  // namespace

// 3. PUBLIC ENTRY POINT
SolveResult solve(const Board& initial, const Board& goal, std::size_t max_nodes) {
	SolveResult result;
	const auto started = std::chrono::steady_clock::now();
	const auto finish = [&result, &started]() {
		result.stats.elapsed_ms = std::chrono::duration<double, std::milli>(
			std::chrono::steady_clock::now() - started
		).count();
		return result;
	};

	if (!is_solvable(initial, goal)) {
		result.status = SolveStatus::Unsolvable;
		return finish();
	}

	if (initial == goal) {
		result.status = SolveStatus::Solved;
		result.states.push_back(initial);
		return finish();
	}

	if (initial.size() <= 3) {
		solve_astar(initial, goal, max_nodes, result);
	} else {
		solve_weighted_astar(initial, goal, max_nodes, result);
	}

	return finish();
}

}  // namespace puzzle