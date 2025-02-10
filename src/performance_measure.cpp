#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

int main() {
    std::vector<std::pair<int, int>> gridSizes = {
        {10,10}, {20,20}, {100,100}, 
        {1000,1000}, {10000,10000}
    };

    std::ofstream csvFile("simulation_results.csv");
    csvFile << "Width,Height,Elapsed Time (s)\n";

    for (const auto& grid : gridSizes) {
        int width = grid.first;
        int height = grid.second;
        int generations = 1;

        std::cout << "Testing " << width << "x" << height << " grid...\n";
        
        std::stringstream cmd;
#ifdef _WIN32
        cmd << "gol_opencl.exe " << width << " " << height << " " << generations;
#else
        cmd << "./gol_opencl " << width << " " << height << " " << generations;
#endif

        auto start = std::chrono::steady_clock::now();
        int ret = system(cmd.str().c_str());
        auto end = std::chrono::steady_clock::now();

        if (ret == 0) {
            auto duration = std::chrono::duration<double>(end - start);
            csvFile << width << "," << height << "," 
                   << std::fixed << std::setprecision(6) 
                   << duration.count() << "\n";
            std::cout << "Time: " << duration.count() << "s\n";
        } else {
            std::cerr << "Failed to run OpenCL evolution\n";
        }
    }
    csvFile.close();
    return 0;
}
