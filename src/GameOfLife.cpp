#include "../include/GameOfLife.h"


GameOfLife::GameOfLife(size_t width, size_t height)
    : m_width(width), m_height(height)
{
    m_currentGrid.resize(m_width * m_height, 0);
    m_nextGrid.resize(m_width * m_height, 0);
}

// Count neighbors with toroidal wrapping
int GameOfLife::countNeighbors(size_t x, size_t y) const
{
    int count = 0;

    // For clarity, define neighbor coordinate offsets
    // relative to the current cell
    const int offsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0},           {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    for (auto& off : offsets)
    {
        // Wrap horizontally
        size_t nx = (x + off[0] + m_width)  % m_width;
        // Wrap vertically
        size_t ny = (y + off[1] + m_height) % m_height;

        count += m_currentGrid[index(nx, ny)];
    }

    return count;
}

void GameOfLife::evolveScalar()
{
    // Compute next generation in m_nextGrid
    for (size_t y = 0; y < m_height; ++y)
    {
        for (size_t x = 0; x < m_width; ++x)
        {
            int neighbors = countNeighbors(x, y);
            int currentState = m_currentGrid[index(x, y)];
            int nextState = 0;

            // Conway's rules:
            if (currentState == 1)
            {
                // A live cell with 2 or 3 neighbors stays alive
                nextState = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            }
            else
            {
                // A dead cell becomes alive if it has exactly 3 neighbors
                nextState = (neighbors == 3) ? 1 : 0;
            }

            m_nextGrid[index(x, y)] = nextState;
        }
    }

    // Swap current and next for the next iteration
    m_currentGrid.swap(m_nextGrid);
}

void GameOfLife::print() const
{
    for (size_t y = 0; y < m_height; ++y)
    {
        for (size_t x = 0; x < m_width; ++x)
        {
            std::cout << (m_currentGrid[index(x, y)] ? "*" : ".");
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


void GameOfLife::randomize(double aliveProbability)
{
    for (size_t y = 0; y < m_height; ++y)
    {
        for (size_t x = 0; x < m_width; ++x)
        {
            double r = static_cast<double>(rand()) / RAND_MAX;
            m_currentGrid[index(x, y)] = (r < aliveProbability) ? 1 : 0;
        }
    }
}

void GameOfLife::setCellState(size_t x, size_t y, int state)
{
    if (x < m_width && y < m_height)
    {
        m_currentGrid[index(x, y)] = state;
    }
}

int GameOfLife::getCellState(size_t x, size_t y) const
{
    if (x < m_width && y < m_height)
    {
        return m_currentGrid[index(x, y)];
    }
    return 0; // Out of range -> just return 0
}
