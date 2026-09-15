#pragma once

#include "puzzle/board.hpp"
#include <string>
#include <vector>

namespace puzzle {

// TODO: declare predefined goal types and goal construction.
std::vector<std::string> goal_types();
Board make_goal(int size, const std::string& type = "ordered");

}  // namespace puzzle
