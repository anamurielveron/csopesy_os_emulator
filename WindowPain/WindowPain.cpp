#include <iostream>
#include "ConsoleManager.h"
#include "MainMenuConsole.h"
#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "Scheduler.h"
#include "Utils.h"

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
    commandLoop(consoleManager);
    return 0;
}