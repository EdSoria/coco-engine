#include "puzzle/goals.hpp"

#include <stdexcept>
#include <utility>

namespace {

int next_tile(int& value, int total) {
	return value == total ? 0 : value++;
}

}  // namespace

namespace puzzle {

std::string to_string(GoalType type) {
	switch (type) {
		case GoalType::Ordered: return "ordered";
		case GoalType::Spiral: return "spiral";
		case GoalType::Columns: return "columns";
		case GoalType::Snake: return "snake";
		case GoalType::Reverse: return "reverse";
	}
	throw std::invalid_argument("unknown goal type");
}

GoalType goal_type_from_string(const std::string& name) {
	for (const auto type : available_goal_types()) {
		if (to_string(type) == name) {
			return type;
		}
	}
	throw std::invalid_argument("unknown goal type: " + name);
}

std::vector<GoalType> available_goal_types() {
	return {
		GoalType::Ordered,
		GoalType::Spiral,
		GoalType::Columns,
		GoalType::Snake,
		GoalType::Reverse
	};
}

Board make_goal(int size, GoalType type) {
	if (size < 2 || size > 5) {
		throw std::invalid_argument("size must be between 2 and 5");
	}

	const int total = size * size;
	std::vector<int> tiles(static_cast<std::size_t>(total), 0);
	int value = 1;
	const auto set_next_tile = [&](int row, int column) {
		tiles[static_cast<std::size_t>(row * size + column)] = next_tile(value, total);
	};

	switch (type) {
		case GoalType::Ordered:
			for (int index = 0; index < total; ++index) {
				tiles[static_cast<std::size_t>(index)] = next_tile(value, total);
			}
			break;

		case GoalType::Columns:
			for (int column = 0; column < size; ++column) {
				for (int row = 0; row < size; ++row) {
					set_next_tile(row, column);
				}
			}
			break;

		case GoalType::Snake:
			for (int row = 0; row < size; ++row) {
				if (row % 2 == 0) {
					for (int column = 0; column < size; ++column) {
						set_next_tile(row, column);
					}
				} else {
					for (int column = size - 1; column >= 0; --column) {
						set_next_tile(row, column);
					}
				}
			}
			break;

		case GoalType::Spiral: {
			int top = 0;
			int bottom = size - 1;
			int left = 0;
			int right = size - 1;

			while (top <= bottom && left <= right) {
				for (int column = left; column <= right; ++column) {
					set_next_tile(top, column);
				}
				++top;

				for (int row = top; row <= bottom; ++row) {
					set_next_tile(row, right);
				}
				--right;

				if (top <= bottom) {
					for (int column = right; column >= left; --column) {
						set_next_tile(bottom, column);
					}
					--bottom;
				}

				if (left <= right) {
					for (int row = bottom; row >= top; --row) {
						set_next_tile(row, left);
					}
					++left;
				}
			}
			break;
		}

		case GoalType::Reverse:
			for (int index = 0; index < total; ++index) {
				tiles[static_cast<std::size_t>(index)] = total - index - 1;
			}
			break;
	}

	return Board(size, std::move(tiles));
}

Board make_goal(int size, const std::string& type) {
	return make_goal(size, goal_type_from_string(type));
}

}  // namespace puzzle
