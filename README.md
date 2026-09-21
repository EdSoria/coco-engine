# Coco

Engine and REST API for an N-Puzzle (8-puzzle generalized to an NxN board) written in C++. Generates solvable random boards, supports predefined or custom goal states, and solves the puzzle using A*, returning both the move sequence and intermediate states.

## Features

- Random initial board generation, guaranteed solvable.
- 5 predefined goal types, plus support for custom user-defined goals.
- Compatibility (solvability) validation between initial state and goal.
- Move generator from a board.
- Optimal solving via A* with Manhattan distance heuristic.
- Response includes the full move sequence and intermediate states.

## Architecture

```mermaid
graph LR
    A[Frontend JS] -->|HTTP/JSON| B[REST API - cpp-httplib]
    B --> C[Coco]
```

The engine has no HTTP dependency at all, it only works with in-memory data structures. The API layer is the only part that knows about cpp-httplib and translates between JSON and the engine's structures.

## Tech stack

- **C++17**
- **[cpp-httplib](https://github.com/yhirose/cpp-httplib)** — HTTP server
- **[nlohmann/json](https://github.com/nlohmann/json)** — JSON serialization
- **[Catch2](https://github.com/catchorg/Catch2)** — testing
- **CMake** — build system

## Requirements

- C++17-compatible compiler (GCC 9+, Clang 10+)
- CMake 3.15+
- Git
- Ninja (recommended) or another CMake-supported generator

## Installation and running

```bash
git clone https://github.com/edsoria/coco-engine.git
cd coco-engine
cmake -S . -B build -G Ninja -DBUILD_TESTING=ON
cmake --build build
./build/puzzle_server
```

The server listens on `http://localhost:8080`.

On Windows, run `build\puzzle_server.exe` after building. The server binds to
`0.0.0.0:8080`, so it can receive requests from another device on the network
if the firewall allows the port.

### Docker

```bash
docker build -t coco-engine .
docker run --rm -p 8080:8080 coco-engine
```

The server is available at `http://localhost:8080`.

## API documentation

| Endpoint | Method | Description |
|---|---|---|
| `/api/puzzle/new` | POST | Generates a solvable random board for a given size N |
| `/api/puzzle/goal` | GET | Returns the canonical (ordered) goal for a given size N |
| `/api/puzzle/goal-types` | GET | Lists the available predefined goal types |
| `/api/puzzle/move` | POST | Given a board, returns the board with the move made |
| `/api/puzzle/validate` | POST | Checks whether an initial/goal pair is solvable together |
| `/api/puzzle/solve` | POST | Solves with A* and returns moves + intermediate states |

The complete public contract, request/response examples and JavaScript usage
are in [docs/API.md](docs/API.md).

The API accepts JSON requests and returns JSON responses. A browser frontend
running on another port may need a development proxy or CORS support. The
simplest local setup is to run both applications on the same computer and
configure the frontend to use `http://localhost:8080`.

## Tests

```bash
cd build
ctest
```

or run the test binary directly:

```bash
./puzzle_tests
```

The suite covers engine behavior and HTTP routes. On Windows, run
`build\puzzle_tests.exe` instead.

### Direct algorithm test

To try the solver from the console without starting the HTTP server, build and
run `puzzle_cli`:

```bash
cmake --build build --target puzzle_cli
./build/puzzle_cli
```

On Windows, run `build\puzzle_cli.exe`. Enter the board size, the initial board
and the goal board as space-separated numbers. For example, a one-move 3x3
test uses:

```text
3
1 2 3 4 5 6 7 0 8
1 2 3 4 5 6 7 8 0
```

The program prints the status, moves, expanded nodes, elapsed time and every
state in the solution path.

## Algorithm

Solving is done with **A\*** using the **Manhattan distance** heuristic (sum of each tile's distance from its target position), which is admissible and guarantees the optimal solution (minimum number of moves).

Before attempting to solve, the (initial, goal) pair is validated for solvability via inversion count and blank-tile position, this avoids running an expensive search on a puzzle that has no possible solution.

## Project structure

```
coco-engine/
├── CMakeLists.txt        # build configuration and dependencies
├── README.md
├── Dockerfile            # build and runtime image
├── .dockerignore
├── .vscode/              # IntelliSense configuration
├── include/puzzle/       # public engine headers
├── src/                  # engine, REST routes and server bootstrap
├── tests/                # engine and HTTP tests
├── docs/API.md           # public API contract
├── third_party/          # auxiliary repository notes
└── TODO.md               # visible project backlog
```

## Roadmap

- [ ] Support for larger boards (6x6+) with additional optimizations (IDA*, walking distance).
- [ ] Deploy the Docker image on Render/Fly.io.