#include "../include/GameOfLife.h"
#include "../include/SharedGlobals.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>

static const char *golKernelSource = R"CLC(
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

GameOfLife::GameOfLife(size_t width, size_t height)
    : m_width(width), m_height(height), openclInitialized(false)
{
    m_currentGrid.resize(m_width * m_height, 0);
    m_nextGrid.resize(m_width * m_height, 0);

    context = nullptr;
    queue = nullptr;
    program = nullptr;
    kernel = nullptr;
    currentBuffer = nullptr;
    nextBuffer = nullptr;
    device = nullptr;
}

GameOfLife::GameOfLife(const std::string &filename) {
    std::ifstream infile(filename);
    if (!infile.is_open())
        throw std::runtime_error("Failed to open file: " + filename);
    
    if (!(infile >> m_width >> m_height))
        throw std::runtime_error("Invalid file format: width and height not found.");
    
    if (m_width == 0 || m_height == 0)
        throw std::runtime_error("Invalid dimensions in file: width and height must be > 0.");
    
    m_currentGrid.resize(m_width * m_height, 0);
    m_nextGrid.resize(m_width * m_height, 0);
    
    for (size_t i = 0; i < m_width * m_height; ++i) {
        int cellValue = 0;
        if (!(infile >> cellValue))
            throw std::runtime_error("Not enough cell values in file. Expected " +
                                     std::to_string(m_width * m_height) + " values.");
        m_currentGrid[i] = cellValue;
    }
    infile.close();
}

GameOfLife::~GameOfLife() {
    cleanupOpenCL();
}

int GameOfLife::countNeighbors(size_t x, size_t y) const {
    int count = 0;
    const int offsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},           {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };
    for (const auto &off : offsets) {
        size_t nx = (x + off[0] + m_width) % m_width;
        size_t ny = (y + off[1] + m_height) % m_height;
        count += m_currentGrid[cellIndex(nx, ny)];
    }
    return count;
}

void GameOfLife::evolveScalar() {
    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            int neighbors = countNeighbors(x, y);
            int currentState = m_currentGrid[cellIndex(x, y)];
            int nextState = 0;
            if (currentState == 1)
                nextState = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            else
                nextState = (neighbors == 3) ? 1 : 0;
            m_nextGrid[cellIndex(x, y)] = nextState;
        }
    }
    m_currentGrid.swap(m_nextGrid);
}

void GameOfLife::print() const {
    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            std::cout << (m_currentGrid[cellIndex(x, y)] ? "*" : ".");
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void GameOfLife::randomize(double aliveProbability) {
    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            double r = static_cast<double>(rand()) / RAND_MAX;
            m_currentGrid[cellIndex(x, y)] = (r < aliveProbability) ? 1 : 0;
        }
    }
}

void GameOfLife::setCellState(size_t x, size_t y, int state) {
    if (x < m_width && y < m_height)
        m_currentGrid[cellIndex(x, y)] = state;
}

int GameOfLife::getCellState(size_t x, size_t y) const {
    if (x < m_width && y < m_height)
        return m_currentGrid[cellIndex(x, y)];
    return 0;
}

void GameOfLife::setCellState1D(size_t idx, int state) {
    size_t x = idx % m_width;
    size_t y = idx / m_width;
    setCellState(x, y, state);
}

int GameOfLife::getCellState1D(size_t idx) const {
    size_t x = idx % m_width;
    size_t y = idx / m_width;
    return getCellState(x, y);
}

void GameOfLife::saveToFile(const std::string &filename) {
    std::ofstream ofs(filename.c_str());
    if (!ofs) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }
    
    ofs << m_width << " " << m_height << "\n";
    
    for (size_t y = 0; y < m_height; ++y) {
        for (size_t x = 0; x < m_width; ++x) {
            ofs << m_currentGrid[y * m_width + x];
            if (x < m_width - 1)
                ofs << " ";
        }
        ofs << "\n";
    }
    
    ofs.close();
}

size_t GameOfLife::getWidth() const {
    return m_width;
}

size_t GameOfLife::getHeight() const {
    return m_height;
}

const std::vector<int>& GameOfLife::getCurrentGrid() const {
    return m_currentGrid;
}

