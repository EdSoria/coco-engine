#include "puzzle/solver.hpp"
#include "puzzle/metrics.hpp"
#include "puzzle/solvability.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>

namespace puzzle {

namespace {

using NodeId = std::size_t;

constexpr NodeId no_parent = std::numeric_limits<NodeId>::max();

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

}  // namespace

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
		return finish();
	}

	result.status = SolveStatus::Solved;
	reconstruct_path(nodes, goal_id, result.moves, result.states);
	result.stats.path_length = result.moves.size();
	return finish();
}

}  // namespace puzzle
