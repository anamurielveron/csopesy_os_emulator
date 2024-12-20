#include "Scheduler.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"

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
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt");

    for (int i = 0; i < screen->totalLines; ++i) {
        if (screen->currentLine >= screen->totalLines) { // Extra safety check
            screen->finished = true;
            logFile.close();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
        // Get timestamp
        time_t now = time(0);
        tm ltm;

#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        // Write log entry
        logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
        logFile.flush();
        screen->currentLine++;
    }

    logFile.close();
    screen->finished = true;
}

// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt", std::ios::app);

    int executedLines = 0;
    while (executedLines < screen->totalLines) {
        int linesToProcess = min(quantumCycles, screen->totalLines - executedLines);

        for (int i = 0; i < linesToProcess; ++i) {
            if (screen->currentLine >= screen->totalLines) { // Extra safety check
                screen->finished = true;
                logFile.close();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
            // Get timestamp
            time_t now = time(0);
            tm ltm;

#ifdef _WIN32
            localtime_s(&ltm, &now);
#else
            localtime_r(&now, &ltm);
#endif
            char timestamp[25];
            strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

            // Write log entry
            logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
            logFile.flush();
            screen->currentLine++;
        }

        executedLines += linesToProcess;

        if (executedLines < screen->totalLines) {
            std::unique_lock<std::mutex> lock(queueMutex);
            screenQueue.push(screen);  // Requeue the process for the next quantum
            cv.notify_one();
            break;  // Yield control to other processes
        }
    }

    if (executedLines >= screen->totalLines) {
        logFile.close();
        screen->finished = true;
    }
}

void Scheduler::addProcess(Screen& screen) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        screen.coreId = nextCore;
        screenQueue.push(&screen);
        nextCore = (nextCore + 1) % numCores;
    }
    cv.notify_one();
}

void Scheduler::finish() {
    finished = true;
    cv.notify_all(); // Notify all threads to finish execution
}
