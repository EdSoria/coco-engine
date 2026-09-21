# Coco API

Public HTTP/JSON contract for the Coco N-Puzzle backend.

## Local server

The default base URL is:

```text
http://localhost:8080
```

Start the server from the repository root after building it:

```bash
cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
./build/puzzle_server
```

On Windows, run `build\puzzle_server.exe` instead. The server listens on
`0.0.0.0:8080`.

## Common data rules

- Supported board sizes are 2 through 5.
- A board is a flat row-major array with exactly `size * size` integers.
- Every integer from `0` through `size * size - 1` must appear exactly once.
- `0` represents the blank space.
- Moves describe the direction in which the blank moves: `UP`, `DOWN`, `LEFT` or `RIGHT`.
- Request bodies must be JSON and use `Content-Type: application/json`.

## Endpoints

### `GET /api/puzzle/goal`

Returns a predefined goal. The `type` query parameter is optional and defaults
to `ordered`.

```text
GET /api/puzzle/goal?size=3&type=ordered
```

Response:

```json
{
	"size": 3,
	"goal": [1, 2, 3, 4, 5, 6, 7, 8, 0]
}
```

Supported goal types are `ordered`, `spiral`, `columns`, `snake` and `reverse`.
Unknown types or invalid sizes return `400` with an `invalid_goal` error.

### `GET /api/puzzle/goal-types`

Returns the available goal type names.

```json
{
	"types": ["ordered", "spiral", "columns", "snake", "reverse"]
}
```

### `POST /api/puzzle/new`

Generates a valid, solvable board. The `goal` field is optional; when omitted,
the ordered goal for `size` is used.

Request:

```json
{
	"size": 3,
	"goal": [1, 2, 3, 4, 5, 6, 7, 8, 0]
}
```

Response:

```json
{
	"size": 3,
	"board": [1, 2, 3, 4, 5, 6, 0, 7, 8]
}
```

Invalid sizes or goals return `400` with an `invalid_board` error.

### `POST /api/puzzle/move`

Applies one move to a board. The move is applied to the blank space.

Request:

```json
{
	"size": 3,
	"board": [1, 2, 3, 4, 5, 6, 7, 0, 8],
	"move": "RIGHT"
}
```

Response:

```json
{
	"size": 3,
	"board": [1, 2, 3, 4, 5, 6, 7, 8, 0]
}
```

Invalid boards, unknown moves and moves outside the board return `400` with an
`invalid_move` error.

### `POST /api/puzzle/validate`

Checks whether an initial board can reach a goal.

Request:

```json
{
	"size": 3,
	"initial": [1, 2, 3, 4, 5, 6, 0, 7, 8],
	"goal": [1, 2, 3, 4, 5, 6, 7, 8, 0]
}
```

Solvable response:

```json
{
	"solvable": true
}
```

Unsolvable response:

```json
{
	"solvable": false,
	"reason": "different_parity"
}
```

Invalid boards or different sizes return `400` with an `invalid_board` error.

### `POST /api/puzzle/solve`

Solves the puzzle with A* and returns the moves, every intermediate state and
search statistics. `states` always contains one more element than `moves`.

Request:

```json
{
	"size": 3,
	"initial": [1, 2, 3, 4, 5, 6, 7, 0, 8],
	"goal": [1, 2, 3, 4, 5, 6, 7, 8, 0]
}
```

Response:

```json
{
	"solvable": true,
	"moves": ["RIGHT"],
	"states": [
		[1, 2, 3, 4, 5, 6, 7, 0, 8],
		[1, 2, 3, 4, 5, 6, 7, 8, 0]
	],
	"stats": {
		"nodes_expanded": 1,
		"time_ms": 0.1,
		"path_length": 1
	}
}
```

For an unsolvable pair, the endpoint returns `200`:

```json
{
	"solvable": false,
	"moves": [],
	"states": []
}
```

If the search reaches its node limit, the endpoint returns `408` with a
`timeout` error. Invalid boards return `400` with `invalid_board`.

## Error format

Errors use this shape:

```json
{
	"error": "invalid_board",
	"message": "human-readable explanation"
}
```

The current error codes are `invalid_goal`, `invalid_board`, `invalid_move`,
`timeout` and `internal_error`.

## JavaScript example

```js
const API_URL = "http://localhost:8080";

const response = await fetch(`${API_URL}/api/puzzle/solve`, {
	method: "POST",
	headers: { "Content-Type": "application/json" },
	body: JSON.stringify({
		size: 3,
		initial: [1, 2, 3, 4, 5, 6, 7, 0, 8],
		goal: [1, 2, 3, 4, 5, 6, 7, 8, 0]
	})
});

if (!response.ok) {
	throw new Error((await response.json()).message);
}

const solution = await response.json();
```

For local development, run the frontend and backend on the same computer. If
the frontend runs on a different port, configure its development proxy or add
CORS support to the backend before making browser requests.
