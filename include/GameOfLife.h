#pragma once
#include <vector>
#include <string>
#include <CL/cl.h> 

class GameOfLife {
private:
    size_t m_width;
    size_t m_height;
    std::vector<int> m_currentGrid;
    std::vector<int> m_nextGrid;
    
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem currentBuffer;
    cl_mem nextBuffer;
    cl_device_id device;
    bool openclInitialized;
    
    int countNeighbors(size_t x, size_t y) const;
    size_t cellIndex(size_t x, size_t y) const; 
    
    bool initializeOpenCL();
    void cleanupOpenCL();

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
