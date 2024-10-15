#include "ScreenManager.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include "Utils.h"

#include <iostream>
#include <iomanip>
#include <ctime>

ScreenManager::ScreenManager(ConsoleManager& cm) : consoleManager(cm) {}

void ScreenManager::screenCreate(const std::string& name) {
    if (screens.find(name) != screens.end()) {
        printInColor("Screen already exists with this name.\n", "red");
        return;
    }

    time_t now = time(0);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%m/%d/%Y %I:%M:%S %p", &ltm);

    // Create a new screen
    Screen newScreen(name, 100);
    newScreen.timestamp = timestamp;

    // Add to map and update current screen
    screens[name] = newScreen;
    currentScreen = name;
}

void ScreenManager::screenRestore(const std::string& name) {
    auto it = screens.find(name);
    if (it == screens.end()) {
        printInColor("No screen found with this name.\n", "red");
        return;
    }

    // Set current screen
    currentScreen = name;

    // Switch to screen console
    consoleManager.switchConsole(ConsoleType::Screen);
}

void ScreenManager::screenList() {
    std::cout << "\n---------------------------------------\n";
    std::cout << "Running processess:\n";

    int cnt_running = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (!screen.second.finished && screen.second.coreId != -1) {
                cnt_running++;
                std::cout << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Core: " << std::setw(3) << std::left << screen.second.coreId << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_running == 0) {
        printInColor("No running processes.\n", "gray");
    }

    std::cout << "\nFinished processess:\n";

    int cnt_finished = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (screen.second.finished) {
                cnt_finished++;
                std::cout << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Finished" << std::left << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_finished == 0) {
        printInColor("No finished processes.\n", "gray");
    }

    std::cout << "---------------------------------------\n\n";
}
