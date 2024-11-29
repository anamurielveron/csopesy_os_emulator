#include "ScreenManager.h"
#include "ConsoleManager.h"
#include "Screen.h"
#include "Scheduler.h"
#include "Utils.h"
#include "Config.h"
#include "PagingAllocator.h"


#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>

using std::max;
using std::min;

ScreenManager::ScreenManager(ConsoleManager& cm) : consoleManager(cm), currentScreen(""), scheduler(nullptr), schedulerRunning(false), testRunning(false) {}




void ScreenManager::screenCreate(const String& name, const String &type) {
    if (screens.find(name) != screens.end()) {
        printInColor("Screen already exists with this name.\n\n", "red");
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
    newScreen.setTimestamp(timestamp);

    // Add to map and update current screen
    screens[name] = newScreen;
    if (type == "screenCreate") {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(config.min_ins, config.max_ins);

        int instructionCount = dist(gen);
        screens[name].setTotalLines(instructionCount);

        // Add the new process to the scheduler
        scheduler->addReadyQueue(screens[name]);
        
        //currentScreen = name;
        printInColor("Process \"" + name + "\" created successfully.\n\n", "green");
    }
}

void ScreenManager::screenRestore(const String& name) {
    auto it = screens.find(name);
    if (it == screens.end()) {
        printInColor("No screen found with this name.\n\n", "red");
        return;
    }

    // Set current screen
    currentScreen = name;

    // Switch to screen console
    consoleManager.switchConsole(ConsoleType::Screen);
}

void ScreenManager::screenList(const String& type) {
    std::ostringstream output;  // Create a stream to capture output

    std::unordered_set<int> activeCoreIds;
    std::unordered_map<int, int> coreProcessCount;

    // Active cores counting for CPU utilization
    for (const auto& screen : screens) {
        if (screen.second.getState() == Screen::State::Running && screen.second.getCoreId() != -1) {
            coreProcessCount[screen.second.getCoreId()]++;
            activeCoreIds.insert(screen.second.getCoreId());
        }
    }

    int activeCores = activeCoreIds.size();
    int coresAvailable = max(0, config.num_cpu - activeCores);
    double cpuUtilization = (static_cast<double>(activeCores) / config.num_cpu) * 100;

    // Capture CPU info to both console and file stream
    output << "\n---------------------------------------\n";
    output << "CPU Utilization: " << cpuUtilization << "%" << "\n";
    output << "Cores Used: " << activeCores << "\n";
    output << "Cores Available: " << coresAvailable << "\n";

    output << "\n---------------------------------------\n";
    output << "Running processes:\n";

    int cnt_running = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (!(screen.second.getState() == Screen::State::Finished)) {
                cnt_running++;

                String stateString = screen.second.getStateString();

                output << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].getTimestamp() << ")    "
                    << "State: " << std::setw(8) << std::left << stateString << "   "
                    << "Core: " << std::setw(3) << std::left << screen.second.getCoreId() << "   "
                    << screen.second.getCurrentLine() << " / " << screen.second.getTotalLines() << "\n";
            }
        }
        if (cnt_running == 0) {
            output << "No running processes.\n";
        }

        output << "\nFinished processes:\n";
        int cnt_finished = 0;
        if (!screens.empty()) {
            for (const auto& screen : screens) {
                // Check if the process is in Finished state
                if (screen.second.getState() == Screen::State::Finished) {
                    cnt_finished++;
                    output << std::setw(10) << std::left << screen.first << "   "
                        << "(" << screens[screen.first].getTimestamp() << ")    "
                        << "Finished" << std::left << "   "
                        << screen.second.getCurrentLine() << " / " << screen.second.getTotalLines() << "\n";
                }
            }
        }
        if (cnt_finished == 0) {
            output << "No finished processes.\n";
        }

        output << "---------------------------------------\n\n";

        if (type == "screenList") {
            std::cout << output.str();
        }
        else if (type == "reportUtil") {
            std::ofstream logFile("csopesy_log.txt");
            if (logFile.is_open()) {
                logFile << output.str();
                logFile.close();
                printInColor("Report generated at csopesy_log.txt\n\n", "green");
            }
            else {
                printInColor("Error: Could not open csopesy_log.txt for writing.\n\n", "red");
            }
        }
    }
}


void ScreenManager::schedulerTest() {
    // Ensure only one instance of the scheduler runs at a time
    if (testRunning) {
        printInColor("Scheduler-test is already running.\n\n", "red");
        return;
    }

    printInColor("Scheduler-test has started.\n\n", "yellow");
    testRunning = true;

    // Clear all existing log files before starting (Just in case there are files with the exact name process)
    for (const auto& screenEntry : screens) {
        std::ofstream logFile(screenEntry.first + ".txt", std::ios::trunc);
        if (logFile.is_open()) {
            logFile.close();
        }
        else {
            std::cerr << "Error: Could not clear file " << screenEntry.first << ".txt\n";
        }
    }

    // Delete all previous processes
    screens.clear();

    std::thread processGeneratorThread([this]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(config.min_ins, config.max_ins);

        int cycleCounter = 0;

        // Background scheduler loop
        while (testRunning) {
            // Add a new process at intervals defined by batch_process_freq
            if (cycleCounter % (config.batch_process_freq) == 0) {
                String screenName = "process" + std::to_string(cycleCounter / config.batch_process_freq);
                int instructionCount = dist(gen);

                // Create a new screen (process) and set its instruction count
                screenCreate(screenName, "schedulerTest");
                screens[screenName].setTotalLines(instructionCount);

                // Add the new process to the scheduler
                scheduler->addReadyQueue(screens[screenName]);

            }

            // Apply delay
            if (config.delays_per_exec >= 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.delays_per_exec + 1));
            }

            

            cycleCounter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
     });

    // Detach the thread to let it run in the background
    processGeneratorThread.detach();
}

