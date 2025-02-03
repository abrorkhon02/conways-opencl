#ifndef CLI_H
#define CLI_H

#include "GameOfLife.h"
#include <string>

class CLI {
public:
    CLI();
    ~CLI();

    // Hauptschleife der Benutzerschnittstelle
    void run();

private:
    GameOfLife* world;  // Aktuelle Simulation (kann auch z. B. smart pointer verwendet werden)
    bool printAfterGeneration; // Steuerung, ob nach jeder Generation ausgegeben wird
    unsigned int delayMs;      // Verzögerung in Millisekunden

    // Kommando-Verarbeitung
    void processCommand(const std::string& command);

    // Funktionen für die einzelnen Befehle:
    void createWorld();  // Neue Welt mit Breite und Höhe erstellen
    void loadWorld();    // Welt aus einer Datei laden
    void saveWorld();    // Welt in eine Datei speichern
    void runEvolution(); // Simulation für n Generationen durchführen
    void setCellState(); // Zellzustand setzen (2D oder 1D)
    void getCellState(); // Zellzustand abfragen (2D oder 1D)
    void addGlider();    // Glider-Muster einfügen
    void addToad();      // Toad-Muster einfügen
    void addBeacon();    // Beacon-Muster einfügen
    void addMethuselah(); // Methuselah-Muster einfügen
    void setCellState1D(); // Neue Funktion, um Zellzustand per 1D-Index zu setzen
    void getCellState1D(); // Neue Funktion, um Zellzustand per 1D-Index abzufragen
    // Hilfsfunktion zur Anzeige der Befehlsübersicht
    void printHelp() const;
};

#endif
