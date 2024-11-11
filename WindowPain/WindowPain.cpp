// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <atomic>
#include <thread>
#include "Utils.h"
#include "ConsoleManager.h"
#include "MainMenuConsole.h"
#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "Scheduler.h"

void commandLoop(ConsoleManager& console) {
    String input;
    bool isInitialized = false;
    std::mutex cpuMutex;

    // CPU cycle thread
    std::thread cpuCycleThread([&console, &cpuMutex]() {
        while (true) {
            // Speed of CPU Cycler
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // TODO: CPU counter should increment first before other threads do their task.
            std::lock_guard<std::mutex> lock(cpuMutex);
            console.getScreenManager().cpuCycles++;

            // DONE: Implement process generation 
            // TODO: Implement batch_process_freq in process generation
            // TODO: Implement queues
            // TODO: Implement cpu cores
            // TODO: Implement FCFS scheduler
            // TODO: Implement RR scheduler
        }
    });
    cpuCycleThread.detach();


    // Command loop
    while (true) {
        if (console.getCurrentConsoleType() == ConsoleType::MainMenu) {
            printInColor("Enter a command: ", "cyan");
        }
        else if (console.getCurrentConsoleType() == ConsoleType::Screen) {
            printInColor("[" + console.getScreenManager().currentScreen + "]$ ", "cyan");
        }
        std::getline(std::cin, input);

        // After initialization, lift command restriction
        if (isInitialized) {
            console.passCommand(input);
        }
        // If not yet initialized, restrict commands
        else if (!isInitialized) {
            if (input == "initialize") {
                console.passCommand(input);
                isInitialized = true;
            }
            else if (input == "exit" || input == "clear") {
                console.passCommand(input);
            }
            else if (input == "help") {
                std::cout << "\n";
                std::cout << "Available commands:\n";
                printInColor("initialize\n", "green");
                printInColor("clear\n", "green");
                printInColor("exit\n", "green");
                std::cout << "\n";
                std::cout << "Restricted commands:\n";
                printInColor("screen\n", "red");
                printInColor("scheduler-test\n", "red");
                printInColor("scheduler-stop\n", "red");
                printInColor("report-util\n", "red");
                std::cout << "\n";
            }
            else {
                printInColor("Other commands are restricted until initialization. Type 'help' for available commands.\n\n", "red");
            }
        }
    }
}

int main() {
    ConsoleManager consoleManager;
    consoleManager.switchConsole(ConsoleType::MainMenu);
    commandLoop(consoleManager);

    return 0;
}