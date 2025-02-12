#ifndef GAMEOFLIFE_H
#define GAMEOFLIFE_H

#include <vector>
#include <string>

class GameOfLife {
public:
    // Constructors
    GameOfLife(size_t width, size_t height);
    GameOfLife(const std::string &filename);

    // Methods
    void evolveScalar();
    void print() const;
    void randomize(double aliveProbability);
    void setCellState(size_t x, size_t y, int state);
    int getCellState(size_t x, size_t y) const;
    void setCellState1D(size_t idx, int state);
    int getCellState1D(size_t idx) const;
    void saveToFile(const std::string &filename);

    // Getters
    size_t getWidth() const;
    size_t getHeight() const;
    const std::vector<int>& getCurrentGrid() const;

private:
    size_t m_width;
    size_t m_height;
    std::vector<int> m_currentGrid;
    std::vector<int> m_nextGrid;

    size_t cellIndex(size_t x, size_t y) const { return y * m_width + x; }
    int countNeighbors(size_t x, size_t y) const;
};

#endif
