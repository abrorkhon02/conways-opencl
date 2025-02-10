#include "GameOfLife.h"
#include <iostream>

// Class was used for testing saving capabilities due to numerous problems we encountered with Segmentation faults (was in the end bcs of Git Bash's MINGW conflict)
int main() {
    try {
        std::cout << "Creating game..." << std::endl;
        GameOfLife game(10, 10);
        std::cout << "Game created." << std::endl;

        std::cout << "Printing grid..." << std::endl;
        game.print();
        std::cout << "Grid printed." << std::endl;

        std::cout << "Saving game..." << std::endl;
        game.saveToFile("test_save_output.txt");
        std::cout << "World saved successfully to test_save_output.txt" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    std::cout << "Exiting main..." << std::endl;
    return 0;
}