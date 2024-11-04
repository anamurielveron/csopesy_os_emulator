// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include "Utils.h"
#include "ConsoleManager.h"
#include "MainMenuConsole.h"
#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "Scheduler.h"

void commandLoop(ConsoleManager& console) {
    String input;
    bool isInitialized = false;

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
                printInColor("initialize", "green");
                std::cout << "\n";
                printInColor("screen", "red");
                std::cout << "\n";
                printInColor("scheduler-test", "red");
                std::cout << "\n";
                printInColor("scheduler-stop", "red");
                std::cout << "\n";
                printInColor("report-util", "red");
                std::cout << "\n";
                printInColor("clear", "green");
                std::cout << "\n";
                printInColor("exit", "green");
                std::cout << "\n";
                std::cout << "\n";
            }
            else {
                printInColor("Other commands are disabled until initialization. Type 'help' for available commands.\n\n", "red");
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