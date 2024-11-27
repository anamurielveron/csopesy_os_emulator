#include "Scheduler.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <algorithm>

using std::max;
using std::min;

Scheduler::Scheduler(const Config& config)
    : config(config), finished(false), numCores(config.num_cpu), nextCore(0),
    schedulerType(config.scheduler == "rr" ? SchedulerType::RR : SchedulerType::FCFS),
    quantumCycles(config.quantum_cycles) {

    // Set up threads based on the number of CPUs from the config
    for (int i = 0; i < config.num_cpu; ++i) {
        cores.emplace_back(&Scheduler::worker, this, i);
    }
}

Scheduler::~Scheduler() {
    finished = true;
    cv.notify_all();
    for (auto& core : cores) {
        core.join();
    }
}

void Scheduler::worker(int coreId) {
    while (true) {
        if (finished) return;

        Screen* screen = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return finished || !screenQueue.empty();
                });

            if (finished && screenQueue.empty()) {
                return;
            }

            if (!screenQueue.empty()) {
                screen = screenQueue.front();
                screenQueue.pop();
                screen->coreId = coreId;
                ++activeCores; // Increment active cores count
            }
            else {
                continue;
            }
        }

        if (screen) {
            if (schedulerType == SchedulerType::FCFS) {
                executeProcessFCFS(screen, coreId);
            }
            else if (schedulerType == SchedulerType::RR) {
                executeProcessRR(screen, coreId);
            }
            --activeCores;
        }
    }
}

// FCFS: Complete execution of each screen process before moving to another process
void Scheduler::executeProcessFCFS(Screen* screen, int coreId) {
    screen->setRunningState();
    screen->coreId = coreId; // Assign the core
    std::ofstream logFile(screen->name + ".txt", std::ios::app);

    // Process all lines
    for (int i = 0; i < screen->totalLines; ++i) {
        if (screen->currentLine >= screen->totalLines) { // Safety check
            screen->setFinishedState(); // Transition to Finished state
            logFile.close();
            return;
        }

        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get the current timestamp
        time_t now = time(0);
        tm ltm;
#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        // Log the process execution
        logFile << timestamp << " Core:" << coreId
            << " \"Hello world from " << screen->name << "!\"\n";
        logFile.flush();

        screen->currentLine++;
    }

    // Once all lines are executed, transition to Finished state
    screen->setFinishedState();
    logFile.close();
}

// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    screen->setRunningState();
    if (screen == nullptr) {
        throw std::runtime_error("executeProcessRR: Screen pointer is null!");
    }

    screen->coreId = coreId;

    std::ofstream logFile(screen->name + ".txt", std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + screen->name + ".txt");
    }

    try {
        int executedLines = 0;

        while (executedLines < screen->totalLines) {
            int linesToProcess = std::min(quantumCycles, screen->totalLines - executedLines);

            for (int i = 0; i < linesToProcess; ++i) {
                if (screen->currentLine >= screen->totalLines) {
                    screen->setFinishedState();
                    logFile.close();
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                logFile << "Hello world from " << screen->name << "! Core: " << coreId
                    << " Line: " << (screen->currentLine + 1) << "\n";
                logFile.flush();
                screen->currentLine++;
            }

            executedLines += linesToProcess;

            if (executedLines < screen->totalLines) {
                screen->setWaitingState();
                addReadyQueue(*screen);
                break;
            }
        }

        if (executedLines >= screen->totalLines) {
            screen->setFinishedState();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in executeProcessRR: " << e.what() << std::endl;
        throw;
    }

    logFile.close();
}

    

void Scheduler::addReadyQueue(Screen& screen) {
    // Validate the screen object
    if (&screen == nullptr) {
        throw std::runtime_error("addReadyQueue: Screen object is null!");
    }

    // Mutex-protected section
    try {
        std::lock_guard<std::mutex> lock(queueMutex);

        // Transition states
        if (screen.getState() == Screen::State::New || screen.getState() == Screen::State::Waiting) {
            screen.setReadyState();
        }

        // Add the process to the queue
        screenQueue.push(&screen);

        // Update core allocation
        nextCore = (nextCore + 1) % numCores;
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in addReadyQueue: " << e.what() << std::endl;
        throw;
    }

    // Notify a waiting worker
    cv.notify_one();
}


void Scheduler::finish() {
    finished = true;
    cv.notify_all(); // Notify all threads to finish execution
}
