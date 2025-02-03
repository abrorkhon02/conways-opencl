#include "CLI.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <limits> // Für numeric_limits
#include <cstddef> 

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
    
    if (token == "create") {
        createWorld();
    }
    else if (token == "load") {
        loadWorld();
    }
    else if (token == "save") {
        saveWorld();
    }
    else if (token == "run") {
        // Optionale Erweiterung: Erlaubt Auswahl zwischen scalar und opencl z. B. "run scalar 100"
        std::string mode;
        int generations = 0;
        iss >> mode >> generations;
        if (mode == "scalar") {
            // Für scalar verwenden wir die vorhandene Methode
            if (!world) {
                std::cout << "Erstelle oder lade zuerst eine Welt." << std::endl;
                return;
            }
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < generations; ++i) {
                world->evolveScalar();
                if (printAfterGeneration) {
                    world->print();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
                }
                // Hier könnte man eine Stabilitätsprüfung einfügen.
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "Scalar-Evolution dauerte " << duration.count() << " ms." << std::endl;
        }
        else if (mode == "opencl") {
            // Hier wäre der Aufruf der OpenCL-Version der Evolution denkbar,
            // z.B. world->evolveOpenCL() oder ein separater Funktionsaufruf.
            std::cout << "OpenCL-Modus noch nicht implementiert." << std::endl;
        }
        else {
            std::cout << "Unbekannter Modus. Bitte 'scalar' oder 'opencl' angeben." << std::endl;
        }
    }
    else if (token == "set") {
        setCellState();
    }
    else if (token == "get") {
        getCellState();
    }
    else if (token == "glider") {
        addGlider();
    }
    else if (token == "toad") {
        addToad();
    }
    else if (token == "beacon") {
        addBeacon();
    }
    else if (token == "methuselah") {
        addMethuselah();
    }
    else if (token == "print") {
        std::string mode;
        iss >> mode;
        if (mode == "on")
            printAfterGeneration = true;
        else if (mode == "off")
            printAfterGeneration = false;
        else
            std::cout << "Bitte 'print on' oder 'print off' verwenden." << std::endl;
        std::cout << "Ausgabe nach Generationen: " << (printAfterGeneration ? "aktiviert" : "deaktiviert") << std::endl;
    }
    else if (token == "delay") {
        iss >> delayMs;
        std::cout << "Verzögerung auf " << delayMs << " ms gesetzt." << std::endl;
    }
    else if (token == "help") {
        printHelp();
    }
    else if (token == "set1d") {
        setCellState1D();
    }   
    else if (token == "get1d") {
        getCellState1D();
    }
    else {
        std::cout << "Unbekannter Befehl. Bitte 'help' für die Befehlsliste eingeben." << std::endl;
    }  
}

void CLI::printHelp() const {
    std::cout << "\nVerfügbare Befehle:" << std::endl;
    std::cout << "  create          : Erzeuge eine neue Welt (Fragt nach Breite und Höhe)" << std::endl;
    std::cout << "  load            : Lade Welt aus Datei (Fragt nach Dateinamen)" << std::endl;
    std::cout << "  save            : Speichere aktuelle Welt in Datei (Fragt nach Dateinamen)" << std::endl;
    std::cout << "  run <mode> <n>  : Lasse n Generationen laufen. Mode: 'scalar' oder 'opencl'" << std::endl;
    std::cout << "  set             : Setze einen Zellzustand (fragt x, y und Zustand ab)" << std::endl;
    std::cout << "  get             : Frage einen Zellzustand ab (fragt x und y ab)" << std::endl;
    std::cout << "  glider          : Füge ein Glider-Muster ein" << std::endl;
    std::cout << "  toad            : Füge ein Toad-Muster ein" << std::endl;
    std::cout << "  beacon          : Füge ein Beacon-Muster ein" << std::endl;
    std::cout << "  methuselah      : Füge ein Methuselah-Muster ein" << std::endl;
    std::cout << "  print on/off    : Schalte die Ausgabe nach jeder Generation an/aus" << std::endl;
    std::cout << "  delay <ms>      : Setze die Verzögerung in Millisekunden" << std::endl;
    std::cout << "  help            : Zeige diese Hilfe an" << std::endl;
    std::cout << "  exit / quit     : Beende das Programm\n" << std::endl;
}

void CLI::createWorld() {
    size_t width, height;
    std::cout << "Breite: ";
    std::cin >> width;
    std::cout << "Höhe: ";
    std::cin >> height;
    // Eingabepuffer leeren
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    delete world;
    world = new GameOfLife(width, height);
    std::cout << "Neue Welt erstellt (" << width << " x " << height << ")." << std::endl;
}

