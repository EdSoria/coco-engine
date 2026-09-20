cmake_minimum_required(VERSION 3.15)
project(coco_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.18.3
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)

FetchContent_MakeAvailable(httplib nlohmann_json Catch2)

add_library(puzzle_engine
    src/board.cpp
    src/move.cpp
    src/goals.cpp
    src/solvability.cpp
    src/solver.cpp
)
target_include_directories(puzzle_engine PUBLIC include)

add_executable(puzzle_server
    src/api.cpp
    src/main.cpp
)
target_link_libraries(puzzle_server PRIVATE puzzle_engine httplib::httplib nlohmann_json::nlohmann_json)

include(CTest)
if(BUILD_TESTING)
    add_executable(puzzle_tests tests/puzzle_tests.cpp)
    target_link_libraries(puzzle_tests PRIVATE puzzle_engine Catch2::Catch2WithMain)
    add_test(NAME puzzle_tests COMMAND puzzle_tests)
endif()
