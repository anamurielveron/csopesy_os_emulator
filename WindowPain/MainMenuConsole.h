#ifndef MAINMENUCONSOLE_H
#define MAINMENUCONSOLE_H

#include "AConsole.h"
#include "ScreenManager.h"
#include "ConsoleManager.h"
#include "Scheduler.h"

class MainMenuConsole : public AConsole {
private:
    ConsoleManager& consoleManager;
    ScreenManager& screenManager;
    Scheduler* scheduler;

    void printTitle();
    void help();
    void initialize();
    void screen();
    void schedulerTest();
    void schedulerStop();
    void reportUtil();
    void clear();
    void exitProgram();

public:
    MainMenuConsole(ScreenManager& sm, ConsoleManager& cm);
    void draw() override;
};

#endif // MAINMENUCONSOLE_H