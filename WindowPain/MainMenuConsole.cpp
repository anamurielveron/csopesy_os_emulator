#include "MainMenuConsole.h"
#include "ConsoleManager.h"
#include "ScreenManager.h"
#include "Screen.h"
#include "Utils.h"

#include <fstream>
#include <iostream>
#include <random>

MainMenuConsole::MainMenuConsole(ScreenManager& sm, ConsoleManager& cm)
    : screenManager(sm), consoleManager(cm) {
    // initializes the command map
    commandMap["help"] = [this]() { help(); };
    commandMap["initialize"] = [this]() { initialize(); };
    commandMap["screen"] = [this]() { screen(); };
    commandMapWithArgs["screen -s"] = [this](const String& args) {
        screenManager.screenCreate(args, "screenCreate");
        if (screenManager.currentScreen != "") {
            consoleManager.switchConsole(ConsoleType::Screen);
        }
    };
    commandMapWithArgs["screen -r"] = [this](const String& args) { screenManager.screenRestore(args); };
    commandMap["screen -ls"] = [this]() { screenManager.screenList("screenList"); };
    commandMap["scheduler-test"] = [this]() { schedulerTest(); };
    commandMap["scheduler-stop"] = [this]() { schedulerStop(); };
    commandMap["report-util"] = [this]() { reportUtil(); };
    commandMap["clear"] = [this]() { clear(); };
    commandMap["exit"] = [this]() { exitProgram(); };
}

void MainMenuConsole::draw() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printTitle();
}
void MainMenuConsole::printTitle() {
    // open file
    std::ifstream file("..\\WindowPain\\TitleASCII.txt");

    // check if file exists
    if (!file) {
        printInColor("Error: TitleASCII.txt not found.\n", "red");
        return;
    }

    // print file
    String line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;  // Print each line to the console
    }

    // close file
    file.close();

    // print subtitle
    std::cout << "\n";
    printInColor("-------------------------------------------------------\n", "yellow");
    printInColor("Welcome to our CSOPESY commandline: WindowPain!\n", "yellow");
    std::cout << "\n";
    printInColor("Developers:\n", "yellow");
    printInColor("Bacosa, Gabriel Luis\n", "yellow");
    printInColor("De Leon, Allan David\n", "yellow");
    printInColor("Mojica, Harold\n", "yellow");
    printInColor("Veron, Ana Muriel\n", "yellow");
    std::cout << "\n";
    printInColor("Last updated: 11/03/2024\n", "yellow");
    printInColor("-------------------------------------------------------\n", "yellow");
    std::cout << "\n";
}

void MainMenuConsole::help() {
    std::cout << "\n";
    std::cout << "Available commands:\n";
    printInColor("initialize", "green");
    std::cout << "\n";
    printInColor("screen", "green");
    std::cout << "\n";
    printInColor("scheduler-test", "green");
    std::cout << "\n";
    printInColor("scheduler-stop", "green");
    std::cout << "\n";
    printInColor("report-util", "green");
    std::cout << "\n";
    printInColor("clear", "green");
    std::cout << "\n";
    printInColor("exit", "green");
    std::cout << "\n";
    std::cout << "\n";
}

void MainMenuConsole::screen() {
    std::cout << "\n";
    std::cout << "'screen' commands:\n";
    printInColor("screen -s <name>", "green");
    std::cout << "\t(create a new screen)\n";
    printInColor("screen -r <name>", "green");
    std::cout << "\t(restore an existing screen)\n";
    printInColor("screen -ls", "green");
    std::cout << "\t\t(list all screens)\n";
    std::cout << "\n";
}


void MainMenuConsole::initialize() {
    screenManager.initialize();
}

void MainMenuConsole::reportUtil() {
    screenManager.screenList("reportUtil");
}

void MainMenuConsole::schedulerTest() {
    screenManager.schedulerTest();
}

void MainMenuConsole::schedulerStop() {
    screenManager.schedulerStop();
}

void MainMenuConsole::clear() {
    draw();
}

void MainMenuConsole::exitProgram() {
    printInColor("Toodles!", "yellow");
    std::cout << "\n";
    exit(0);
}