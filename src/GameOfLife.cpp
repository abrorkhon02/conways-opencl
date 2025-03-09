#include "../include/GameOfLife.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <iostream>
#include <cstring>

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

bool GameOfLife::initializeOpenCL() {
    if (openclInitialized) return true;
    
    cl_int err = CL_SUCCESS;
    
    // Get platform
    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (err != CL_SUCCESS || numPlatforms == 0) {
        std::cerr << "Failed to find any OpenCL platforms." << std::endl;
        return false;
    }

    std::vector<cl_platform_id> platforms(numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    cl_platform_id platform = platforms[0];

    // Get device
    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    std::vector<cl_device_id> devices;
    
    if (numDevices == 0) {
        // Try CPU if no GPU is available
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, nullptr, &numDevices);
        if (numDevices == 0) {
            std::cerr << "No OpenCL devices found." << std::endl;
            return false;
        }
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices.data(), nullptr);
    } else {
        devices.resize(numDevices);
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
    }
    
    device = devices[0];

    // Create context
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create OpenCL context." << std::endl;
        return false;
    }

    // Create command queue - use OpenCL 1.2 version
    // Replace clCreateCommandQueueWithProperties with clCreateCommandQueue
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue." << std::endl;
        cleanupOpenCL();
        return false;
    }

    // Create program
    const char* source = golKernelSource;
    size_t sourceSize = std::strlen(source);
    program = clCreateProgramWithSource(context, 1, &source, &sourceSize, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create program." << std::endl;
        cleanupOpenCL();
        return false;
    }

    // Build program
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::string buildLog(logSize, ' ');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], nullptr);
        std::cerr << "Build error:\n" << buildLog << std::endl;
        cleanupOpenCL();
        return false;
    }

    // Create kernel
    kernel = clCreateKernel(program, "evolveToroidal", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create kernel." << std::endl;
        cleanupOpenCL();
        return false;
    }

    // Set fixed kernel arguments
    err = clSetKernelArg(kernel, 2, sizeof(int), &m_width);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &m_height);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arguments." << std::endl;
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

bool GameOfLife::evolveOpenCL(int generations) {
    if (!initializeOpenCL()) {
        return false;
    }
    
    cl_int err = CL_SUCCESS;
    size_t gridSize = m_width * m_height;
    
    currentBuffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * gridSize, m_currentGrid.data(), &err
    );
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create current buffer." << std::endl;
        cleanupOpenCL();
        return false;
    }
    
    nextBuffer = clCreateBuffer(
        context, CL_MEM_READ_WRITE, sizeof(int) * gridSize, nullptr, &err
    );
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create next buffer." << std::endl;
        if (currentBuffer) clReleaseMemObject(currentBuffer);
        cleanupOpenCL();
        return false;
    }
    
    size_t globalWorkSize[2] = { m_width, m_height };
    
    err = clSetKernelArg(kernel, 2, sizeof(int), &m_width);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &m_height);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to set kernel arguments." << std::endl;
        cleanupOpenCL();
        return false;
    }
    
    for (int i = 0; i < generations; i++) {
        err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &currentBuffer);
        err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &nextBuffer);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to set kernel arguments for iteration." << std::endl;
            if (currentBuffer) clReleaseMemObject(currentBuffer);
            if (nextBuffer) clReleaseMemObject(nextBuffer);
            return false;
        }
        
        err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, globalWorkSize, nullptr, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Failed to execute kernel." << std::endl;
            if (currentBuffer) clReleaseMemObject(currentBuffer);
            if (nextBuffer) clReleaseMemObject(nextBuffer);
            return false;
        }
        
        clFinish(queue);
        
        cl_mem temp = currentBuffer;
        currentBuffer = nextBuffer;
        nextBuffer = temp;
    }
    
    err = clEnqueueReadBuffer(queue, currentBuffer, CL_TRUE, 0, sizeof(int) * gridSize, m_currentGrid.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to read back results." << std::endl;
        if (currentBuffer) clReleaseMemObject(currentBuffer);
        if (nextBuffer) clReleaseMemObject(nextBuffer);
        return false;
    }
    
    if (currentBuffer) clReleaseMemObject(currentBuffer);
    if (nextBuffer) clReleaseMemObject(nextBuffer);
    currentBuffer = nextBuffer = nullptr;
    
    return true;
}

inline size_t GameOfLife::cellIndex(size_t x, size_t y) const {
    return y * m_width + x;
}
