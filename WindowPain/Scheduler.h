#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Config.h"
#include "ScreenManager.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class Screen;
class ScreenManager;
enum class SchedulerType { FCFS, RR };

class Scheduler {
private:
    ScreenManager& screenManager;

    std::queue<Screen*> screenQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool finished = false;
    std::vector<std::thread> cores;
    int numCores;
    int nextCore = 0;

    SchedulerType schedulerType;
    int configQuantumCycles;

    void worker(int coreId);
    void executeProcessFCFS(Screen* screen, int coreId);
    void executeProcessRR(Screen* screen, int coreId);

public:
    const Config& config; // Now Config is fully defined and can be used
    Scheduler(const Config& config, ScreenManager& sm);
    ~Scheduler();
    void addProcess(Screen& screen);
    void finish();
};

#endif // SCHEDULER_H
