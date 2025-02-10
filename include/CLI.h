#ifndef CLI_H
#define CLI_H

#include "GameOfLife.h"
#include <string>

class CLI {
public:
    CLI();
    ~CLI();

    void run();

private:
    GameOfLife* world;
    bool printAfterGeneration;
    int delayMs; // delay in milliseconds

    void processCommand(const std::string& command);
    void printHelp() const;
    void createWorld();
    void loadWorld();
    void saveWorld();
    void runEvolution(const std::string& mode, int generations);
    void setCellState();
    void getCellState();
    void setCellState1D();
    void getCellState1D();
    void addGlider();
    void addToad();
    void addBeacon();
    void addMethuselah();
};

#endif // CLI_H
