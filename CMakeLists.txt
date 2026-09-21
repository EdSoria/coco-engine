cmake_minimum_required(VERSION 3.15)
project(coco_engine LANGUAGES CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.3.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    Catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(httplib nlohmann_json Catch2)

add_library(puzzle_engine
    src/board.cpp
    src/move.cpp
    src/goals.cpp
    src/metrics.cpp
    src/solvability.cpp
    src/random_board.cpp
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
    add_executable(puzzle_tests
        tests/puzzle_tests.cpp
        src/api.cpp
    )
    target_link_libraries(
        puzzle_tests
        PRIVATE
        puzzle_engine
        httplib::httplib
        nlohmann_json::nlohmann_json
        Catch2::Catch2WithMain
    )
    add_test(NAME puzzle_tests COMMAND puzzle_tests)
endif()
