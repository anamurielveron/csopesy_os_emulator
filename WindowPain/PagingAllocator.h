#ifndef PAGINGALLOCATOR_H
#define PAGINGALLOCATOR_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib> // For rand()

struct Frame {
    int frameId;
    bool isFree;
};

class PagingAllocator {
private:
    int maxMemory;           // Total memory available
    int pageSize;              // Size of each page/frame
    int minMemPerProc;         // Minimum memory per process
    int maxMemPerProc;         // Maximum memory per process
    std::vector<Frame> frames; // Frames available for allocation
    std::unordered_map<std::string, std::vector<int>> pageTables; // Page tables per process
    std::unordered_map<std::string, int> processMemorySizes;

    int totalMemory;

    int randomMemorySize() const {
        return minMemPerProc + (std::rand() % (maxMemPerProc - minMemPerProc + 1));
    }

public:
    PagingAllocator(int totalMem, int pageSz, int minMemProc, int maxMemProc)
        : maxMemory(totalMem), pageSize(pageSz), minMemPerProc(minMemProc), maxMemPerProc(maxMemProc), totalMemory(totalMem) {
        int numFrames = totalMem / pageSz;
        for (int i = 0; i < numFrames; ++i) {
            frames.push_back({ i, true }); // Initialize all frames as free
        }
    }

    int getFreeFrames() const {
        int count = 0;
        for (const auto& frame : frames) {
            if (frame.isFree) count++;
        }
        return count;
    }

    bool allocatePages(const std::string& processName) {
        int memoryRequirement;

        // Check if memory size is already assigned to this process
        auto it = processMemorySizes.find(processName);
        if (it != processMemorySizes.end()) {
            memoryRequirement = it->second; // Use existing size
        }
        else {
            // Randomize new size and store it
            memoryRequirement = randomMemorySize();
            processMemorySizes[processName] = memoryRequirement;
        }

        int numPages = (memoryRequirement + pageSize - 1) / pageSize; // Calculate pages required

        // Check if enough free frames are available
        if (numPages > getFreeFrames()) {
            return false;
        }

        // Allocate frames for the process
        std::vector<int> allocatedFrames;
        for (int i = 0; i < numPages; ++i) {
            for (auto& frame : frames) {
                if (frame.isFree) {
                    frame.isFree = false;
                    allocatedFrames.push_back(frame.frameId);
                    break;
                }
            }
        }

        // Store the allocation in the page table
        pageTables[processName] = allocatedFrames;
        return true;
    }

    void deallocatePages(const std::string& processName) {
        auto it = pageTables.find(processName);
        if (it != pageTables.end()) {
            // Free all frames allocated to this process
            for (int frameId : it->second) {
                frames[frameId].isFree = true;
            }
            pageTables.erase(it);
        }
        processMemorySizes.erase(processName);
    }

    // Get the total memory in the system
    int getTotalMemory() const {
        return totalMemory;
    }

    // Get the memory currently in use
    int getUsedMemory() const {
        int usedFrames = frames.size() - getFreeFrames();
        return usedFrames * pageSize;
    }

    // Get the available (free) memory
    int getFreeMemory() const {
        return getFreeFrames() * pageSize;
    }

    // Get the running processes and their memory usage
    std::unordered_map<std::string, int> getRunningProcesses() const {
        std::unordered_map<std::string, int> runningProcesses;
        for (const auto& entry : pageTables) {
            const std::string& processName = entry.first;
            int memoryUsage = entry.second.size() * pageSize;
            runningProcesses[processName] = memoryUsage;
        }
        return runningProcesses;
    }

    void printMemoryState() const {
        std::cout << "----- Memory State -----\n";
        std::cout << "Frames:\n";
        for (const auto& frame : frames) {
            std::cout << "  Frame ID: " << frame.frameId << ", Status: " << (frame.isFree ? "Free" : "Allocated") << "\n";
        }
        std::cout << "Page Tables:\n";
        for (const auto& entry : pageTables) {
            std::cout << "  Process: " << entry.first << ", Frames: ";
            for (int frameId : entry.second) {
                std::cout << frameId << " ";
            }
            std::cout << "\n";
        }
        std::cout << "-------------------------\n";
    }

    void generateSnapshot(int quantumCycle) const {
        std::ofstream file("paging_snapshot_" + std::to_string(quantumCycle) + ".txt");
        if (!file.is_open()) {
            std::cerr << "[ERROR] Failed to open snapshot file.\n";
            return;
        }

        // Snapshot content
        file << "----- Paging Memory Snapshot -----\n";
        file << "Quantum Cycle: " << quantumCycle << "\n";
        file << "Total Frames: " << frames.size() << "\n";
        file << "Free Frames: " << getFreeFrames() << "\n\n";

        file << "Frames:\n";
        for (const auto& frame : frames) {
            file << "  Frame ID: " << frame.frameId << ", Status: " << (frame.isFree ? "Free" : "Allocated") << "\n";
        }

        file << "\nPage Tables:\n";
        for (const auto& entry : pageTables) {
            file << "  Process: " << entry.first << ", Frames: ";
            for (int frameId : entry.second) {
                file << frameId << " ";
            }
            file << "\n";
        }
        file << "-----------------------------------\n";

        file.close();
    }

    void processSMI() {
        std::cout << "----- PROCESS-SMI VOL. 1.00 Driver Version: 01.00 -----\n";
        std::cout << "CPU-Util: 100% placeholder\n"; // Replace with actual CPU usage if available

        std::cout << frames.size() << " " << getFreeFrames() << " \n";
        int usedFrames = frames.size() - getFreeFrames();

        std::cout << "Memory Usage: " << (usedFrames * pageSize) / 1024 << "MiB / " << totalMemory / 1024 << "MiB\n";
        std::cout << "Memory Util: " << std::fixed << std::setprecision(2)
            << (static_cast<double>(usedFrames * pageSize) / totalMemory) * 100 << "%\n";

        auto processes = getRunningProcesses();

        std::cout << "\nRunning processes and memory usage:\n";
        for (const auto& process : processes) {
            const std::string& processName = process.first;
            int memoryUsage = process.second;
            std::cout << processName << ": " << memoryUsage / 1024 << "MiB\n";
        }

        std::cout << "-------------------------------------------------------\n";
    }

};

#endif // PAGINGALLOCATOR_H