void CLI::loadWorld() {
    std::string filename;
    std::cout << "Dateiname zum Laden: ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    try {
        delete world;
        world = new GameOfLife(filename);
        std::cout << "Welt aus '" << filename << "' geladen." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Fehler beim Laden: " << e.what() << std::endl;
    }
}

void CLI::runEvolution() {
    // Diese Funktion wird im "run"-Befehl bereits abgehandelt, da hier zwischen
    // scalar und opencl unterschieden wird.
    // Falls du eine gemeinsame Logik haben möchtest, könntest du hier eine Wrapper-Funktion
    // implementieren.
}

void CLI::setCellState() {
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t x, y;
    int state;
    std::cout << "x: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cout << "Zustand (0 oder 1): ";
    std::cin >> state;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    world->setCellState(x, y, state);
    std::cout << "Zellzustand an (" << x << ", " << y << ") gesetzt." << std::endl;
}

void CLI::getCellState() {
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t x, y;
    std::cout << "x: ";
    std::cin >> x;
    std::cout << "y: ";
    std::cin >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    int state = world->getCellState(x, y);
    std::cout << "Zellzustand an (" << x << ", " << y << ") ist: " << state << std::endl;
}

void CLI::setCellState1D()
{
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t p;
    int state;
    std::cout << "Eindimensionaler Index p: ";
    std::cin >> p;
    std::cout << "Zustand (0 oder 1): ";
    std::cin >> state;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    world->setCellState1D(p, state);
    std::cout << "Zellzustand an Index " << p << " gesetzt auf " << state << std::endl;
}

void CLI::getCellState1D()
{
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t p;
    std::cout << "Eindimensionaler Index p: ";
    std::cin >> p;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    int state = world->getCellState1D(p);
    std::cout << "Zellzustand an Index " << p << " ist: " << state << std::endl;
}

void CLI::saveWorld()
{
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }

    std::string filename;
    std::cout << "Dateiname zum Speichern: ";
    std::cin >> filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    try {
        world->saveToFile(filename);
        std::cout << "Welt erfolgreich in '" << filename << "' gespeichert." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Fehler beim Speichern: " << e.what() << std::endl;
    }
}

void CLI::addGlider() {
    if (!world) {
        std::cout << "Keine Welt geladen/erstellt.\n";
        return;
    }
    size_t x, y;
    std::cout << "Eckpunkt (x, y) fuer den Glider angeben:\n";
    std::cin >> x >> y;
    world->setCellState(x+1, y, 1);
    world->setCellState(x+2, y+1, 1);
    world->setCellState(x,   y+2, 1);
    world->setCellState(x+1, y+2, 1);
    world->setCellState(x+2, y+2, 1);

    std::cout << "Glider hinzugefuegt.\n";
}

void CLI::addToad() {
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t x, y;
    std::cout << "Position (x, y) fuer Toad eingeben: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Annahme, dass (x+2) und (y+1) noch im Grid sind
    world->setCellState(x+1, y,   1);
    world->setCellState(x+2, y,   1);
    world->setCellState(x+3, y,   1);

    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);
    world->setCellState(x+2, y+1, 1);

    std::cout << "Toad hinzugefuegt.\n";
}


void CLI::addBeacon() {
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t x, y;
    std::cout << "Position (x, y) fuer Beacon eingeben: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Achtung auf Grid-Grenzen
    world->setCellState(x,   y,   1);
    world->setCellState(x+1, y,   1);
    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);

    world->setCellState(x+2, y+2, 1);
    world->setCellState(x+3, y+2, 1);
    world->setCellState(x+2, y+3, 1);
    world->setCellState(x+3, y+3, 1);

    std::cout << "Beacon hinzugefuegt.\n";
}


void CLI::addMethuselah() {
    if (!world) {
        std::cout << "Keine Welt vorhanden! Erstelle oder lade zuerst eine Welt." << std::endl;
        return;
    }
    size_t x, y;
    std::cout << "Position (x, y) fuer Methuselah eingeben: ";
    std::cin >> x >> y;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // R-Pentomino (eine mögliche Ausrichtung):
    world->setCellState(x,   y+1, 1);
    world->setCellState(x+1, y+1, 1);
    world->setCellState(x,   y,   1);
    world->setCellState(x+1, y-1, 1); // Achtung: y-1 muss >= 0 sein
    world->setCellState(x+2, y,   1);

    std::cout << "Methuselah (z.B. R-Pentomino) hinzugefuegt.\n";
}
