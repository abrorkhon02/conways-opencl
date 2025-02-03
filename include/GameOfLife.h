#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include <vector>
#include <cstdlib>
#include <iostream>

class GameOfLife
{
private:
    size_t m_width;
    size_t m_height;

    std::vector<int> m_currentGrid;
    std::vector<int> m_nextGrid;

    inline size_t index(size_t x, size_t y) const {
        return y * m_width + x;
    }

    int countNeighbors(size_t x, size_t y) const;

public:
    GameOfLife(size_t width, size_t height);

    void evolveScalar();

    void print() const;

    void randomize(double aliveProbability = 0.2);

    size_t getWidth() const { return m_width; }
    size_t getHeight() const { return m_height; }

    void setCellState(size_t x, size_t y, int state);
    int  getCellState(size_t x, size_t y) const;
    GameOfLife(const std::string& filename);
};

#endif
