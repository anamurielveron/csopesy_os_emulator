#include "MemoryManager.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <mutex>




MemoryManager::MemoryManager(int maxMemory, int memPerProc, int memPerFrame, int numberCPU)
    : maxOverallMemory(maxMemory),
    memoryPerProcess(memPerProc),
    memoryPerFrame(memPerFrame),
    numCpu(numberCPU),
    totalExternalFragmentation(0),
    numProcessesInMemory(0) {}

MemoryManager::~MemoryManager() {}

bool MemoryManager::allocateMemory(const String& processName, int memoryRequired) {
    if (processName.empty()) {
        printInColor("Error: Cannot allocate memory for an empty process name.\n", "red");
        return false;
    }

    int freeSpace = maxOverallMemory;

    // Calculate free space
    for (const auto& block : memoryBlocks) {
        freeSpace -= (block.endAddress - block.startAddress);
    }

    // Check if there's enough free space
    if (freeSpace >= memoryRequired) {
        int startAddress = 0;

        // Find a free space large enough
        for (const auto& block : memoryBlocks) {
            if (block.startAddress - startAddress >= memoryRequired) {
                memoryBlocks.push_back({ startAddress, startAddress + memoryRequired, processName });
                numProcessesInMemory++;
                return true;
            }
            startAddress = block.endAddress;
        }

        // Check after the last allocated block
        if (maxOverallMemory - startAddress >= memoryRequired) {
            memoryBlocks.push_back({ startAddress, startAddress + memoryRequired, processName });
            numProcessesInMemory++;
            return true;
        }
    }

    return false;  // Not enough memory available
}



void MemoryManager::releaseMemory(const String& processName) {
    memoryBlocks.erase(
        std::remove_if(memoryBlocks.begin(), memoryBlocks.end(),
            [&processName](const MemoryBlock& block) {
                return block.processName == processName;
            }),
        memoryBlocks.end()
    );
    numProcessesInMemory--;
}

void MemoryManager::calculateFragmentation() {
    totalExternalFragmentation = 0;
    if (memoryBlocks.empty()) {
        totalExternalFragmentation = maxOverallMemory;
        return;
    }

    // Sort blocks by startAddress
    std::sort(memoryBlocks.begin(), memoryBlocks.end(),
        [](const MemoryBlock& a, const MemoryBlock& b) {
            return a.startAddress < b.startAddress;
        });

    // Calculate fragmentation between blocks
    int previousEnd = 0;
    for (const auto& block : memoryBlocks) {
        totalExternalFragmentation += block.startAddress - previousEnd;
        previousEnd = block.endAddress;
    }

    // Include fragmentation after the last block
    totalExternalFragmentation += maxOverallMemory - previousEnd;
}


bool MemoryManager::addProcess(const Screen& screen) {
    auto it = std::find_if(memoryBlocks.begin(), memoryBlocks.end(),
        [&screen](const MemoryBlock& block) {
            return block.processName == screen.name;
        });

    if (it != memoryBlocks.end()) {
        return true;
    }

    // If there is not enough memory, attempt to free up space
    if (memoryBlocks.size() >= numCpu) {
        return false;
    }

    if (allocateMemory(screen.name, memoryPerProcess)) {
        return true;
    }

    return false;
}

void MemoryManager::removeProcess(const Screen& screen) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    // Find the process in memoryBlocks
    for (auto it = memoryBlocks.begin(); it != memoryBlocks.end();) {
        if (it->processName == screen.name) {
            it = memoryBlocks.erase(it); // Erase returns the next valid iterator
            numProcessesInMemory--; // Decrement process count
        }
        else {
            ++it; // Increment iterator if no erase occurred
        }
    }

}



void MemoryManager::printMemorySnapshot(int quantumCycle) {
    std::lock_guard<std::mutex> lock(memoryMutex);

    calculateFragmentation();

   
    std::vector<MemoryBlock> sortedBlocks = memoryBlocks;
    std::sort(sortedBlocks.begin(), sortedBlocks.end(), [](const MemoryBlock& a, const MemoryBlock& b) {
        return a.endAddress > b.endAddress; // Sort by high to low
        });

    // File name
    std::string fileName = "memory_stamp_" + std::to_string(quantumCycle) + ".txt";

    std::ofstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Error: Could not create snapshot file " << fileName << "\n";
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
    if (strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm) == 0) {
        file << "Timestamp unavailable\n";
    }
    else {
        file << "Quantum Cycle: " << quantumCycle << "\n";
        file << "Timestamp: " << timestamp << "\n";
    }

    file << "Number of processes in memory: " << numProcessesInMemory << "\n";
    file << "Total external fragmentation in KB: " << totalExternalFragmentation / 1024 << "\n";
    file << "----end---- = " << maxOverallMemory << "\n";

    for (const auto& block : sortedBlocks) {
        file << block.endAddress - 1 << "\n";
        file << block.processName << "\n";
        file << block.startAddress << "\n";
    }

    file << "----start---- = 0\n";
    file.close();
}



bool MemoryManager::isInMemory(const Screen& screen) const {
    auto it = std::find_if(memoryBlocks.begin(), memoryBlocks.end(),
        [&screen](const MemoryBlock& block) {
            return block.processName == screen.name;
        });
    return it != memoryBlocks.end();
}
