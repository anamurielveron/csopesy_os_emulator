#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "Utils.h"
#include "Screen.h"
#include "Scheduler.h"
#include <map>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

class Scheduler;
class ConsoleManager;
class Screen;

// Screen Manager
class ScreenManager {
private:
    ConsoleManager& consoleManager; // reference to the console manager
    Scheduler* scheduler;           // pointer to Scheduler
public:
    std::map<int, String> orderedScreens;
    std::map<String, Screen> screens; // list of screens
    std::map<String, Screen> activeScreens;
    String currentScreen;                       // current screen displayed
    ScreenManager(ConsoleManager& cm);
    void screenCreate(const String& name, const String& type);
    void screenRestore(const String& name);
    void screenList(const String& type);
    void schedulerTest();
    void schedulerStop();
    void initialize();
    void memoryStamp();
    void loadConfig(const String& filename);
    Scheduler* getScheduler() const;
    std::atomic<bool> testRunning{ false };
    std::atomic<bool> schedulerRunning{ false };
    std::thread schedulerThread;
    int processIdCounter = 0;
    int processDummyCounter = 0;

    // Synchronization of threads
    std::atomic<int> cpuCycles{0};
    std::atomic<int> quantumCycles{0};
    std::mutex mtx;
    std::condition_variable cycleCv;

};

#endif // SCREENMANAGER_H
