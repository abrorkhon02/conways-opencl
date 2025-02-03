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

    // We'll store current and next states in two separate vectors.
    // Each cell is either 0 (dead) or 1 (alive).
    std::vector<int> m_currentGrid;
    std::vector<int> m_nextGrid;

    // Helper to map (x,y) to index in the 1D vector
    inline size_t index(size_t x, size_t y) const {
        return y * m_width + x;
    }

    // Count neighbors with toroidal wrapping
    int countNeighbors(size_t x, size_t y) const;

public:
    // Constructor: create empty or random grid
    GameOfLife(size_t width, size_t height);

    // Evolve one generation in a scalar (CPU-only) way
    void evolveScalar();

    // Print the grid to console
    void print() const;

    // Simple random fill (optional demo usage)
    void randomize(double aliveProbability = 0.2);

    // Accessors
    size_t getWidth() const { return m_width; }
    size_t getHeight() const { return m_height; }

    // Set or get cell state
    void setCellState(size_t x, size_t y, int state);
    int  getCellState(size_t x, size_t y) const;
};

#endif
