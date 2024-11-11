#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Utils.h"
#include "Screen.h"
#include "Scheduler.h"
#include <unordered_map>
#include <string>
#include <atomic>

class ConsoleManager;
class Screen;

// Screen Manager
class ScreenManager {
private:
    ConsoleManager& consoleManager;             // reference to the console manager
    Scheduler* scheduler;                            // pointer to Scheduler
public:
    std::atomic<int> cpuCycles{0};
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
};

#endif // SCREENMANAGER_H
