#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include <unordered_map>
#include <string>

class ConsoleManager;
class Screen;

class ScreenManager {
private:
    ConsoleManager& consoleManager;
public:
    std::unordered_map<std::string, Screen> screens;
    std::string currentScreen;
    ScreenManager(ConsoleManager& cm);
    void screenCreate(const std::string& name);
    void screenRestore(const std::string& name);
    void screenList();
};

#endif // SCREENMANAGER_H
