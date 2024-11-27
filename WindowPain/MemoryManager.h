#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "Screen.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>

struct MemoryBlock {
    int startAddress;       // Start address of the block
    int endAddress;         // End address of the block
    String processName;     // Name of the process occupying this block
};

class MemoryManager {
private:
    const int maxOverallMemory;     // Maximum memory available
    const int memoryPerProcess;     // Memory required per process
    const int memoryPerFrame;       // Size of memory frames (not used in this example but kept for flexibility)
    int numCpu;
    std::vector<MemoryBlock> memoryBlocks; // List of allocated memory blocks
    int totalExternalFragmentation; // Track external fragmentation
    int numProcessesInMemory;       // Number of running processes in memory
    std::mutex memoryMutex;

    // Internal helpers
    bool allocateMemory(const String& processName, int memoryRequired);
    void releaseMemory(const String& processName);
    void calculateFragmentation();

public:
    MemoryManager(int maxMemory, int memPerProc, int memPerFrame, int numberCPU);
    ~MemoryManager();

    // Public methods
    bool addProcess(const Screen& screen); // Add a process if it can fit
    void removeProcess(const Screen& screen); // Remove a process when it finishes
    void printMemorySnapshot(int quantumCycle); // Generate snapshot and write to file
    bool isInMemory(const Screen& screen) const;
};

#endif // MEMORYMANAGER_H
