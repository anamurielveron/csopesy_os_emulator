// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <windows.h>
#include <string>
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

void cleanMemorySnapshots(const std::string& directory) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA((directory + "\\memory_stamp_*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] No matching files found.\n";
        return;
    }

    do {
        std::string filePath = directory + "\\" + findFileData.cFileName;
        if (DeleteFileA(filePath.c_str())) {
            std::cout << "[INFO] Deleted: " << filePath << std::endl;
        }
        else {
            std::cerr << "[ERROR] Could not delete: " << filePath << std::endl;
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}


int main() {
    cleanMemorySnapshots("./");
    ConsoleManager consoleManager;
    consoleManager.switchConsole(ConsoleType::MainMenu);
    commandLoop(consoleManager);

    return 0;
}