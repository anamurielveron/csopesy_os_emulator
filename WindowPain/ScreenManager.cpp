#include "ScreenManager.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <algorithm>

using std::max;
using std::min;

ScreenManager::ScreenManager(ConsoleManager& cm) : consoleManager(cm), currentScreen("") {}

void ScreenManager::screenCreate(const String& name) {
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

void ScreenManager::screenRestore(const String& name) {
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
    std::ostringstream output;  // Create a stream to capture output

    std::unordered_set<int> activeCoreIds;
    std::unordered_map<int, int> coreProcessCount;

    // Active cores counting for cpu utilization
    for (const auto& screen : screens) {
        if (!screen.second.finished && screen.second.coreId != -1) {
            coreProcessCount[screen.second.coreId]++;
            activeCoreIds.insert(screen.second.coreId);
        }
    }

    int activeCores = activeCoreIds.size();
    int coresAvailable = max(0, config.num_cpu - activeCores);
    double cpuUtilization = (static_cast<double>(activeCores) / config.num_cpu) * 100;

    // Capture CPU info to both console and file steam use
    output << "\n---------------------------------------\n";
    output << "CPU Utilization: " << cpuUtilization << "%" << "\n";
    output << "Cores Used: " << activeCores << "\n";
    output << "Cores Available: " << coresAvailable << "\n";

    output << "\n---------------------------------------\n";
    output << "Running processes:\n";

    int cnt_running = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (!screen.second.finished && screen.second.coreId != -1) {
                cnt_running++;
                output << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Core: " << std::setw(3) << std::left << screen.second.coreId << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_running == 0) {
        output << "No running processes.\n";
    }

    output << "\nFinished processes:\n";
    int cnt_finished = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (screen.second.finished) {
                cnt_finished++;
                output << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Finished" << std::left << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_finished == 0) {
        output << "No finished processes.\n";
    }

    output << "---------------------------------------\n\n";

    // Print to console
    std::cout << output.str();

    // Save the output for the reportUtil function
    lastScreenListOutput = output.str();
}