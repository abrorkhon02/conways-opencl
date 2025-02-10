#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <cstring> 

#ifdef DISABLE_PRINT
#define SHOULD_PRINT 0
#else
#define SHOULD_PRINT 1
#endif

// -------------- KERNEL: Embedded as a string literal --------------
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
            if (dx == 0 && dy == 0) continue;
            int nx = (x + dx + width) % width;
            int ny = (y + dy + height) % height;
            count += currentGrid[ INDEXFN(nx, ny, width) ];
        }
    }

    int currentState = currentGrid[ INDEXFN(x, y, width) ];
    int nextState = 0;
    if (currentState == 1) {
        nextState = ((count == 2) || (count == 3)) ? 1 : 0;
    } else {
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

int main(int argc, char* argv[]) {
    if(argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <width> <height> <generations>\n";
        return 1;
    }
    
    int width = std::atoi(argv[1]);
    int height = std::atoi(argv[2]);
    int generations = std::atoi(argv[3]);
    size_t gridSize = width * height;

    // Setup grid data (host)
    std::vector<int> hostGrid(gridSize, 0);
    for (size_t i = 0; i < gridSize; i++) {
        hostGrid[i] = ((rand() % 100) < 30) ? 1 : 0;
    }

    if (SHOULD_PRINT) {
        std::cout << "Initial Grid:\n";
        printGrid(hostGrid, width, height);
    }

    cl_int err = 0;

    // Get the first available platform
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (numPlatforms == 0) {
        std::cerr << "No OpenCL platforms found.\n";
        return 1;
    }
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    cl_platform_id platform = platforms[0];

    // Get a device (GPU or CPU)
    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    std::vector<cl_device_id> devices;
    if(numDevices == 0) {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if(numDevices == 0) {
            std::cerr << "No GPU/CPU devices found.\n";
            return 1;
        }
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices.data(), nullptr);
    } else {
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
    }
    cl_device_id device = devices[0];

    // Create context and command queue
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    cl_queue_properties properties[] = {0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, properties, &err);

    // Build the program from source
    const char* source = golKernelSrc;
    size_t sourceSize = std::strlen(source);
    cl_program program = clCreateProgramWithSource(context, 1, &source, &sourceSize, &err);
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string buildLog(logSize, ' ');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
        std::cerr << "Build error:\n" << buildLog << std::endl;
        return 1;
    }

    cl_kernel kernel = clCreateKernel(program, "evolveToroidal", &err);

    // Create buffers
    cl_mem currentBuf = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * gridSize,
        hostGrid.data(), &err
    );
    cl_mem nextBuf = clCreateBuffer(
        context, CL_MEM_READ_WRITE,
        sizeof(int) * gridSize,
        nullptr, &err
    );

    // Set fixed kernel args (width and height)
    err = clSetKernelArg(kernel, 2, sizeof(int), &width);
    err = clSetKernelArg(kernel, 3, sizeof(int), &height);

    size_t globalWorkSize[2] = { static_cast<size_t>(width), static_cast<size_t>(height) };

    // Evolution loop with buffer swapping
    for(int i = 0; i < generations; i++) {
        // Set kernel args for current iteration (buffer pointers may change each loop)
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &currentBuf);
        err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &nextBuf);
        
        err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
        clFinish(queue);

        // Swap buffers: currentBuf <-> nextBuf
        cl_mem temp = currentBuf;
        currentBuf = nextBuf;
        nextBuf = temp;
    }

    // Read final grid from currentBuf (final result is in currentBuf after swapping)
    std::vector<int> finalGrid(gridSize, 0);
    err = clEnqueueReadBuffer(queue, currentBuf, CL_TRUE, 0, sizeof(int) * gridSize, finalGrid.data(), 0, nullptr, nullptr);

    if (SHOULD_PRINT) {
        std::cout << "\nFinal Grid:\n";
        printGrid(finalGrid, width, height);
    }

    // Cleanup resources
    clReleaseMemObject(currentBuf);
    clReleaseMemObject(nextBuf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
