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

Scheduler::Scheduler(const Config& config, MemoryManager* memoryManager)
    : config(config), memoryManager(memoryManager), finished(false),
    numCores(config.num_cpu), nextCore(0),
    schedulerType(config.scheduler == "rr" ? SchedulerType::RR : SchedulerType::FCFS),
    quantumCycles(config.quantum_cycles) {
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
    int quantumCycle = 0;

    try {
        while (true) {
            if (finished) {
                std::cout << "Debug: Worker " << coreId << " exiting as finished flag is set.\n";
                return;
            }

            Screen* screen = nullptr;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [this] {
                    return finished || !screenQueue.empty();
                    });

                if (finished && screenQueue.empty()) {
                    std::cout << "Debug: Worker " << coreId << " exiting as finished and queue is empty.\n";
                    return;
                }

                if (!screenQueue.empty()) {
                    screen = screenQueue.front();
                    screenQueue.pop();

                    // Debug log for dequeued screen
                    std::cout << "Debug: Dequeued screen: " << screen->name
                        << " | State: " << screen->getStateString() << "\n";

                    // Transition a Waiting process back to Ready
                    if (screen->getState() == ScreenState::Waiting) {
                        if (memoryManager->isInMemory(*screen)) {
                            memoryManager->removeProcess(*screen);
                        }

                        screen->setState(ScreenState::Ready);
                        screenQueue.push(screen);
                        cv.notify_one();
                        continue;  // Skip to next iteration
                    }
                }
            }

            if (screen) {
                // Debug log for screen processing
                std::cout << "Debug: Processing screen: " << screen->name
                    << " | State: " << screen->getStateString() << "\n";

                if (screen->getState() == ScreenState::Ready) {
                    screen->setState(ScreenState::Running);
                    screen->coreId = coreId;

                    if (!memoryManager->isInMemory(*screen)) {
                        if (!memoryManager->addProcess(*screen)) {
                            std::cout << "Debug: Not enough memory for screen: " << screen->name << "\n";

                            // Queue the process back if not already in the queue
                            bool alreadyInQueue = false;
                            std::queue<Screen*> tempQueue;

                            while (!screenQueue.empty()) {
                                Screen* tempScreen = screenQueue.front();
                                screenQueue.pop();
                                tempQueue.push(tempScreen);

                                if (tempScreen == screen) {
                                    alreadyInQueue = true;
                                }
                            }

                            // Restore the queue
                            while (!tempQueue.empty()) {
                                screenQueue.push(tempQueue.front());
                                tempQueue.pop();
                            }

                            if (!alreadyInQueue) {
                                screenQueue.push(screen);
                                cv.notify_one();
                            }

                            continue; // Skip this process and advance to the next iteration
                        }
                    }
                }

                if (screen->getState() == ScreenState::Finished) {
                    std::cout << "Debug: Screen finished: " << screen->name << "\n";
                    memoryManager->removeProcess(*screen);
                    continue;
                }

                if (schedulerType == SchedulerType::RR) {
                    executeProcessRR(screen, coreId);
                }
                else if (schedulerType == SchedulerType::FCFS) {
                    executeProcessFCFS(screen, coreId);
                }
            }

            // Ensure memory snapshot is printed only when global quantum cycle matches the expected slice
            if (coreId == 0 && quantumCycle % quantumCycles == 0) {
                std::cout << "Debug: Printing memory snapshot for quantum cycle: " << quantumCycle << "\n";
                memoryManager->printMemorySnapshot(quantumCycle);
            }

            quantumCycle += quantumCycles;  // Increment by the slice
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in worker " << coreId << ": " << e.what() << "\n";
    }
    catch (...) {
        std::cerr << "Unknown error occurred in worker " << coreId << "\n";
    }
}




// FCFS: Complete execution of each screen process before moving to another process
void Scheduler::executeProcessFCFS(Screen* screen, int coreId) {
    screen->setState(ScreenState::Running);
    screen->coreId = coreId;

    std::ofstream logFile(screen->name + ".txt");

    for (int i = 0; i < screen->totalLines; ++i) {
        if (screen->currentLine >= screen->totalLines) {
            screen->setState(ScreenState::Finished);  // Transition to Finished
            logFile.close();
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work

        // Log progress
        time_t now = time(0);
        tm ltm;

#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
        logFile.flush();
        screen->currentLine++;
    }

    screen->setState(ScreenState::Finished);  // Transition to Finished
    logFile.close();
}


// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    screen->setState(ScreenState::Running);
    screen->coreId = coreId;

    std::ofstream logFile(screen->name + ".txt", std::ios::app);

    int executedLines = 0;
    while (executedLines < screen->totalLines) {
        int linesToProcess = min(quantumCycles, screen->totalLines - executedLines);

        for (int i = 0; i < linesToProcess; ++i) {
            if (screen->currentLine >= screen->totalLines) {
                screen->setState(ScreenState::Finished);
                logFile.close();
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
            screen->currentLine++;
        }

        executedLines += linesToProcess;

        if (executedLines < screen->totalLines) {
                screen->setState(ScreenState::Waiting);
                std::unique_lock<std::mutex> lock(queueMutex);
                screenQueue.push(screen);
                cv.notify_one();
            break;
        }
    }

    if (executedLines >= screen->totalLines) {
        screen->setState(ScreenState::Finished);
        logFile.close();
    }
}





void Scheduler::addProcess(Screen& screen) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        screen.setState(ScreenState::Running);
        screen.coreId = nextCore;
        screenQueue.push(&screen);
        nextCore = (nextCore + 1) % numCores;
    }
    cv.notify_one();
}

void Scheduler::newProcess(Screen& screen) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        screen.setState(ScreenState::Ready); // Set state to Ready
        screenQueue.push(&screen);          // Add to the queue
    }
    cv.notify_one();
}

void Scheduler::finish() {
    finished = true;
    cv.notify_all();

    // Clear memory for all finished processes
    while (!screenQueue.empty()) {
        Screen* screen = screenQueue.front();
        screenQueue.pop();
    }
}