#include "Scheduler.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"

#include <fstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>

using std::max;
using std::min;

Scheduler::Scheduler(const Config& config, ScreenManager& sm)
    : screenManager(sm), 
    config(config), 
    finished(false), 
    numCores(config.num_cpu), 
    nextCore(0),
    schedulerType(config.scheduler == "rr" ? SchedulerType::RR : SchedulerType::FCFS),
    configQuantumCycles(config.quantum_cycles),
    maxOverallMem(config.max_overall_mem),
    memPerFrame(config.mem_per_frame),
    memPerProc(config.mem_per_proc){

    //totalFrames = maxOverallMem / memPerFrame;
    //framesPerProc = memPerProc / memPerFrame;
    memoryFrames.resize(maxOverallMem, false);

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
    int delay = 0;
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt");

    for (int i = 0; i < screen->totalLines; ++i) {
        if (screen->currentLine >= screen->totalLines) { // Extra safety check
            screen->finished = true;
            logFile.close();
            return;
        }
        // Execute after cpu cycle increments
        std::unique_lock<std::mutex> lock(screenManager.mtx);
        screenManager.cycleCv.wait(lock);

        if (delay == 0) {

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

            // Reset delay
            delay = config.delays_per_exec;
        }
        else {
            delay--;
            i--;
        }
    }

    logFile.close();
    screen->finished = true;
}

// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    int delay = 0;
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt", std::ios::app);

    int executedLines = 0;
    while (executedLines < screen->totalLines) {
        int linesToProcess = min(configQuantumCycles, screen->totalLines - executedLines);

        for (int i = 0; i < linesToProcess; ++i) {
            if (screen->currentLine >= screen->totalLines) { // Extra safety check
                screen->finished = true;
                logFile.close();
                return;
            }
            // Execute after cpu cycle increments
            std::unique_lock<std::mutex> lock(screenManager.mtx);
            screenManager.cycleCv.wait(lock);

            if (delay == 0) {
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

                // Reset delay
                delay = config.delays_per_exec;
            }
            else {
                delay--;
                i--;
            }
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
    std::lock_guard<std::mutex> lock(queueMutex);
    screen.coreId = nextCore;
    screenQueue.push(&screen);
    nextCore = (nextCore + 1) % numCores;
    cv.notify_one();
}

void Scheduler::finish() {
    finished = true;
    cv.notify_all(); // Notify all threads to finish execution
}

int Scheduler::calculateFragmentation() {
    int freeFrames = 0;

    for (int i = 0; i < maxOverallMem; ++i) {
        if (!memoryFrames[i]) {
            freeFrames++;
        }
    }

    return freeFrames;
}

void Scheduler::allocateMemory(Screen& screen) {
    // Search for contiguous free frames
    int startFrame = -1;
    int freeFramesCount = 0;

    for (int i = 0; i < maxOverallMem; ++i) {
        if (!memoryFrames[i]) {
            freeFramesCount++;
            if (startFrame == -1) startFrame = i;  // Mark the start of free space
            if (freeFramesCount == memPerProc) break;  // Enough frames found
        }
        else {
            startFrame = -1;  // Reset if block is interrupted
            freeFramesCount = 0;
        }
    }

    if (freeFramesCount < memPerProc) {
        // Not enough contiguous memory for this process
        return;
    }

    // Allocate frames for this process
    for (int i = startFrame; i < startFrame + memPerProc; ++i) {
        memoryFrames[i] = true;
    }
    screen.startFrame = startFrame;  // Store the starting frame for this process
}

void Scheduler::deallocateMemory(Screen& screen) {
    for (int i = screen.startFrame; i < screen.startFrame + maxOverallMem - 1; ++i) {
        memoryFrames[i] = false;
    }
}