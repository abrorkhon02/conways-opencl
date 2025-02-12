#include "../include/GameOfLife.h"
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <iostream>

GameOfLife::GameOfLife(size_t width, size_t height)
    : m_width(width), m_height(height)
{
    m_currentGrid.resize(m_width * m_height, 0);
    m_nextGrid.resize(m_width * m_height, 0);
}

GameOfLife::GameOfLife(const std::string &filename) {
    std::ifstream infile(filename);
    if (!infile.is_open())
        throw std::runtime_error("Failed to open file: " + filename);
    
    // Read dimensions with error checking
    if (!(infile >> m_width >> m_height))
        throw std::runtime_error("Invalid file format: width and height not found.");
    
    if (m_width == 0 || m_height == 0)
        throw std::runtime_error("Invalid dimensions in file: width and height must be > 0.");
    
    m_currentGrid.resize(m_width * m_height, 0);
    m_nextGrid.resize(m_width * m_height, 0);
    
    // Read cell states
    for (size_t i = 0; i < m_width * m_height; ++i) {
        int cellValue = 0;
        if (!(infile >> cellValue))
            throw std::runtime_error("Not enough cell values in file. Expected " +
                                     std::to_string(m_width * m_height) + " values.");
        m_currentGrid[i] = cellValue;
    }
    infile.close();
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
    
    // Write dimensions on the first line
    ofs << m_width << " " << m_height << "\n";
    
    // Write the grid row by row
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
