#include "CLI.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <limits>
#include <cstddef>
#include <vector>
#include <functional>



constexpr size_t hash(const char* str) {
    return str[0] ? std::hash<char>{}(str[0]) + 33 * hash(str + 1) : 5381;
}
size_t hash(const std::string& str) {
    return hash(str.c_str());
}

CLI::CLI()
    : world(nullptr), printAfterGeneration(false), delayMs(0)
{
}

CLI::~CLI() {
    delete world;
}

void CLI::run() {
    std::string input;
    printHelp();
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit" || input == "quit") {
            break;
        }
        processCommand(input);
    }
}

void CLI::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token;
    
    switch(hash(token)) {
        case hash("create"):
            createWorld();
            break;
        case hash("load"):
            loadWorld();
            break;
        case hash("save"):
            saveWorld();
            break;
        case hash("run"): {
            std::string mode;
            int generations = 0;
            iss >> mode >> generations;
            runEvolution(mode, generations);
            break;
        }
        case hash("set"):
            setCellState();
            break;
        case hash("get"):
            getCellState();
            break;
        case hash("glider"):
            addGlider();
            break;
        case hash("toad"):
            addToad();
            break;
        case hash("beacon"):
            addBeacon();
            break;
        case hash("methuselah"):
            addMethuselah();
            break;
        case hash("print"): {
            std::string mode;
            iss >> mode;
            if (mode == "on")
                printAfterGeneration = true;
            else if (mode == "off")
                printAfterGeneration = false;
            else
                std::cout << "Please use 'print on' or 'print off'.\n";
            std::cout << "Printing after generation: " << (printAfterGeneration ? "enabled" : "disabled") << std::endl;
            break;
        }
        case hash("delay"): {
            iss >> delayMs;
            std::cout << "Delay set to " << delayMs << " ms.\n";
            break;
        }
        case hash("help"):
            printHelp();
            break;
        case hash("set1d"):
            setCellState1D();
            break;
        case hash("get1d"):
            getCellState1D();
            break;
        default:
            std::cout << "Unknown command. Please enter 'help' for a list of commands.\n";
            break;
    }
}

void CLI::printHelp() const {
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  create          : Create a new world (asks for width and height)" << std::endl;
    std::cout << "  load            : Load world from file (asks for filename)" << std::endl;
    std::cout << "  save            : Save current world to file (asks for filename)" << std::endl;
    std::cout << "  run <mode> <n>  : Run evolution for n generations. Mode: 'scalar' or 'opencl'" << std::endl;
    std::cout << "  set             : Set cell state (asks for x, y and state)" << std::endl;
    std::cout << "  get             : Get cell state (asks for x and y)" << std::endl;
    std::cout << "  glider          : Add a glider pattern" << std::endl;
    std::cout << "  toad            : Add a toad pattern" << std::endl;
    std::cout << "  beacon          : Add a beacon pattern" << std::endl;
    std::cout << "  methuselah      : Add a methuselah pattern" << std::endl;
    std::cout << "  print on/off    : Enable/disable printing after each generation" << std::endl;
    std::cout << "  delay <ms>      : Set delay (ms) for printing" << std::endl;
    std::cout << "  help            : Show this help" << std::endl;
    std::cout << "  exit / quit     : Exit the program\n" << std::endl;
}

void CLI::createWorld() {
    size_t width, height;
    std::cout << "Width: ";
    std::cin >> width;
    std::cout << "Height: ";
    std::cin >> height;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    delete world;
    world = new GameOfLife(width, height);
    std::cout << "New world created (" << width << " x " << height << ")." << std::endl;
}

void CLI::loadWorld() {
    std::string filename;
    std::cout << "Filename to load: ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    try {
        delete world;
        world = new GameOfLife(filename);
        std::cout << "World loaded from '" << filename << "'." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error loading world: " << e.what() << std::endl;
    }
}

