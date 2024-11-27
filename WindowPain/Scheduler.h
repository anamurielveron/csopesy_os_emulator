#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Config.h"
#include "MemoryManager.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class Screen;
enum class SchedulerType { FCFS, RR };



class Scheduler {
private:
    MemoryManager* memoryManager;
    std::queue<Screen*> screenQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::vector<std::thread> cores;
    int numCores;
    int nextCore = 0;
    bool finished;
    std::atomic<int> globalQuantumCycle = 0;
    std::mutex snapshotMutex;

    SchedulerType schedulerType;
    int quantumCycles;

    void worker(int coreId);
    void executeProcessFCFS(Screen* screen, int coreId);
    void executeProcessRR(Screen* screen, int coreId);

public:
    const Config& config; // Now Config is fully defined and can be used
    Scheduler(const Config& config, MemoryManager* memoryManager);
    ~Scheduler();
    void addProcess(Screen& screen);
    void newProcess(Screen& screen);
    void finish();
};

#endif // SCHEDULER_H
