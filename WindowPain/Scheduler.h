#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "MemoryManager.h"
#include "Config.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class Screen;
enum class SchedulerType { FCFS, RR };

class Scheduler {
private:
    MemoryManager memoryManager;
    std::queue<Screen*> screenQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool finished = false;
    std::vector<std::thread> cores;
    int numCores;
    int nextCore = 0;


    SchedulerType schedulerType;
    int quantumCycles;

    void worker(int coreId);
    void executeProcessFCFS(Screen* screen, int coreId);
    void executeProcessRR(Screen* screen, int coreId);

public:
    const Config& config; // Now Config is fully defined and can be used
    Scheduler(const Config& config);
    ~Scheduler();
    void addReadyQueue(Screen& screen);
    void finish();
    void logQueueState();
};

#endif // SCHEDULER_H
