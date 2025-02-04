#include <iostream>
#include <vector>
#include <tuple>
#include <chrono>
#include <cstdlib>   
#include <fstream>
#include <sstream>
#include <iomanip>



int main() {

    std::vector<std::pair<int, int>> gridSizes = {
        {10,10},
        {20,20},
        {100,100},
        {1000,1000},
        {10000,10000},
    };


    // Number of generations
    const std::string generations = "1";

    //Path for executable file
    const std::string executable = "./gol_opencl.exe";

    std::vector<std::tuple<int, int,double>> results;

    std::cout << "Starting the simulation for different grid sizes... \n\n";

    for (const auto& grid : gridSizes) {
        int width = grid.first;
        int height = grid.second;


        std::stringstream cmdStream;
        cmdStream << executable << " " << width << " " << height << " " << generations;
        std::string command = cmdStream.str();

        std::cout << "Starting simulation for grid " << width << "x" << height << "...\n";

        auto start = std::chrono::steady_clock::now();


        int retCode = system(command.c_str());
      if (retCode != 0) {
            std::cerr << "Error in execution " << command << "\n";
            continue;  
        }

        auto end = std::chrono::steady_clock::now();

          std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Required time: " << std::fixed << std::setprecision(6)
                  << elapsed_seconds.count() << " Sekunden\n\n";

        // saving 
        results.push_back(std::make_tuple(width, height, elapsed_seconds.count()));
    }

    // Saving results in CSV-File

    std::ofstream csvFile("simulation_results.csv");
    if (!csvFile.is_open()) {
        std::cerr << "Error opening simulation_results.csv \n";
        return 1;
    }

    // CSV-Header
    csvFile << "Width,Height,Elapsed Time (s)\n";
    // Write down all results
    for (const auto& result : results) {
        int width, height;
        double elapsed;
        std::tie(width, height, elapsed) = result;
        csvFile << width << "," << height << "," << std::fixed << std::setprecision(6) << elapsed << "\n";
    }
    csvFile.close();

    std::cout << "Events saved in 'simulation_results.csv'.\n";
    return 0;

    }
