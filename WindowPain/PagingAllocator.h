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
    int totalMemory;           // Total memory available
    int pageSize;              // Size of each page/frame
    int minMemPerProc;         // Minimum memory per process
    int maxMemPerProc;         // Maximum memory per process
    std::vector<Frame> frames; // Frames available for allocation
    std::unordered_map<std::string, std::vector<int>> pageTables; // Page tables per process
    std::unordered_map<std::string, int> processMemorySizes;

    int randomMemorySize() const {
        return minMemPerProc + (std::rand() % (maxMemPerProc - minMemPerProc + 1));
    }

public:
    PagingAllocator(int totalMem, int pageSz, int minMemProc, int maxMemProc)
        : totalMemory(totalMem), pageSize(pageSz), minMemPerProc(minMemProc), maxMemPerProc(maxMemProc) {
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

};

#endif // PAGINGALLOCATOR_H