void CLI::runEvolution(const std::string& mode, int generations) {
    if (mode == "opencl") {
        if (!world) {
            std::cout << "No world loaded.\n";
            return;
        }
        std::cout << "Launching OpenCL evolution for " << generations << " generation(s)...\n";
        // Construct command with width, height, and generation count
        std::stringstream cmd;
#ifdef _WIN32
        cmd << "gol_opencl.exe " << world->getWidth() << " " << world->getHeight() << " " << generations;
#else
        cmd << "./gol_opencl " << world->getWidth() << " " << world->getHeight() << " " << generations;
#endif
        int ret = system(cmd.str().c_str());
        if(ret != 0) {
            std::cout << "OpenCL execution failed with code " << ret << "\n";
        }
    } else if (mode == "scalar") {
        if(world) {
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < generations; ++i) {
                std::vector<int> prevGrid = world->getCurrentGrid();
                world->evolveScalar();
                if (printAfterGeneration) {
                    world->print();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
                }
                if (world->getCurrentGrid() == prevGrid) {
                    std::cout << "Stable state reached at generation " << (i+1) << ".\n";
                    break;
                }
            }
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration<double>(end - start);
            std::cout << "Scalar evolution completed in " << duration.count() << " seconds.\n";
        } else {
            std::cout << "No world loaded.\n";
        }
    } else {
        std::cout << "Unrecognized mode. Use 'scalar' or 'opencl'.\n";
    }
}

void CLI::setCellState() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t x, y;
    int state;
    std::cout << "x: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cout << "State (0 or 1): ";
    std::cin >> state;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState(x, y, state);
    std::cout << "Cell state at (" << x << ", " << y << ") set.\n";
}

void CLI::getCellState() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t x, y;
    std::cout << "x: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int state = world->getCellState(x, y);
    std::cout << "Cell state at (" << x << ", " << y << ") is: " << state << "\n";
}

void CLI::setCellState1D() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t p;
    int state;
    std::cout << "1D index p: ";
    std::cin >> p;
    std::cout << "State (0 or 1): ";
    std::cin >> state;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState1D(p, state);
    std::cout << "Cell state at index " << p << " set to " << state << "\n";
}

void CLI::getCellState1D() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t p;
    std::cout << "1D index p: ";
    std::cin >> p;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int state = world->getCellState1D(p);
    std::cout << "Cell state at index " << p << " is: " << state << "\n";
}

void CLI::saveWorld() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    std::string filename;
    std::cout << "Filename to save: ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    try {
        world->saveToFile(filename);
        std::cout << "World saved to '" << filename << "'.\n";
    } catch (const std::exception& e) {
        std::cout << "Error saving world: " << e.what() << "\n";
    }
}

void CLI::addGlider() {
    if (!world) {
        std::cout << "No world loaded.\n";
        return;
    }
    size_t x, y;
    std::cout << "Enter (x, y) for glider placement:\n";
    std::cin >> x >> y;
    world->setCellState(x+1, y, 1);
    world->setCellState(x+2, y+1, 1);
    world->setCellState(x,   y+2, 1);
    world->setCellState(x+1, y+2, 1);
    world->setCellState(x+2, y+2, 1);
    std::cout << "Glider added.\n";
}

void CLI::addToad() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t x, y;
    std::cout << "Enter (x, y) for toad placement: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState(x+1, y,   1);
    world->setCellState(x+2, y,   1);
    world->setCellState(x+3, y,   1);
    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);
    world->setCellState(x+2, y+1, 1);
    std::cout << "Toad added.\n";
}

void CLI::addBeacon() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t x, y;
    std::cout << "Enter (x, y) for beacon placement: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState(x,   y,   1);
    world->setCellState(x+1, y,   1);
    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);
    world->setCellState(x+2, y+2, 1);
    world->setCellState(x+3, y+2, 1);
    world->setCellState(x+2, y+3, 1);
    world->setCellState(x+3, y+3, 1);
    std::cout << "Beacon added.\n";
}

void CLI::addMethuselah() {
    if (!world) {
        std::cout << "No world available! Create or load a world first.\n";
        return;
    }
    size_t x, y;
    std::cout << "Enter (x, y) for Methuselah placement: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);
    world->setCellState(x,   y,   1);
    world->setCellState(x+1, y-1, 1);
    world->setCellState(x+2, y,   1);
    std::cout << "Methuselah (R-Pentomino) added.\n";
}
