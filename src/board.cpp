#include "puzzle/board.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace puzzle {

Board::Board(int size, std::vector<int> tiles) : size_(size), tiles_(std::move(tiles)) {
	if (!is_valid()) {
		throw std::invalid_argument("board must contain each value from 0 to N * N - 1 exactly once");
	}
}

int Board::size() const noexcept { return size_; }
const std::vector<int>& Board::tiles() const noexcept { return tiles_; }

bool Board::is_valid() const {
	if (size_ < 2 || tiles_.size() != static_cast<std::size_t>(size_ * size_)) {
		return false;
	}

	std::vector<bool> visited(tiles_.size(), false);
	for (const int tile : tiles_) {
		if (tile < 0 || tile >= static_cast<int>(tiles_.size()) || visited[static_cast<std::size_t>(tile)]) {
			return false;
		}
		visited[static_cast<std::size_t>(tile)] = true;
	}
	return true;
}

std::size_t Board::blank_index() const {
	const auto blank = std::find(tiles_.begin(), tiles_.end(), 0);
	if (blank == tiles_.end()) {
		throw std::logic_error("valid board has no blank tile");
	}
	return static_cast<std::size_t>(std::distance(tiles_.begin(), blank));
}

Board Board::moved(Move move) const {
	const auto blank = blank_index();
	const int row = static_cast<int>(blank) / size_;
	const int column = static_cast<int>(blank) % size_;
	int target = -1;

	switch (move) {
		case Move::Up:
			if (row > 0) target = static_cast<int>(blank) - size_;
			break;
		case Move::Down:
			if (row + 1 < size_) target = static_cast<int>(blank) + size_;
			break;
		case Move::Left:
			if (column > 0) target = static_cast<int>(blank) - 1;
			break;
		case Move::Right:
			if (column + 1 < size_) target = static_cast<int>(blank) + 1;
			break;
	}

	if (target < 0) {
		throw std::invalid_argument("move is outside the board");
	}

	auto next_tiles = tiles_;
	std::swap(next_tiles[blank], next_tiles[static_cast<std::size_t>(target)]);
	return Board(size_, std::move(next_tiles));
}

std::vector<Neighbor> Board::neighbors() const {
	std::vector<Neighbor> result;
	for (const auto move : {Move::Up, Move::Down, Move::Left, Move::Right}) {
		try {
			result.push_back({move, moved(move)});
		} catch (const std::invalid_argument&) {
			// Boundary moves are not neighbors.
		}
	}
	return result;
}

std::string Board::key() const {
	std::string result;
	result.reserve(tiles_.size() * 3);
	for (const int& tile : tiles_) {
		result += std::to_string(tile);
		result += ',';
	}
	return result;
}

bool Board::operator==(const Board& other) const noexcept {
	return size_ == other.size_ && tiles_ == other.tiles_;
}

bool Board::operator!=(const Board& other) const noexcept {
	return !(*this == other);
}

}  // namespace puzzle
