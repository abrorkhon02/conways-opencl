#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "../include/GameOfLife.h"

int main() {
    std::vector<std::pair<int, int>> gridSizes = {
        {10,10}, {20,20}, {100,100}, 
        {1000,1000}, {10000,10000}
    };

    std::ofstream csvFile("simulation_results.csv");
    csvFile << "Width,Height,Generations,Elapsed Time (s)\n";

    for (const auto& grid : gridSizes) {
        int width = grid.first;
        int height = grid.second;
        int generations = 100; 
        
        std::cout << "Testing " << width << "x" << height << " grid with OpenCL...\n";
        
        try {
            GameOfLife world(width, height);
            world.randomize(0.3);
            auto start = std::chrono::steady_clock::now();
            bool success = world.evolveOpenCL(generations);
            if (!success) {
                std::cerr << "OpenCL evolution failed for " << width << "x" << height << " grid\n";
                continue;
            }
            
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration<double>(end - start);
            
            csvFile << width << "," << height << "," 
                   << generations << "," 
                   << std::fixed << std::setprecision(6) 
                   << duration.count() << "\n";
            
            std::cout << "Time: " << duration.count() << "s\n";
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    
    csvFile.close();
    std::cout << "Results saved to simulation_results.csv\n";
    return 0;
}
