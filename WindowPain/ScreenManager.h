#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Utils.h"
#include "Screen.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include <unordered_map>
#include <string>

class ConsoleManager;
class Screen;

// Screen Manager
class ScreenManager {
private:
    ConsoleManager& consoleManager;             // reference to the console manager
    Scheduler* scheduler;                            // pointer to Scheduler
    MemoryManager memoryManager;
public:
    std::unordered_map<String, Screen> screens; // list of screens
    String currentScreen;                  // current screen displayed
    ScreenManager(ConsoleManager& cm);
    void screenCreate(const String& name, const String& type);     // create screen
    void screenRestore(const String& name);    // inspect screen
    void screenList(const String& type);              // display screen list
    void schedulerTest();                            // Method to start the scheduler
    void schedulerStop();
    void initialize();
    void loadConfig(const String& filename);
    std::atomic<bool> testRunning{ false };
    std::atomic<bool> schedulerRunning{ false };
    std::thread schedulerThread;

    void addProcessToMemory(Screen& screen);
    void removeProcessFromMemory(Screen& screen);
};

#endif // SCREENMANAGER_H
