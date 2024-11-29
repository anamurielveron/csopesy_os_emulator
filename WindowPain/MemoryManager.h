#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <tuple>
#include <map>
#include <unordered_map>

class MemoryManager {
private:
    struct MemoryBlock {
        int start;
        int size;
    };

    const int maxMemory;      // Total memory
    const int minMemPerProc;  // Minimum memory per process
    const int maxMemPerProc;
    const int memPerFrame;    // Frame size
    const int numCpu;         // Number of CPUs

    int totalMemory;
    int usedMemory;
    int freeMemory;

    std::vector<MemoryBlock> freeBlocks; // Free memory blocks
    std::vector<std::tuple<std::string, int, int>> processes; // Process name and start address
    std::map<std::string, int> processMemorySizes;
    std::queue<std::pair<std::string, int>> backingStore;
    std::unordered_map<std::string, int> runningProcesses;

    int idleCpuTicks = 0;       // Tracks idle CPU ticks
    int activeCpuTicks = 0;     // Tracks active CPU ticks
    int totalCpuTicks = 0;      // Tracks total CPU ticks

public:
    MemoryManager(int maxMem, int minMemProc, int maxMemProc, int memFrame, int numCpus)
        : maxMemory(maxMem), minMemPerProc(minMemProc), maxMemPerProc(maxMemProc), memPerFrame(memFrame), numCpu(numCpus), totalMemory(maxMem)
        , usedMemory(0), freeMemory(0) {
        freeBlocks.push_back({ 0, maxMem }); // Initially, all memory is free
    }

    int getTotalMemory() const { return totalMemory; }
    int getUsedMemory() const { return usedMemory; }
    int getFreeMemory() const { return freeMemory; }

    int getIdleCpuTicks() const { return idleCpuTicks; }
    int getActiveCpuTicks() const { return activeCpuTicks; }
    int getTotalCpuTicks() const { return totalCpuTicks; }

    void incrementIdleCpuTicks(int ticks) { idleCpuTicks += ticks; }
    void incrementActiveCpuTicks(int ticks) { activeCpuTicks += ticks; }
    void incrementTotalCpuTicks(int ticks) { totalCpuTicks += ticks; }


    std::unordered_map<std::string, int> getRunningProcesses() const {
        std::unordered_map<std::string, int> runningProcesses;

        for (const auto& process : processes) {
            std::string processName = std::get<0>(process);
            int allocSize = std::get<2>(process);           // Allocated size
            runningProcesses[processName] = allocSize;
        }

        return runningProcesses;
    }


    int randomMemPerProc() const {
        return minMemPerProc + (std::rand() % (maxMemPerProc - minMemPerProc + 1));
    }

