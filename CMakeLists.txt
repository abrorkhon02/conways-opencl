cmake_minimum_required(VERSION 3.15)
project(GameOfLifeHPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
add_compile_options(-O3)

# Set OpenCL target version to avoid warnings
add_compile_definitions(CL_TARGET_OPENCL_VERSION=120)

find_package(OpenCL REQUIRED)

include_directories(${OpenCL_INCLUDE_DIRS})

# Main game of life executable
add_executable(game_of_life
    src/main.cpp
    src/CLI.cpp
    src/GameOfLife.cpp
)
target_include_directories(game_of_life PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(game_of_life PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(game_of_life PRIVATE ${OpenCL_LIBRARIES})

# Performance measurement executable
add_executable(performance_measure
    src/performance_measure.cpp
    src/GameOfLife.cpp
)
target_include_directories(performance_measure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(performance_measure PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(performance_measure PRIVATE ${OpenCL_LIBRARIES})
