#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "AConsole.h"
#include "Screen.h"
#include "Utils.h"

#include <iostream>
#include <unordered_map>
#include <string>
#include <functional>
#include <cstdlib>

ScreenConsole::ScreenConsole(ScreenManager& sm, ConsoleManager& cm)
    : screenManager(sm), consoleManager(cm) {
    commandMap["help"] = [this]() { help(); };
    commandMap["clear"] = [this]() { clear(); };
    commandMap["exit"] = [this]() { exitScreen(); };
    commandMap["process-smi"] = [this]() { processSMI(); };
}

void ScreenConsole::draw() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    processSMI();
}

void ScreenConsole::processSMI() {
    String currentScreenString = screenManager.currentScreen;
    Screen currentScreen = screenManager.screens[currentScreenString];

    std::cout << "\nScreen Name: " << currentScreen.name << "\n";
    std::cout << "Timestamp: " << currentScreen.timestamp << "\n";
    std::cout << "Current Line: " << currentScreen.currentLine << " / " << currentScreen.totalLines << "\n";

    if (currentScreen.currentLine >= currentScreen.totalLines) {
        printInColor("This process has finished!", "green");
    }

    std::cout << "\n";
}

void ScreenConsole::help() {
    std::cout << "\nAvailable commands:\n";
    printInColor("process-smi", "green");
    std::cout << "\n";
    printInColor("clear", "green");
    std::cout << "\n";
    printInColor("exit", "green");
    std::cout << "\n\n";
}

void ScreenConsole::clear() {
    draw();
}

void ScreenConsole::exitScreen() {
    screenManager.currentScreen = "";
    consoleManager.switchConsole(ConsoleType::MainMenu);
}