void ScreenManager::schedulerStop() {
    if (testRunning) {
        // Stop the background scheduler loop
        testRunning = false;
        printInColor("Scheduler-test stopped.\n\n", "yellow");
    }
    else {
        printInColor("Scheduler-test is not running.\n\n", "red");
    }
}

void ScreenManager::loadConfig(const String& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }

    String parameter;

    while (file >> parameter) {
        if (parameter == "num-cpu") {
            int value;
            file >> value;
            config.num_cpu = clamp(value, 1, 128);
        }
        else if (parameter == "scheduler") {
            String schedulerValue;
            file >> std::ws;

            char firstChar = file.peek();
            if (firstChar == '"') {
                file.get();
                std::getline(file, schedulerValue, '"'); 
            }
            else {
                file >> schedulerValue;
            }

            if (schedulerValue == "fcfs" || schedulerValue == "rr") {
                config.scheduler = schedulerValue;
            }
            else {
                throw std::runtime_error("Invalid scheduler value.");
            }
        }
        else if (parameter == "quantum-cycles") {
            int value;
            file >> value;
            config.quantum_cycles = clamp(value, 1, 4294967296); // [1, 2^32]
        }
        else if (parameter == "batch-process-freq") {
            int value;
            file >> value;
            config.batch_process_freq = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "min-ins") {
            int value;
            file >> value;
            config.min_ins = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "max-ins") {
            int value;
            file >> value;
            config.max_ins = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "delay-per-exec") {
            int value;
            file >> value;
            config.delays_per_exec = clamp(value, 0, 4294967296); // [0, 2^32
        }
        else if (parameter == "max-overall-mem") {
            int value;
            file >> value;
            config.max_overall_mem = clamp(value, 1, 4294967296);
        }
        else if (parameter == "mem-per-frame") {
            int value;
            file >> value;
            config.mem_per_frame = clamp(value, 1, 4294967296);
        }
        else if (parameter == "min-mem-per-proc") {
            int value;
            file >> value;
            config.min_mem_per_proc = clamp(value, 1, 4294967296);
        }
        else if (parameter == "max-mem-per-proc") {
            int value;
            file >> value;
            config.max_mem_per_proc = clamp(value, 1, 4294967296);
        }
        else {
            std::cerr << "Unknown parameter in config file: " << parameter << std::endl;
        }
    }

    file.close();
}

void ScreenManager::initialize() {

    if (scheduler) {
        // Delete the previous scheduler and all previous processes
        schedulerRunning = false;
        scheduler->finish();
        delete scheduler;
        scheduler = nullptr;
        screens.clear();
    }

    try {
        loadConfig("config.txt");
    }
    catch (const std::exception& e) {
        printInColor("Error: " + String(e.what()) + "\n", "red");
        return;
    }

    std::cout << "\n";
    std::cout << "Configuration Loaded:\n";
    std::cout << "Number of CPUs: " << config.num_cpu << "\n";
    std::cout << "Scheduler: " << config.scheduler << "\n";
    std::cout << "Quantum Cycles: " << config.quantum_cycles << "\n";
    std::cout << "Batch Process Frequency: " << config.batch_process_freq << "\n";
    std::cout << "Minimum Instructions: " << config.min_ins << "\n";
    std::cout << "Maximum Instructions: " << config.max_ins << "\n";
    std::cout << "Delays per Exec: " << config.delays_per_exec << "\n";
    std::cout << "Maximum Overall Memory: " << config.max_overall_mem << "\n";
    std::cout << "Memory per Frame: " << config.mem_per_frame << "\n";
    std::cout << "Maximum memory per process: " << config.max_mem_per_proc << "\n";
    std::cout << "Minimum memory per process: " << config.min_mem_per_proc << "\n";

    if (config.max_overall_mem == config.mem_per_frame) {
        memoryManager = std::make_unique<MemoryManager>(
            config.max_overall_mem,
            config.min_mem_per_proc,
            config.max_mem_per_proc,
            config.mem_per_frame,
            config.num_cpu
        );
        pagingAllocator.reset();
    }
    else {
        pagingAllocator = std::make_unique<PagingAllocator>(
            config.max_overall_mem,
            config.mem_per_frame,
            config.min_mem_per_proc,
            config.max_mem_per_proc
        );
        memoryManager.reset();
    }
    scheduler = new Scheduler(config);

    // Start the scheduler thread
    schedulerRunning = true;
    schedulerThread = std::thread([this]() {
        while (schedulerRunning) {
            // Scheduler background tasks go here, if any
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Polling delay
        }
        });

    schedulerThread.detach();
    printInColor("Initialization complete.\n\n", "green");
}

void ScreenManager::processSMI() {

    String Type = "";

    if (scheduler->getTypeString() == "FlatMemory")
        scheduler->processSMI
}


//void ScreenManager::vmstat() {
//    std::cout << "----- VMSTAT -----\n";
//    std::cout << config.max_overall_mem << " K total memory\n";
//    std::cout << usedMemory << " K used memory\n";
//    std::cout << (pagingallocator->getFreeframes()) * config.mem_per_frame << " K free memory\n";
//    std::cout << totalSwap << " K total swap\n";
//    std::cout << freeSwap << " K free swap\n";
//    std::cout << idleCpuTicks << " idle cpu ticks\n";
//    std::cout << systemCpuTicks << " system cpu ticks\n";
//    std::cout << interrupts << " interrupts\n";
//    std::cout << bootTime << " boot time\n";
//    std::cout << "-------------------------------------------------------\n";
//}
