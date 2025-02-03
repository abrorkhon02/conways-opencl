#include <CL/cl.h>  
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <cstring> 

// -------------- KERNEL: Embedded as a string literal --------------
// Toroidal Game of Life: Each work-item processes exactly one cell.
// We define a macro for indexing, and everything else is in the kernel.

static const char *golKernelSrc = R"CLC(
#define INDEXFN(xx, yy, w) ((yy)*(w) + (xx))

__kernel void evolveToroidal(__global const int* currentGrid,
                             __global int* nextGrid,
                             int width,
                             int height)
{
    int x = get_global_id(0);
    int y = get_global_id(1);

    // Count neighbors with toroidal wrap
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue; // skip self

            int nx = (x + dx + width) % width;
            int ny = (y + dy + height) % height;

            count += currentGrid[ INDEXFN(nx, ny, width) ];
        }
    }

    int currentState = currentGrid[ INDEXFN(x, y, width) ];
    int nextState = 0;
    if (currentState == 1) {
        // Survives if 2 or 3 neighbors
        nextState = ((count == 2) || (count == 3)) ? 1 : 0;
    } else {
        // Becomes alive if exactly 3 neighbors
        nextState = (count == 3) ? 1 : 0;
    }

    nextGrid[ INDEXFN(x, y, width) ] = nextState;
}
)CLC";

static void printGrid(const std::vector<int>& grid, int width, int height)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::cout << (grid[y * width + x] == 1 ? "*" : ".");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main()
{
    // ---------------- 1) Setup grid data (host) ----------------
    const int width  = 10;
    const int height = 10;
    const size_t gridSize = width * height;

    // Create a random initial grid
    std::vector<int> currentGrid(gridSize, 0);
    for (size_t i = 0; i < gridSize; i++) {
        currentGrid[i] = ((rand() % 100) < 30) ? 1 : 0; // ~30% alive
    }

    std::cout << "Initial Grid:\n";
    printGrid(currentGrid, width, height);

    // We'll store GPU results in nextGrid
    std::vector<int> nextGrid(gridSize, 0);

    // ---------------- 2) OpenCL Boilerplate ----------------
    cl_int err = 0;

    // 2a) Get the first available platform
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (numPlatforms == 0) {
        std::cerr << "No OpenCL platforms found.\n";
        return 1;
    }
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);

    cl_platform_id platform = platforms[0];

    // 2b) Get a device (GPU or CPU)
    cl_uint numDevices = 0;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    if (numDevices == 0) {
        // fallback to CPU if GPU not found
        clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if (numDevices == 0) {
            std::cerr << "No GPU/CPU devices found.\n";
            return 1;
        }
    }
    std::vector<cl_device_id> devices(numDevices);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
    cl_device_id device = devices[0];

    // 2c) Create context and command queue
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    // ---------------- 3) Build the program from source ----------------
    const char* source = golKernelSrc;
    size_t sourceSize = std::strlen(source);

    cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize, &err);
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        // Print build log on error
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string buildLog(logSize, ' ');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
        std::cerr << "Build error:\n" << buildLog << std::endl;
        return 1;
    }

    // 3b) Create kernel
    cl_kernel kernel = clCreateKernel(program, "evolveToroidal", &err);

    // ---------------- 4) Create buffers ----------------
    cl_mem currentBuf = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * gridSize,
        currentGrid.data(), &err
    );

    cl_mem nextBuf = clCreateBuffer(
        context, CL_MEM_READ_WRITE,
        sizeof(int) * gridSize,
        nullptr, &err
    );

    // ---------------- 5) Set kernel args ----------------
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &currentBuf);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &nextBuf);
    err = clSetKernelArg(kernel, 2, sizeof(int), &width);
    err = clSetKernelArg(kernel, 3, sizeof(int), &height);

    // ---------------- 6) Enqueue kernel ----------------
    // We'll run a 2D NDRange: global size = (width, height)
    size_t globalWorkSize[2] = { (size_t)width, (size_t)height };
    err = clEnqueueNDRangeKernel(
        queue, kernel,
        2,    // work_dim (2D)
        nullptr,       // global_work_offset
        globalWorkSize,// global_work_size
        nullptr,       // local_work_size
        0, nullptr, nullptr
    );

    // ---------------- 7) Read result back ----------------
    err = clEnqueueReadBuffer(
        queue, nextBuf, CL_TRUE, // blocking read
        0, sizeof(int) * gridSize,
        nextGrid.data(),
        0, nullptr, nullptr
    );

    // ---------------- 8) Cleanup resources ----------------
    clReleaseMemObject(currentBuf);
    clReleaseMemObject(nextBuf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    // ---------------- Print the result ----------------
    std::cout << "Next Generation (GPU-computed):\n";
    printGrid(nextGrid, width, height);

    return 0;
}
