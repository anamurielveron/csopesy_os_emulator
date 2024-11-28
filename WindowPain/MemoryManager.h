#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <algorithm>

class MemoryManager {
private:
    struct MemoryBlock {
        int start;
        int size;
    };

    const int maxMemory;   // Total memory
    const int memPerProc;  // Memory per process
    const int memPerFrame; // Frame size
    const int numCpu;      // Number of CPUs

    std::vector<MemoryBlock> freeBlocks; // Free memory blocks
    std::vector<std::pair<std::string, int>> processes; // Process name and start address

public:
    MemoryManager(int maxMem, int memProc, int memFrame, int numCpus)
        : maxMemory(maxMem), memPerProc(memProc), memPerFrame(memFrame), numCpu(numCpus) {
        freeBlocks.push_back({ 0, maxMem }); // Initially, all memory is free
    }

    // Allocate memory using first-fit strategy
    bool allocateMemory(const std::string& processName) {
        for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
            if (it->size >= memPerProc) { // Check if the block is large enough
                processes.push_back({ processName, it->start }); // Allocate process
                it->start += memPerProc;  // Update the start of the free block
                it->size -= memPerProc;   // Reduce the size of the free block

                if (it->size == 0) {
                    freeBlocks.erase(it); // Remove the block if fully consumed
                }
                return true; // Allocation successful
            }
        }
        return false; // No suitable block found
    }

    // Deallocate memory for a finished process
    void deallocateMemory(const std::string& processName) {
        auto it = std::find_if(processes.begin(), processes.end(),
            [&processName](const auto& process) { return process.first == processName; });
        if (it != processes.end()) {
            int startAddr = it->second;
            processes.erase(it);

            // Add a new free block
            freeBlocks.push_back({ startAddr, memPerProc });

            // Sort and merge adjacent free blocks
            std::sort(freeBlocks.begin(), freeBlocks.end(),
                [](const MemoryBlock& a, const MemoryBlock& b) { return a.start < b.start; });
            for (size_t i = 0; i < freeBlocks.size() - 1; ++i) {
                if (freeBlocks[i].start + freeBlocks[i].size == freeBlocks[i + 1].start) {
                    freeBlocks[i].size += freeBlocks[i + 1].size; // Merge blocks
                    freeBlocks.erase(freeBlocks.begin() + i + 1); // Remove merged block
                    --i; // Recheck the current index
                }
            }
        }
    }


    // Calculate external fragmentation
    int calculateFragmentation() const {
        int fragmentation = 0;

        for (const auto& block : freeBlocks) {
            // Add the size of each free block to fragmentation
            fragmentation += block.size;
        }

        return fragmentation;
    }

    // Generate memory snapshot
    void generateSnapshot(int quantumCycle) const {
        std::ofstream file("memory_stamp_" + std::to_string(quantumCycle) + ".txt");
        if (!file.is_open()) {
            std::cerr << "Failed to open snapshot file.\n";
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
            int startAddr = process.second;
            int endAddr = startAddr + memPerProc;
            file << endAddr << "\n" << process.first << "\n" << startAddr << "\n";
        }

        file << "----start---- = 0\n";
        file.close();
    }

    void logMemoryState() const {
        std::cout << "----- Memory State -----\n";
        std::cout << "Allocated Memory:\n";
        for (const auto& process : processes) {
            std::cout << "  Process: " << process.first << ", Start: " << process.second
                << ", Size: " << memPerProc << " KB\n";
        }

        std::cout << "Free Memory Blocks:\n";
        for (const auto& block : freeBlocks) {
            std::cout << "  Start: " << block.start << ", Size: " << block.size << " KB\n";
        }
        std::cout << "-------------------------\n";
    }
};

#endif // MEMORYMANAGER_H
