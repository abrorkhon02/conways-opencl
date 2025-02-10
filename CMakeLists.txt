cmake_minimum_required(VERSION 3.15)
project(GameOfLifeHPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-O3)

# Main executable (CLI-based game)
add_executable(game_of_life
    src/main.cpp
    src/CLI.cpp
    src/GameOfLife.cpp
)
target_include_directories(game_of_life PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# OpenCL executable with printing disabled for performance measurement
find_package(OpenCL REQUIRED)
add_executable(gol_opencl
    src/gol_opencl.cpp
)
target_include_directories(gol_opencl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(gol_opencl PRIVATE OpenCL::OpenCL)
target_compile_definitions(gol_opencl PRIVATE DISABLE_PRINT)

# Performance measurement executable
add_executable(performance_measure
    src/performance_measure.cpp
)
target_include_directories(performance_measure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(test_save
    src/test_save.cpp
    src/GameOfLife.cpp   
)
target_include_directories(test_save PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
