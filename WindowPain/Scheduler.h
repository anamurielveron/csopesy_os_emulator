#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class Screen;

class Scheduler {
private:
    std::queue<Screen*> screenQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool finished;
    std::vector<std::thread> cores;
    int numCores;
    int nextCore;

    void worker(int coreId);
    void executeProcess(Screen* screen, int coreId);

public:
    Scheduler(int numCores);
    ~Scheduler();
    void addProcess(Screen& screen);
    void finish();
};

#endif // SCHEDULER_H
