# conways-opencl

High-performance implementation of Conway’s Game of Life using OpenCL and scalar approaches. This project, developed as the final assignment for the High-Performance Computer Architectures course, demonstrates parallel computing techniques, efficient memory management, and advanced simulation strategies.

## Requirements

- CMake (version 3.15 or later)
- OpenCL SDK
- C++17 Compiler (MinGW-w64 or equivalent for Windows)
- Windows build tools (MSYS2 recommended)

## Build

```powershell
cd build
cmake ..
cmake --build . --target game_of_life gol_opencl
```

## This will build both:

- game_of_life.exe – The interactive CLI version.
- gol_opencl.exe – The standalone OpenCL evolution executable.

## Running the Project

From the build folder, run:

```powershell
# CLI Version
./game_of_life.exe

# OpenCL Direct
./gol_opencl.exe <generations>
```

## The CLI supports the following commands:

- create : Create a new world (prompts for width and height)
- load : Load a world from a file (prompts for filename)
- save : Save the current world to a file (prompts for filename)
- run <mode> <n> : Run the simulation for n generations
- scalar : CPU-based evolution
- opencl : OpenCL (GPU/CPU-based) evolution
- set : Set the state of a cell (prompts for coordinates and state)
- get : Get the state of a cell (prompts for coordinates)
- glider : Insert a "Glider" pattern at a specified position
- toad : Insert a "Toad" pattern at a specified position
- beacon : Insert a "Beacon" pattern at a specified position
- methuselah : Insert a Methuselah pattern at a specified position
- print on/off : Enable or disable printing after each generation
- delay <ms> : Set the delay (in milliseconds) for simulation
