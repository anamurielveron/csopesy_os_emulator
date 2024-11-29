#ifndef MAINMENUCONSOLE_H
#define MAINMENUCONSOLE_H

#include "Utils.h"
#include "AConsole.h"
#include "ScreenManager.h"
#include "ConsoleManager.h"
#include "Scheduler.h"

// Main Menu Console
class MainMenuConsole : public AConsole {
private:
    ConsoleManager& consoleManager;   // reference to the console manager
    ScreenManager& screenManager;   // reference to the screen manager

    void printTitle();      // prints the main menu title
    void help();            // list all commands for main menu console
    void initialize();      // N/A
    void screen();          // lists the commands for screen
    void schedulerTest();   // N/A
    void schedulerStop();   // N/A
    void reportUtil();      // N/A
    void processSMI();
    void VMstat();
    void clear();           // redraws the screen console
    void exitProgram();     // exits the program

public:
    MainMenuConsole(ScreenManager& sm, ConsoleManager& cm);
    void draw() override;   // draws the main menu console
};

#endif // MAINMENUCONSOLE_H