void GameOfLife::printCLError(cl_int err, const char* operation) {
    const char* errorString;
    switch (err) {
        case CL_SUCCESS: errorString = "Success"; break;
        case CL_DEVICE_NOT_FOUND: errorString = "Device not found"; break;
        case CL_DEVICE_NOT_AVAILABLE: errorString = "Device not available"; break;
        case CL_COMPILER_NOT_AVAILABLE: errorString = "Compiler not available"; break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: errorString = "Memory allocation failure"; break;
        case CL_OUT_OF_RESOURCES: errorString = "Out of resources"; break;
        case CL_OUT_OF_HOST_MEMORY: errorString = "Out of host memory"; break;
        case CL_PROFILING_INFO_NOT_AVAILABLE: errorString = "Profiling info not available"; break;
        case CL_MEM_COPY_OVERLAP: errorString = "Memory copy overlap"; break;
        case CL_IMAGE_FORMAT_MISMATCH: errorString = "Image format mismatch"; break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: errorString = "Image format not supported"; break;
        case CL_BUILD_PROGRAM_FAILURE: errorString = "Build program failure"; break;
        case CL_MAP_FAILURE: errorString = "Map failure"; break;
        case CL_INVALID_VALUE: errorString = "Invalid value"; break;
        case CL_INVALID_DEVICE_TYPE: errorString = "Invalid device type"; break;
        case CL_INVALID_PLATFORM: errorString = "Invalid platform"; break;
        case CL_INVALID_DEVICE: errorString = "Invalid device"; break;
        case CL_INVALID_CONTEXT: errorString = "Invalid context"; break;
        case CL_INVALID_QUEUE_PROPERTIES: errorString = "Invalid queue properties"; break;
        case CL_INVALID_COMMAND_QUEUE: errorString = "Invalid command queue"; break;
        case CL_INVALID_HOST_PTR: errorString = "Invalid host pointer"; break;
        case CL_INVALID_MEM_OBJECT: errorString = "Invalid memory object"; break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: errorString = "Invalid image format descriptor"; break;
        case CL_INVALID_IMAGE_SIZE: errorString = "Invalid image size"; break;
        case CL_INVALID_SAMPLER: errorString = "Invalid sampler"; break;
        case CL_INVALID_BINARY: errorString = "Invalid binary"; break;
        case CL_INVALID_BUILD_OPTIONS: errorString = "Invalid build options"; break;
        case CL_INVALID_PROGRAM: errorString = "Invalid program"; break;
        case CL_INVALID_PROGRAM_EXECUTABLE: errorString = "Invalid program executable"; break;
        case CL_INVALID_KERNEL_NAME: errorString = "Invalid kernel name"; break;
        case CL_INVALID_KERNEL_DEFINITION: errorString = "Invalid kernel definition"; break;
        case CL_INVALID_KERNEL: errorString = "Invalid kernel"; break;
        case CL_INVALID_ARG_INDEX: errorString = "Invalid argument index"; break;
        case CL_INVALID_ARG_VALUE: errorString = "Invalid argument value"; break;
        case CL_INVALID_ARG_SIZE: errorString = "Invalid argument size"; break;
        case CL_INVALID_KERNEL_ARGS: errorString = "Invalid kernel arguments"; break;
        case CL_INVALID_WORK_DIMENSION: errorString = "Invalid work dimension"; break;
        case CL_INVALID_WORK_GROUP_SIZE: errorString = "Invalid work group size"; break;
        case CL_INVALID_WORK_ITEM_SIZE: errorString = "Invalid work item size"; break;
        case CL_INVALID_GLOBAL_OFFSET: errorString = "Invalid global offset"; break;
        case CL_INVALID_EVENT_WAIT_LIST: errorString = "Invalid event wait list"; break;
        case CL_INVALID_EVENT: errorString = "Invalid event"; break;
        case CL_INVALID_OPERATION: errorString = "Invalid operation"; break;
        case CL_INVALID_GL_OBJECT: errorString = "Invalid OpenGL object"; break;
        case CL_INVALID_BUFFER_SIZE: errorString = "Invalid buffer size"; break;
        case CL_INVALID_MIP_LEVEL: errorString = "Invalid MIP level"; break;
        default: errorString = "Unknown error"; break;
    }
    std::cerr << "OpenCL Error during " << operation << ": " << errorString << " (Code: " << err << ")" << std::endl;
}

