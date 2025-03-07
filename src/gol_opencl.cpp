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


struct OpenCLResources {
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
    cl_program program = nullptr;
    cl_kernel kernel = nullptr;
    cl_mem currentBuf = nullptr;
    cl_mem nextBuf = nullptr;
    cl_device_id device = nullptr;
};


OpenCLResources setupOpenCL(const std::vector<int>& hostGrid, 
                            int width, 
                            int height, 
                            cl_int &err)
{
    OpenCLResources res;

    cl_uint numPlatforms = 0;
    clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (numPlatforms == 0) {
        throw std::runtime_error("No OpenCL platforms found.");
    }
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    cl_platform_id platform = platforms[0];

    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    std::vector<cl_device_id> devices;
    if(numDevices == 0) {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if(numDevices == 0) {
            throw std::runtime_error("No GPU/CPU devices found.");
        }
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices.data(), nullptr);
    } else {
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
    }
    res.device = devices[0];

    res.context = clCreateContext(nullptr, 1, &res.device, nullptr, nullptr, &err);
    cl_queue_properties properties[] = {0};
    res.queue = clCreateCommandQueueWithProperties(res.context, res.device, properties, &err);

    const char* source = golKernelSrc;
    size_t sourceSize = std::strlen(source);
    res.program = clCreateProgramWithSource(res.context, 1, &source, &sourceSize, &err);
    err = clBuildProgram(res.program, 1, &res.device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(res.program, res.device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string buildLog(logSize, ' ');
        clGetProgramBuildInfo(res.program, res.device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
        throw std::runtime_error("Build error:\n" + buildLog);
    }

    res.kernel = clCreateKernel(res.program, "evolveToroidal", &err);
  
    size_t gridSize = width * height;
    res.currentBuf = clCreateBuffer(
        res.context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * gridSize,
        hostGrid.data(), &err
    );
    res.nextBuf = clCreateBuffer(
        res.context, CL_MEM_READ_WRITE,
        sizeof(int) * gridSize,
        nullptr, &err
    );

    err = clSetKernelArg(res.kernel, 2, sizeof(int), &width);
    err = clSetKernelArg(res.kernel, 3, sizeof(int), &height);

    return res;
}


void runSimulation(OpenCLResources& res, 
                   int width, 
                   int height, 
                   int generations,
                   cl_int &err)
{
    size_t globalWorkSize[2] = { static_cast<size_t>(width), static_cast<size_t>(height) };

    for(int i = 0; i < generations; i++) {
        err = clSetKernelArg(res.kernel, 0, sizeof(cl_mem), &res.currentBuf);
        err = clSetKernelArg(res.kernel, 1, sizeof(cl_mem), &res.nextBuf);
        
        err = clEnqueueNDRangeKernel(res.queue, res.kernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
        clFinish(res.queue);

        cl_mem temp = res.currentBuf;
        res.currentBuf = res.nextBuf;
        res.nextBuf = temp;
    }
}


void cleanupOpenCL(OpenCLResources& res)
{
    if(res.currentBuf) clReleaseMemObject(res.currentBuf);
    if(res.nextBuf) clReleaseMemObject(res.nextBuf);
    if(res.kernel) clReleaseKernel(res.kernel);
    if(res.program) clReleaseProgram(res.program);
    if(res.queue) clReleaseCommandQueue(res.queue);
    if(res.context) clReleaseContext(res.context);
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

    std::vector<int> hostGrid(gridSize, 0);
    for (size_t i = 0; i < gridSize; i++) {
        hostGrid[i] = ((rand() % 100) < 30) ? 1 : 0;
    }

    if (SHOULD_PRINT) {
        std::cout << "Initial Grid:\n";
        printGrid(hostGrid, width, height);
    }

    cl_int err = 0;
    OpenCLResources resources;

    try {
        resources = setupOpenCL(hostGrid, width, height, err);

        runSimulation(resources, width, height, generations, err);

        std::vector<int> finalGrid(gridSize, 0);
        err = clEnqueueReadBuffer(resources.queue, resources.currentBuf, CL_TRUE, 0,
                                  sizeof(int) * gridSize, finalGrid.data(), 0, nullptr, nullptr);

        if (SHOULD_PRINT) {
            std::cout << "\nFinal Grid:\n";
            printGrid(finalGrid, width, height);
        }
    }
    catch (const std::exception &ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        cleanupOpenCL(resources);
        return 1;
    }

    cleanupOpenCL(resources);

    return 0;
}
