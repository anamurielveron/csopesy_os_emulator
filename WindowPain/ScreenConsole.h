#ifndef SCREENCONSOLE_H
#define SCREENCONSOLE_H

#include "AConsole.h"
#include "ScreenManager.h"
#include "ConsoleManager.h"

// Screen Console
class ScreenConsole : public AConsole {
private:
    ConsoleManager& consoleManager; // reference to the console manager
    ScreenManager& screenManager;   // reference to the screen manager
    void help();            // list all commands for screen console
    void clear();           // redraws the screen console
    void exitScreen();      // goes back to main menu console
    void processSMI();      // prints information of the screen process
public:
    ScreenConsole(ScreenManager& sm, ConsoleManager& cm);
    void draw() override;   // draws the screen console
};

#endif // SCREENCONSOLE_H