bool GameOfLife::initializeOpenCL() {
    if (openclInitialized) return true;
    
    cl_int err = CL_SUCCESS;
    
    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (err != CL_SUCCESS || numPlatforms == 0) {
        printCLError(err, "getting platform count");
        return false;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    if (err != CL_SUCCESS) {
        printCLError(err, "getting platform IDs");
        return false;
    }
    
    char platformName[128];
    clGetPlatformInfo(platforms[0], CL_PLATFORM_NAME, sizeof(platformName), platformName, NULL);
    std::cout << "Using OpenCL platform: " << platformName << std::endl;
    
    cl_platform_id platform = platforms[0];

    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    
    if (err != CL_SUCCESS || numDevices == 0) {
        std::cout << "No GPU found, trying CPU device..." << std::endl;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if (err != CL_SUCCESS || numDevices == 0) {
            printCLError(err, "finding any OpenCL devices");
            return false;
        }
        
        std::vector<cl_device_id> devices(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices.data(), nullptr);
        if (err != CL_SUCCESS) {
            printCLError(err, "getting CPU device IDs");
            return false;
        }
        device = devices[0];
        
        char deviceName[128];
        clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
        std::cout << "Using CPU device: " << deviceName << std::endl;
    } else {
        std::vector<cl_device_id> devices(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
        if (err != CL_SUCCESS) {
            printCLError(err, "getting GPU device IDs");
            return false;
        }
        device = devices[0];
        
        char deviceName[128];
        clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
        std::cout << "Using GPU device: " << deviceName << std::endl;
    }

    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        printCLError(err, "creating context");
        return false;
    }

    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        printCLError(err, "creating command queue");
        cleanupOpenCL();
        return false;
    }

    const char* source = golKernelSource;
    size_t sourceSize = std::strlen(source);
    program = clCreateProgramWithSource(context, 1, &source, &sourceSize, &err);
    if (err != CL_SUCCESS) {
        printCLError(err, "creating program");
        cleanupOpenCL();
        return false;
    }

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string buildLog(logSize, ' ');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
        std::cerr << "OpenCL Build error:\n" << buildLog << std::endl;
        cleanupOpenCL();
        return false;
    }

    kernel = clCreateKernel(program, "evolveToroidal", &err);
    if (err != CL_SUCCESS) {
        printCLError(err, "creating kernel");
        cleanupOpenCL();
        return false;
    }

    err = clSetKernelArg(kernel, 2, sizeof(int), &m_width);
    if (err != CL_SUCCESS) {
        printCLError(err, "setting width kernel argument");
        cleanupOpenCL();
        return false;
    }
    
    err = clSetKernelArg(kernel, 3, sizeof(int), &m_height);
    if (err != CL_SUCCESS) {
        printCLError(err, "setting height kernel argument");
        cleanupOpenCL();
        return false;
    }

    openclInitialized = true;
    return true;
}

void GameOfLife::cleanupOpenCL() {
    if (currentBuffer) clReleaseMemObject(currentBuffer);
    if (nextBuffer) clReleaseMemObject(nextBuffer);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    if (queue) clReleaseCommandQueue(queue);
    if (context) clReleaseContext(context);
    
    currentBuffer = nullptr;
    nextBuffer = nullptr;
    kernel = nullptr;
    program = nullptr;
    queue = nullptr;
    context = nullptr;
    device = nullptr;
    openclInitialized = false;
}

bool GameOfLife::createOpenCLBuffers(cl_mem& inBuffer, cl_mem& outBuffer, size_t gridSize) {
    cl_int err = CL_SUCCESS;
    
    inBuffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * gridSize, m_currentGrid.data(), &err
    );
    
    if (err != CL_SUCCESS) {
        printCLError(err, "creating input buffer");
        return false;
    }
    
    outBuffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE, 
        sizeof(int) * gridSize, nullptr, &err
    );
    
    if (err != CL_SUCCESS) {
        printCLError(err, "creating output buffer");
        clReleaseMemObject(inBuffer);
        return false;
    }
    
    return true;
}

bool GameOfLife::runOpenCLKernel(cl_mem inBuffer, cl_mem outBuffer) {
    cl_int err = CL_SUCCESS;
    
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inBuffer);
    if (err != CL_SUCCESS) {
        printCLError(err, "setting input buffer argument");
        return false;
    }
    
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &outBuffer);
    if (err != CL_SUCCESS) {
        printCLError(err, "setting output buffer argument");
        return false;
    }
    
    size_t globalWorkSize[2] = { m_width, m_height };
    
    size_t maxWorkItems[3];
    err = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, 
                        sizeof(maxWorkItems), maxWorkItems, NULL);
    if (err != CL_SUCCESS) {
        printCLError(err, "getting device work item sizes");
        return false;
    }
    
    if (globalWorkSize[0] > maxWorkItems[0]) globalWorkSize[0] = maxWorkItems[0];
    if (globalWorkSize[1] > maxWorkItems[1]) globalWorkSize[1] = maxWorkItems[1];
    
    err = clEnqueueNDRangeKernel(
        queue, kernel, 2, nullptr, 
        globalWorkSize, nullptr, 
        0, nullptr, nullptr
    );
    
    if (err != CL_SUCCESS) {
        printCLError(err, "enqueueing kernel");
        return false;
    }
    
    err = clFinish(queue);
    if (err != CL_SUCCESS) {
        printCLError(err, "waiting for kernel completion");
        return false;
    }
    
    return true;
}

