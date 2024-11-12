// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Utils.h"
#include "ConsoleManager.h"
#include "MainMenuConsole.h"
#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "Scheduler.h"

void commandLoop(ConsoleManager& console) {
    String input;
    bool isInitialized = false;

    // CPU cycle thread
    std::thread cpuCycleThread([&console]() {
        while (true) {
            // Speed of CPU Cycler
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            //std::lock_guard<std::mutex> lock(console.getScreenManager().mtx);
            console.getScreenManager().cpuCycles++;
            console.getScreenManager().cycleCv.notify_all();

            // DONE: Integrate CPU cycler with process generation 
            // DONE: Implement batch_process_freq in process generation
            // DONE: Integrate CPU cycler with process execution
            // DONE: Implement delays_per_exec in process execution
            // DONE: Fix order of screens when entering screen -ls
            // TODO: Make CPU cycles run faster
            // TODO: Fix implementation of core assigning and queues
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