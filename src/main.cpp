#include <cstdlib>
#include <clocale>
#include "../include/GameOfLife.h"
#include "CLI.h"
#include <iostream>

int main() {
    system("chcp 65001");
    std::setlocale(LC_ALL, "");
    
    CLI cli;
    cli.run();
    return 0;
}
