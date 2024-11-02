#ifndef SCREENCONSOLE_H
#define SCREENCONSOLE_H

#include "AConsole.h"
#include "ScreenManager.h"
#include "ConsoleManager.h"

class ScreenConsole : public AConsole {
private:
    ConsoleManager& consoleManager;
    ScreenManager& screenManager;
    void help();
    void screen();
    void clear();
    void exitScreen();
public:
    ScreenConsole(ScreenManager& sm, ConsoleManager& cm);
    void draw() override;
};

#endif // SCREENCONSOLE_H
