#include "Scheduler.h"
#include "Screen.h"

#include <fstream>
#include <chrono>
#include <ctime>

Scheduler::Scheduler(int numCores) : finished(false), numCores(numCores), nextCore(0) {
    for (int i = 0; i < numCores; ++i) {
        cores.emplace_back(&Scheduler::worker, this, i);
    }
}

Scheduler::~Scheduler() {
    finished = true;
    cv.notify_all();
    for (auto& core : cores) {
        core.join();
    }
}

void Scheduler::addProcess(Screen& screen) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        screenQueue.push(&screen);
    }
    cv.notify_one();
}

void Scheduler::finish() {
    finished = true;
    cv.notify_all();
}

void Scheduler::worker(int coreId) {
    while (true) {
        Screen* screen = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return finished || !screenQueue.empty();
                });

            if (finished && screenQueue.empty()) {
                return;
            }

            if (nextCore == coreId && !screenQueue.empty()) {
                screen = screenQueue.front();
                screenQueue.pop();
                nextCore = (nextCore + 1) % numCores;
            }
            else {
                continue;
            }
        }

        if (screen) {
            executeProcess(screen, coreId);
        }
    }
}

void Scheduler::executeProcess(Screen* screen, int coreId) {
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt");

    for (int i = 0; i < screen->totalLines; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        time_t now = time(0);
        tm ltm;

#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
        screen->currentLine++;
    }

    logFile.close();
    screen->finished = true;
}
