#pragma once

#include "puzzle/board.hpp"
#include <string>
#include <vector>

namespace puzzle {

enum class GoalType {
	Ordered,
	Spiral,
	Columns,
	Snake,
	Reverse
};

// Stable names used by the HTTP/API boundary.
std::string to_string(GoalType type);
GoalType goal_type_from_string(const std::string& name);
std::vector<GoalType> available_goal_types();

// Goal construction is the only place that knows each layout's geometry.
Board make_goal(int size, GoalType type = GoalType::Ordered);
Board make_goal(int size, const std::string& type);

}  // namespace puzzle