bool GameOfLife::readOpenCLResults(cl_mem buffer, size_t gridSize) {
    cl_int err = clEnqueueReadBuffer(
        queue, buffer, CL_TRUE, 0,
        sizeof(int) * gridSize, m_currentGrid.data(),
        0, nullptr, nullptr
    );
    
    if (err != CL_SUCCESS) {
        printCLError(err, "reading results");
        return false;
    }
    
    return true;
}

bool GameOfLife::evolveOpenCL(int generations) {
    trace_log("evolveOpenCL called with " + std::to_string(generations) + " generations");
    std::cout << "Starting OpenCL evolution for " << generations << " generations..." << std::endl;
    
    if (m_width == 0 || m_height == 0) {
        trace_log("Error: Invalid grid dimensions");
        std::cerr << "Invalid grid dimensions for OpenCL evolution" << std::endl;
        return false;
    }

    // Only initialize OpenCL when needed
    trace_log("Initializing OpenCL...");
    if (!initializeOpenCL()) {
        trace_log("Error: Failed to initialize OpenCL");
        std::cerr << "Failed to initialize OpenCL" << std::endl;
        return false;
    }
    trace_log("OpenCL initialized successfully");
    
    // Print device capabilities for debugging
    size_t maxWorkGroupSize;
    clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, 
                    sizeof(size_t), &maxWorkGroupSize, NULL);
    trace_log("Device info - Max work group size: " + std::to_string(maxWorkGroupSize));
    std::cout << "Device info - Max work group size: " << maxWorkGroupSize << std::endl;
    
    size_t gridSize = m_width * m_height;
    trace_log("Grid size: " + std::to_string(m_width) + "x" + std::to_string(m_height) + " = " + std::to_string(gridSize) + " cells");
    std::cout << "Grid size: " << m_width << "x" << m_height << " = " << gridSize << " cells" << std::endl;
    
    // Create OpenCL buffers
    trace_log("Creating OpenCL buffers...");
    cl_mem inBuffer = nullptr, outBuffer = nullptr;
    if (!createOpenCLBuffers(inBuffer, outBuffer, gridSize)) {
        trace_log("Error: Failed to create OpenCL buffers");
        std::cerr << "Failed to create OpenCL buffers" << std::endl;
        return false;
    }
    trace_log("OpenCL buffers created successfully");
    
    std::cout << "Starting evolution loop..." << std::endl;
    trace_log("Starting evolution loop...");
    
    // Check if we want to print intermediate generations
    bool shouldPrintGenerations = false;
    extern bool g_printAfterGeneration;  
    extern int g_delayMs;                
    shouldPrintGenerations = g_printAfterGeneration;
    trace_log("Print after generation: " + std::string(shouldPrintGenerations ? "true" : "false"));

    for (int i = 0; i < generations; i++) {
        trace_log("Generation " + std::to_string(i+1) + " of " + std::to_string(generations));
        
        // Execute the kernel with current buffers
        if (!runOpenCLKernel(inBuffer, outBuffer)) {
            trace_log("Error: Failed to run OpenCL kernel for generation " + std::to_string(i+1));
            std::cerr << "Failed to run OpenCL kernel for generation " << i+1 << std::endl;
            clReleaseMemObject(inBuffer);
            clReleaseMemObject(outBuffer);
            return false;
        }
        
        if (shouldPrintGenerations) {
            trace_log("Reading back intermediate results for display");
            if (!readOpenCLResults(outBuffer, gridSize)) {
                trace_log("Error: Failed to read results for intermediate generation");
                std::cerr << "Failed to read results for intermediate generation" << std::endl;
                clReleaseMemObject(inBuffer);
                clReleaseMemObject(outBuffer);
                return false;
            }
            
            print();
            
            if (g_delayMs > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(g_delayMs));
            }
        }
        
        if (i < generations - 1) {
            trace_log("Swapping buffers for next generation");
            cl_mem temp = inBuffer;
            inBuffer = outBuffer;
            outBuffer = temp;
        }
    }
    
    trace_log("Evolution loop completed, reading results...");
    std::cout << "Evolution loop completed, reading results..." << std::endl;
    
    if (!shouldPrintGenerations) {
        trace_log("Reading final results");
        if (!readOpenCLResults(outBuffer, gridSize)) {
            trace_log("Error: Failed to read OpenCL results");
            std::cerr << "Failed to read OpenCL results" << std::endl;
            clReleaseMemObject(inBuffer);
            clReleaseMemObject(outBuffer);
            return false;
        }
    }
    
    // Clean up
    trace_log("Releasing OpenCL buffer objects");
    clReleaseMemObject(inBuffer);
    clReleaseMemObject(outBuffer);
    
    trace_log("OpenCL evolution completed successfully");
    std::cout << "OpenCL evolution completed successfully" << std::endl;
    return true;
}

inline size_t GameOfLife::cellIndex(size_t x, size_t y) const {
    return y * m_width + x;
}
