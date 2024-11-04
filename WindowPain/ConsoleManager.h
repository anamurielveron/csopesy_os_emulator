#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include "Utils.h"
#include "AConsole.h"
#include "ScreenManager.h"
#include <memory>
#include <unordered_map>
#include <string>

enum class ConsoleType { MainMenu, Screen };

// Console Manager
class ConsoleManager {
private:
    std::unique_ptr<AConsole> currentConsole;       // pointer to current console
    std::unique_ptr<ScreenManager> screenManager;   // pointer to ScreenManager
    ConsoleType currentConsoleType;                 // current console type
public:
    ConsoleManager();
    ScreenManager& getScreenManager();
    ConsoleType getCurrentConsoleType();
    void switchConsole(ConsoleType consoleType);    // switch to a specified console
    void passCommand(const String& command);        // passes command to the current console
};

#endif // CONSOLEMANAGER_H