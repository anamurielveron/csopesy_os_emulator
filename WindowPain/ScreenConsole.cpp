#include "ScreenConsole.h"
#include "ScreenManager.h"
#include "Screen.h"
#include "Utils.h"


#include <iostream>

ScreenConsole::ScreenConsole(ScreenManager& sm, ConsoleManager& cm)
    : screenManager(sm), consoleManager(cm) {
    commandMap["help"] = [this]() { help(); };
    commandMap["screen"] = [this]() { screen(); };
    commandMapWithArgs["screen -s"] = [this](const std::string& args) { screenManager.screenCreate(args); };
    commandMapWithArgs["screen -r"] = [this](const std::string& args) { screenManager.screenRestore(args); };
    commandMap["screen -ls"] = [this]() { screenManager.screenList(); };
}

void ScreenConsole::draw() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    String currentScreen = screenManager.currentScreen;

    std::cout << "Screen Name: " << screenManager.screens[currentScreen].name << "\n";
    std::cout << "Current Line: " << screenManager.screens[currentScreen].currentLine << " / " << screenManager.screens[currentScreen].totalLines << "\n";
    std::cout << "Timestamp: " << screenManager.screens[currentScreen].timestamp << "\n\n";
}

void ScreenConsole::help() {
    std::cout << "\nAvailable commands:\n";
    printInColor("screen", "green");
    std::cout << "\n";
    printInColor("clear", "green");
    std::cout << "\n";
    printInColor("exit", "green");
    std::cout << "\n\n";
}

void ScreenConsole::screen() {
    std::cout << "\n'screen' commands:\n";
    printInColor("screen -s <name>", "green");
    std::cout << "\t(create a new screen)\n";
    printInColor("screen -r <name>", "green");
    std::cout << "\t(restore an existing screen)\n";
    printInColor("screen -ls", "green");
    std::cout << "\t\t(list all screens)\n\n";
}

void ScreenConsole::clear() {
    draw();
}

void ScreenConsole::exitScreen() {
    screenManager.currentScreen = "";
    consoleManager.switchConsole(ConsoleType::MainMenu);
}
