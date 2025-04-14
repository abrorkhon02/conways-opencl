# Conway's Game of Life with OpenCL

High-performance implementation of Conway's Game of Life using both OpenCL and scalar (CPU) approaches. This project demonstrates advanced parallel computing techniques, efficient memory management, and robust simulation strategies.

## Requirements

- **CMake** (version 3.15 or later)
- **OpenCL SDK** (NVIDIA, AMD, or Intel)
- **C++17 Compiler** (MinGW-w64 or equivalent for Windows)
- **Windows build tools** (MSYS2 recommended) or Linux/WSL

## Directory Structure

- **include/**: Header files
  - `GameOfLife.h`: Main simulation class definition
  - `CLI.h`: Command-line interface class
  - `SharedGlobals.h`: Global variables shared across modules
- **src/**: Source code files
  - `GameOfLife.cpp`: Implementation of simulation logic
  - `CLI.cpp`: Command-line interface implementation
  - `main.cpp`: Entry point for interactive program
  - `performance_measure.cpp`: Benchmarking tool
  - `SharedGlobals.cpp`: Implementation of shared globals

## Build

Open a terminal and navigate to the project root. Then, create a build directory and run CMake:

```powershell
mkdir build
cd build
cmake ..
cmake --build . --target game_of_life performance_measure
```

### This will build the following executables:

- **game_of_life.exe** – The interactive CLI version with integrated OpenCL support
- **performance_measure.exe** – The performance measurement tool

## Running the Project

From the build folder, run:

### CLI Version

```powershell
./game_of_life.exe
```

The CLI supports the following commands:

- **create**: Create a new world (prompts for width and height)
- **load**: Load a world from a file (prompts for filename)
- **save**: Save the current world to a file (prompts for filename)
- **run \<mode\> \<n\>**: Run the simulation for n generations
  - **scalar**: CPU-based evolution
  - **opencl**: OpenCL (GPU/CPU-based) evolution
- **set**: Set the state of a cell (prompts for coordinates and state)
- **get**: Get the state of a cell (prompts for coordinates)
- **glider**: Insert a "Glider" pattern at a specified position
- **toad**: Insert a "Toad" pattern at a specified position
- **beacon**: Insert a "Beacon" pattern at a specified position
- **methuselah**: Insert a Methuselah pattern at a specified position
- **print on/off**: Enable or disable printing after each generation
- **delay \<ms\>**: Set the delay (in milliseconds) for simulation
- **help**: Display this help message
- **exit / quit**: Exit the program

### Performance Measurement

```powershell
./performance_measure.exe
```

This executable runs simulations on various grid sizes using OpenCL, outputs a CSV file (simulation_results.csv), and can be used to generate performance plots.

## Technical Details

- **Toroidal Grid**: The grid wraps around at the edges, ensuring that every cell always has eight neighbors.
- **Double Buffering**: Two separate buffers are used to store the current and next generation of cell states.
- **OpenCL Integration**: The OpenCL kernel is directly integrated into the main application, eliminating the need for external process calls.
- **OpenCL Parallelization**: Each work-item computes the next state of a single cell, allowing for significant acceleration on parallel hardware.
- **Memory Management**: STL containers (e.g., std::vector) manage memory safely and efficiently, leveraging RAII principles.
- **Error Handling**: Comprehensive OpenCL error detection and reporting system to diagnose issues during kernel execution.
- **Diagnostic Tracing**: Built-in diagnostic logging system for troubleshooting OpenCL execution issues.
- **Device Flexibility**: Automatically falls back to CPU computation if a GPU device is not available.

### OpenCL Implementation

The OpenCL implementation follows these steps:

1. Initialization of the OpenCL environment (platform, device, context, queue)
2. Creation of the program from embedded kernel source
3. Creation of kernel and memory buffers for input/output
4. Execution of the kernel for each generation
5. Reading back results after computation
6. Proper resource cleanup

Each cell's state is computed in parallel, providing significant performance improvements for large grids.

## Input Format Flexibility

The application supports flexible input formats for coordinates:

- Standard format: `x,y`
- With parentheses: `(x,y)`
- With spaces: `x y`

## Troubleshooting

If the OpenCL implementation crashes or behaves unexpectedly:

- Check the OpenCL debug trace file (`opencl_debug_trace.log`) created in your working directory
- Verify that your OpenCL drivers are up-to-date
- Try running with smaller grid sizes first to verify functionality

## License

This project is licensed under the MIT License - see the LICENSE file for details.
