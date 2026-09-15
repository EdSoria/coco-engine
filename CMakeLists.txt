cmake_minimum_required(VERSION 3.15)
project(coco_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(puzzle_engine
    src/board.cpp
    src/goals.cpp
    src/solver.cpp
)
target_include_directories(puzzle_engine PUBLIC include)

add_executable(puzzle_server
    src/api.cpp
    src/main.cpp
)
target_link_libraries(puzzle_server PRIVATE puzzle_engine)

include(CTest)
if(BUILD_TESTING)
    add_executable(puzzle_tests tests/puzzle_tests.cpp)
    target_link_libraries(puzzle_tests PRIVATE puzzle_engine)
    add_test(NAME puzzle_tests COMMAND puzzle_tests)
endif()

# TODO: add cpp-httplib, nlohmann/json and Catch2 through FetchContent or third_party/.
