#pragma once
#include <vector>
#include <string>

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <iostream>

class GameOfLife {
private:
    size_t m_width;
    size_t m_height;
    std::vector<int> m_currentGrid;
    std::vector<int> m_nextGrid;
    
    // OpenCL resources
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem currentBuffer;
    cl_mem nextBuffer;
    cl_device_id device;
    bool openclInitialized;
    
    // Core methods
    int countNeighbors(size_t x, size_t y) const;
    size_t cellIndex(size_t x, size_t y) const;
    
    // OpenCL methods
    bool initializeOpenCL();
    void cleanupOpenCL();
    bool createOpenCLBuffers(cl_mem& inBuffer, cl_mem& outBuffer, size_t gridSize);
    bool runOpenCLKernel(cl_mem inBuffer, cl_mem outBuffer);
    bool readOpenCLResults(cl_mem buffer, size_t gridSize);
    void printCLError(cl_int err, const char* operation);

public:
    GameOfLife(size_t width, size_t height);
    GameOfLife(const std::string &filename);
    ~GameOfLife();
    
    void evolveScalar();
    bool evolveOpenCL(int generations = 1);
    
    void print() const;
    void randomize(double aliveProbability = 0.3);
    
    void setCellState(size_t x, size_t y, int state);
    int getCellState(size_t x, size_t y) const;
    void setCellState1D(size_t idx, int state);
    int getCellState1D(size_t idx) const;
    void saveToFile(const std::string &filename);
    
    size_t getWidth() const;
    size_t getHeight() const;
    const std::vector<int>& getCurrentGrid() const;
};
