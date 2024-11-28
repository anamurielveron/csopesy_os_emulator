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

    std::vector<MemoryBlock> freeBlocks; // Free memory blocks
    std::vector<std::tuple<std::string, int, int>> processes; // Process name and start address
    std::map<std::string, int> processMemorySizes;
    std::queue<std::pair<std::string, int>> backingStore;

public:
    MemoryManager(int maxMem, int minMemProc, int maxMemProc, int memFrame, int numCpus)
        : maxMemory(maxMem), minMemPerProc(minMemProc), maxMemPerProc(maxMemProc), memPerFrame(memFrame), numCpu(numCpus) {
        freeBlocks.push_back({ 0, maxMem }); // Initially, all memory is free
    }

    //void moveToBackingStore() {
    //    if (processes.empty()) return;

    //    // Remove the oldest process (first in the list)
    //    auto oldestProcess = processes.front();
    //    processes.erase(processes.begin());

    //    // Add it to the backing store
    //    backingStore.push(oldestProcess);

    //    // Deallocate its memory
    //    freeBlocks.push_back({ oldestProcess.second, memPerProc });
    //    mergeFreeBlocks();

    //    std::cout << "[INFO] Moved process " << oldestProcess.first << " to backing store.\n";
    //}

    //void reloadFromBackingStore() {
    //    if (backingStore.empty()) return;

    //    while (freeBlocks.empty() || freeBlocks[0].size < memPerProc) {
    //        moveToBackingStore();
    //    }

    //    auto process = backingStore.front();
    //    if (allocateMemory(process.first)) {
    //        backingStore.pop();
    //        std::cout << "[INFO] Reloaded process " << process.first << " from backing store.\n";
    //    }
    //}

    int randomMemPerProc() const {
        return minMemPerProc + (std::rand() % (maxMemPerProc - minMemPerProc + 1));
    }

    // Allocate memory using first-fit strategy
    bool allocateMemory(const std::string& processName) {
        if (processName.empty()) {
            std::cout << "[DEBUG] Skipping unnamed process allocation.\n";
            return false;
        }

        int allocSize;

        // Check if memory size is already assigned to this process
        auto it = processMemorySizes.find(processName);
        if (it != processMemorySizes.end()) {
            allocSize = it->second; // Use existing size
            std::cout << "[DEBUG] Using existing memory size " << allocSize << " KB for process " << processName << ".\n";
        }
        else {
            allocSize = randomMemPerProc(); // Randomize new size
            processMemorySizes[processName] = allocSize; // Store the size
            std::cout << "[DEBUG] Randomized memory size " << allocSize << " KB for process " << processName << ".\n";
        }

        // Try to allocate memory
        for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
            if (it->size >= allocSize) { // Check if the block is large enough
                processes.push_back({ processName, it->start, allocSize }); // Allocate process
                it->start += allocSize;  // Update the start of the free block
                it->size -= allocSize;   // Reduce the size of the free block

                if (it->size == 0) {
                    freeBlocks.erase(it); // Remove the block if fully consumed
                }

                std::cout << "[DEBUG] Allocated memory for process " << processName
                    << " (Start: " << std::get<1>(processes.back())
                    << ", Size: " << allocSize << ").\n";
                return true;
            }
        }

        std::cout << "[DEBUG] Memory allocation failed for process " << processName << ".\n";
        return false;
    }


    // Deallocate memory for a finished process
    void deallocateMemory(const std::string& processName) {
        // Find the process in the list of allocated processes
        auto it = std::find_if(processes.begin(), processes.end(),
            [&processName](const auto& process) { return std::get<0>(process) == processName; });

        if (it != processes.end()) {
            int startAddr = std::get<1>(*it); // Get the start address of the process
            int allocSize = std::get<2>(*it); // Get the size of the allocated block
            processes.erase(it);             // Remove the process from the allocated list

            // Add the deallocated memory as a new free block
            freeBlocks.push_back({ startAddr, allocSize });

            // Merge adjacent free blocks
            mergeFreeBlocks();

            std::cout << "[DEBUG] Deallocated memory for process " << processName
                << " (Start: " << startAddr << ", Size: " << allocSize << ").\n";
        }
        else {
            std::cout << "[DEBUG] Process " << processName << " not found in memory.\n";
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
        std::cout << "[DEBUG] Merged adjacent free blocks.\n";
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

        std::cout << "[DEBUG] Memory snapshot generated for quantum cycle " << quantumCycle << ".\n";
    }
};


#endif // MEMORYMANAGER_H