    // Allocate memory using first-fit strategy
    bool allocateMemory(const std::string& processName) {
        if (processName.empty()) {
            return false;
        }

        int allocSize;

        // Check if memory size is already assigned to this process
        auto it = processMemorySizes.find(processName);
        if (it != processMemorySizes.end()) {
            allocSize = it->second; // Use existing size
        }
        else {
            allocSize = randomMemPerProc(); // Randomize new size
            processMemorySizes[processName] = allocSize; // Store the size
        }

        // Try to allocate memory
        while (true) {
            for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
                if (it->size >= allocSize) { // Check if the block is large enough
                    processes.push_back({ processName, it->start, allocSize }); // Allocate process
                    it->start += allocSize;  // Update the start of the free block
                    it->size -= allocSize;   // Reduce the size of the free block

                    if (it->size == 0) {
                        freeBlocks.erase(it); // Remove the block if fully consumed
                    }

                    usedMemory += allocSize;
                    freeMemory -= allocSize;

                    runningProcesses[processName] = allocSize;

                    return true;
                }
            }

            // If memory allocation failed, move oldest process to backing store
            if (!processes.empty()) {
                moveToBackingStore();
            }
            else {
                return false;
            }
        }
    }

    void moveToBackingStore() {
        if (processes.empty()) return;

        auto oldestProcess = processes.front();
        processes.erase(processes.begin());

        backingStore.push({ std::get<0>(oldestProcess), std::get<2>(oldestProcess) });

        int deallocatedSize = std::get<2>(oldestProcess);
        freeBlocks.push_back({ std::get<1>(oldestProcess), std::get<2>(oldestProcess) });
        mergeFreeBlocks();

        usedMemory -= deallocatedSize;
        freeMemory += deallocatedSize;

        runningProcesses.erase(std::get<0>(oldestProcess));
    }

    void reloadFromBackingStore() {
        if (backingStore.empty()) {
            return;
        }

        auto process = backingStore.front();
        backingStore.pop();

        if (!allocateMemory(process.first)) {
            backingStore.push(process);
        }
        else {
            runningProcesses[process.first] = process.second;
        }
    }



    // Deallocate memory for a finished process
    void deallocateMemory(const std::string& processName) {
        auto it = std::find_if(processes.begin(), processes.end(),
            [&processName](const auto& process) { return std::get<0>(process) == processName; });

        if (it != processes.end()) {
            int startAddr = std::get<1>(*it);
            int allocSize = std::get<2>(*it);
            processes.erase(it);

            auto runningIt = runningProcesses.find(processName);
            if (runningIt != runningProcesses.end()) {
                runningProcesses.erase(runningIt);
            }

            freeBlocks.push_back({ startAddr, allocSize });
            mergeFreeBlocks();

            usedMemory -= allocSize;
            freeMemory += allocSize;

            // Reload a process from the backing store if possible
            reloadFromBackingStore();
        }
    }


    // Merge adjacent free blocks to reduce fragmentation
    void mergeFreeBlocks() {
        std::sort(freeBlocks.begin(), freeBlocks.end(),
            [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });

        for (size_t i = 0; i < freeBlocks.size() - 1; ++i) {
            if (freeBlocks[i].start + freeBlocks[i].size == freeBlocks[i + 1].start) {
                freeBlocks[i].size += freeBlocks[i + 1].size; // Merge blocks
                freeBlocks.erase(freeBlocks.begin() + i + 1); // Remove merged block
                --i; // Recheck the current index after merging
            }
        }
    }


    // Calculate external fragmentation
    int calculateFragmentation() const {
        int fragmentation = 0;

        // Add sizes of all free memory blocks
        for (const auto& block : freeBlocks) {
            fragmentation += block.size;
        }

        return fragmentation;
    }


    // Generate memory snapshot
    void generateSnapshot(int quantumCycle) const {
        std::ofstream file("memory_stamp_" + std::to_string(quantumCycle) + ".txt");
        if (!file.is_open()) {
            std::cerr << "[ERROR] Failed to open snapshot file.\n";
            return;
        }

        // Timestamp
        time_t now = time(0);
        tm ltm;
#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        // Snapshot content
        file << "Timestamp: " << timestamp << "\n";
        file << "Number of processes in memory: " << processes.size() << "\n";
        file << "Total external fragmentation in KB: " << calculateFragmentation() << "\n";
        file << "\n----end---- = " << maxMemory << "\n";

        for (const auto& process : processes) {
            int startAddr = std::get<1>(process);
            int endAddr = startAddr + std::get<2>(process) - 1;
            file << endAddr << "\n" << std::get<0>(process) << "\n" << startAddr << "\n";
        }

        file << "----start---- = 0\n";
        file.close();

    }


    void processSMI() {
        std::cout << "----- PROCESS-SMI VOL. 1.00 Driver Version: 01.00 -----\n";
        int activeProcesses = getRunningProcesses().size();
        double cpuUtil = static_cast<double>(activeProcesses) / numCpu * 100;
        std::cout << "CPU-Util: " << std::fixed << std::setprecision(2) << cpuUtil << "%\n";

        // Get memory usage dynamically
        int totalMemory = getTotalMemory();
        int usedMemory = getUsedMemory();

        std::cout << "Memory Usage: " << usedMemory / 1024 << "MiB / " << totalMemory / 1024 << "MiB\n";
        std::cout << "Memory Util: " << std::fixed << std::setprecision(2)
            << (static_cast<double>(usedMemory) / totalMemory) * 100 << "%\n";

        auto processes = getRunningProcesses();

        std::cout << "\nRunning processes and memory usage:\n";
        for (const auto& process : processes) {
            const std::string& processName = process.first;
            int memoryUsage = process.second;
            std::cout << processName << ": " << memoryUsage / 1024 << "MiB\n";
        }

        std::cout << "-------------------------------------------------------\n";
    }

    void VMstat() const {
        std::cout << "----- VMSTAT REPORT -----\n";
        std::cout << "Total Memory: " << totalMemory / 1024 << " MiB\n";
        std::cout << "Used Memory: " << (totalMemory - usedMemory) / 1024 << " MiB\n";
        std::cout << "Free Memory: " << (freeMemory) / 1024 << " MiB\n";
        std::cout << "Idle CPU Ticks: " << idleCpuTicks << "\n";
        std::cout << "Active CPU Ticks: " << activeCpuTicks << "\n";
        std::cout << "Total CPU Ticks: " << totalCpuTicks << "\n";
        std::cout << "-------------------------\n";
    }
};


#endif // MEMORYMANAGER_H