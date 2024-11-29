#include "Scheduler.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"
#include "MemoryManager.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <algorithm>

using std::max;
using std::min;

Scheduler::Scheduler(const Config& config)
    : config(config),
    memoryManager(config.max_overall_mem, config.min_mem_per_proc, config.max_mem_per_proc, config.mem_per_frame, config.num_cpu),
    pagingAllocator(config.max_overall_mem, config.mem_per_frame, config.min_mem_per_proc, config.max_mem_per_proc, config.num_cpu),
    schedulerType(config.scheduler == "rr" ? SchedulerType::RR : SchedulerType::FCFS),
    quantumCycles(config.quantum_cycles),
    numCores(config.num_cpu) {

    allocatorType = (config.max_overall_mem == config.mem_per_frame)
        ? AllocatorType::FlatMemory
        : AllocatorType::Paging;

    for (int i = 0; i < numCores; ++i) {
        cores.emplace_back(&Scheduler::worker, this, i);
    }
}

Scheduler::AllocatorType Scheduler::getAllocatorType() const {
    return allocatorType;
}

Scheduler::~Scheduler() {
    finished = true;
    cv.notify_all();
    for (auto& core : cores) {
        core.join();
    }
}

void Scheduler::worker(int coreId) {
    int quantumCycle = 0; // Tracks quantum cycles executed by this worker thread.

    while (true) {
        if (finished) {
            if (screenQueue.empty()) {
                // Increment idle ticks if the queue is empty
                updateCpuTicks(true);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
        }

        Screen* screen = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { 
                return finished || !screenQueue.empty();
                });

            if (finished && screenQueue.empty()) {
                return; // Exit if finished and no processes are in the queue.
            }

            if (!screenQueue.empty()) {
                screen = screenQueue.front();
                screenQueue.pop();
            }
            else {
                // Increment idle ticks if the queue is empty
                updateCpuTicks(true);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
        }

        if (screen) {
            // Increment active ticks as a process is being handled
            updateCpuTicks(false);

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Delay to simulate processing

            if (screen->getState() == Screen::State::Finished) {
                continue; // Skip to the next process.
            }

            // Handle Ready state
            if (screen->getState() == Screen::State::Ready) {
                bool allocated = false;

                // Use the appropriate memory allocator
                if (allocatorType == AllocatorType::FlatMemory) {
                    allocated = memoryManager.allocateMemory(screen->getName());
                }
                else if (allocatorType == AllocatorType::Paging) {
                    allocated = pagingAllocator.allocatePages(screen->getName());
                }

                if (allocated) {
                    screen->setRunningState();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Delay for memory allocation

                    // Execute one quantum slice
                    if (schedulerType == SchedulerType::FCFS) {
                        executeProcessFCFS(screen, coreId);
                    }
                    else if (schedulerType == SchedulerType::RR) {
                        executeProcessRR(screen, coreId);
                    }

                    // Generate snapshot only if this is the designated core
                    if (coreId == 0) {
                        if (allocatorType == AllocatorType::FlatMemory) {
                            memoryManager.generateSnapshot(quantumCycle);
                        }
                        else if (allocatorType == AllocatorType::Paging) {
                            pagingAllocator.generateSnapshot(quantumCycle);
                        }

                        quantumCycle += quantumCycles;
                        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Delay for snapshot generation
                    }

                    // Check if the process is now finished
                    if (screen->getState() == Screen::State::Finished) {

                        if (allocatorType == AllocatorType::FlatMemory) {
                            memoryManager.deallocateMemory(screen->getName());
                        }
                        else if (allocatorType == AllocatorType::Paging) {
                            pagingAllocator.deallocatePages(screen->getName());
                        }

                        continue; // Do not requeue the process
                    }

                    // Transition to Waiting state after processing
                    screen->setWaitingState();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Delay for state transition
                }
                else {
                    updateCpuTicks(true); // Core is idle
                    addReadyQueue(*screen);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Delay to avoid busy loop
                }
            }
            else {
                updateCpuTicks(true);
            }

            // Handle Waiting state
            if (screen->getState() == Screen::State::Waiting) {
                if (allocatorType == AllocatorType::FlatMemory) {
                    memoryManager.deallocateMemory(screen->getName());
                }
                else if (allocatorType == AllocatorType::Paging) {
                    pagingAllocator.deallocatePages(screen->getName());
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Delay for deallocation
                addReadyQueue(*screen);

                continue;
            }
        }
        else {
            std::cerr << "[ERROR] Null screen pointer encountered.\n";
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Delay for null pointer handling
            continue; // Safely skip to the next iteration.
        }
    }
}




// FCFS: Complete execution of each screen process before moving to another process
void Scheduler::executeProcessFCFS(Screen* screen, int coreId) {
    if (screen == nullptr) {
        throw std::runtime_error("executeProcessFCFS: Screen pointer is null!");
    }

    // Set process state and assign core
    screen->setRunningState();
    screen->setCoreId(coreId);

    // Open the log file for appending
    std::ofstream logFile(screen->getName() + ".txt", std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + screen->getName() + ".txt");
    }

    try {
        // Process all lines
        for (int i = screen->getCurrentLine(); i < screen->getTotalLines(); ++i) {
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
                << " \"Hello world from " << screen->getName() << "!\"\n";
            logFile.flush();

            // Increment the current line
            screen->setCurrentLine(screen->getCurrentLine() + 1);
        }

        // Transition to Finished state and deallocate memory
        screen->setFinishedState();
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in executeProcessFCFS: " << e.what() << std::endl;
        throw; // Re-throw exception for further handling
    }

    // Close the log file
    logFile.close();
}


// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    if (screen == nullptr) {
        throw std::runtime_error("executeProcessRR: Screen pointer is null!");
    }

    screen->setCoreId(coreId);

    // Open log file for appending
    std::ofstream logFile(screen->getName() + ".txt", std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Failed to open log file: " + screen->getName() + ".txt");
    }

    try {
        while (screen->getCurrentLine() < screen->getTotalLines()) {
            int linesToProcess = std::min(quantumCycles, screen->getTotalLines() - screen->getCurrentLine());

            for (int i = 0; i < linesToProcess; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                logFile << "Core " << coreId << " processed line " << screen->getCurrentLine() + 1
                    << " of process " << screen->getName() << "\n";
                screen->setCurrentLine(screen->getCurrentLine() + 1);
            }

            if (screen->getCurrentLine() >= screen->getTotalLines()) {
                screen->setFinishedState();
                logFile.close();
                return;
            }

            break; // Yield to other processes
        }
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in executeProcessRR: " << e.what() << std::endl;
    }

    logFile.close();
}
    

void Scheduler::addReadyQueue(Screen& screen) {

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

void Scheduler::logQueueState() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::cout << "[DEBUG] Ready Queue State: ";
    std::queue<Screen*> tempQueue = screenQueue; // Copy the queue for inspection.
    while (!tempQueue.empty()) {
        Screen* screen = tempQueue.front();
        tempQueue.pop();
        std::cout << screen->getName() << " ";
    }
    std::cout << "\n";
}


void Scheduler::finish() {
    finished = true;
    cv.notify_all(); // Notify all threads to finish execution
}


void Scheduler::processSMI() {
    if (allocatorType == AllocatorType::FlatMemory) {
        memoryManager.processSMI();
    }
    else if (allocatorType == AllocatorType::Paging) {
        pagingAllocator.processSMI();
    }
}

void Scheduler::updateCpuTicks(bool isIdle) {
    if (allocatorType == AllocatorType::Paging) {
        if (isIdle) {
            pagingAllocator.incrementIdleCpuTicks(1);
        }
        else {
            pagingAllocator.incrementActiveCpuTicks(1);
        }
        pagingAllocator.incrementTotalCpuTicks(1);
    }
    else if (allocatorType == AllocatorType::FlatMemory) {
        if (isIdle) {
            memoryManager.incrementIdleCpuTicks(1);
        }
        else {
            memoryManager.incrementActiveCpuTicks(1);
        }
        memoryManager.incrementTotalCpuTicks(1);
    }

}

void Scheduler::VMstat() {
    if (allocatorType == AllocatorType::Paging) {
        pagingAllocator.VMstat();
    }
    else if (allocatorType == AllocatorType::FlatMemory) {
        memoryManager.VMstat();
    }
}