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

    // Command loop
    while (true) {
        if (console.getCurrentConsoleType() == ConsoleType::MainMenu) {
            printInColor("Enter a command: ", "cyan");
        }
        else if (console.getCurrentConsoleType() == ConsoleType::Screen) {
            printInColor("[" + console.getScreenManager().currentScreen + "]$ ", "cyan");
        }
        std::getline(std::cin, input);
        console.passCommand(input);
    }
}

int main() {
    ConsoleManager consoleManager;
    consoleManager.switchConsole(ConsoleType::MainMenu);
    commandLoop(consoleManager);

    return 0;
}