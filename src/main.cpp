#include "../include/GameOfLife.h"
#include <iostream>

int main()
{
    // Create a 10x10 world
    GameOfLife gol(10, 10);

    // Randomly initialize some cells
    gol.randomize(0.3);

    std::cout << "Initial State:" << std::endl;
    gol.print();

    // Evolve 5 generations
    for (int i = 1; i <= 5; ++i)
    {
        gol.evolveScalar();
        std::cout << "Generation " << i << ":" << std::endl;
        gol.print();
    }

    return 0;
}
