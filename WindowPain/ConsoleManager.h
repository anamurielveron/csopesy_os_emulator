#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <memory>
#include <unordered_map>
#include <string>

class AConsole;
class ScreenManager;
enum class ConsoleType { MainMenu, Screen };

class ConsoleManager {
private:
    std::unique_ptr<AConsole> currentConsole;
    std::unique_ptr<ScreenManager> screenManager;
    ConsoleType currentConsoleType;
public:
    ConsoleManager();
    ScreenManager& getScreenManager();
    ConsoleType getCurrentConsoleType();
    void switchConsole(ConsoleType consoleType);
    void passCommand(const std::string& command);
};

#endif // CONSOLEMANAGER_H