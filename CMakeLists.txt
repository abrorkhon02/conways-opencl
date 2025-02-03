cmake_minimum_required(VERSION 3.15)
project(GameOfLifeHPC)

set(CMAKE_CXX_STANDARD 17)

add_executable(game_of_life
    src/main.cpp
    src/GameOfLife.cpp
)

target_include_directories(game_of_life PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)