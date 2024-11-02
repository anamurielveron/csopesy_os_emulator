#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Utils.h"
#include "Screen.h"
#include <unordered_map>
#include <string>

class ConsoleManager;
class Screen;

// Screen Manager
class ScreenManager {
private:
    ConsoleManager& consoleManager;             // reference to the console manager
public:
    std::unordered_map<String, Screen> screens; // list of screens
    String lastScreenListOutput;
    String currentScreen;                  // current screen displayed
    ScreenManager(ConsoleManager& cm);
    void screenCreate(const String& name);     // create screen
    void screenRestore(const String& name);    // inspect screen
    void screenList();                         // display screen list
};

#endif // SCREENMANAGER_H
