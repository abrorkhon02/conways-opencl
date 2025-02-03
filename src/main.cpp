#include "../include/GameOfLife.h"
#include <iostream>

int main()
{
    GameOfLife gol(10, 10);
    gol.randomize(0.3);

    std::cout << "Initial State:" << std::endl;
    gol.print();

    for (int i = 1; i <= 5; ++i)
    {
        gol.evolveScalar();
        std::cout << "Generation " << i << ":" << std::endl;
        gol.print();
    }

    return 0;
}
