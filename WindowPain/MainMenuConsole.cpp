#include "MainMenuConsole.h"
#include "ScreenManager.h"
#include "Screen.h"
#include "Utils.h"

#include <fstream>
#include <iostream>

MainMenuConsole::MainMenuConsole(ScreenManager& sm, ConsoleManager& cm)
    : screenManager(sm), consoleManager(cm), scheduler(nullptr) {
    commandMap["help"] = [this]() { help(); };
    commandMap["initialize"] = [this]() { initialize(); };
    commandMap["screen"] = [this]() { screen(); };
    commandMapWithArgs["screen -s"] = [this](const std::string& args) {
        screenManager.screenCreate(args);
        consoleManager.switchConsole(ConsoleType::Screen);
        };
    commandMapWithArgs["screen -r"] = [this](const std::string& args) { screenManager.screenRestore(args); };
    commandMap["screen -ls"] = [this]() { screenManager.screenList(); };
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
    printInColor("Welcome to our CSOPESY commandline: WindowPain!", "yellow");
    std::cout << "\n\n";
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

// TODO: Implement
void MainMenuConsole::initialize() {
	std::cout << "'initialize' command recognized. Doing something.\n"; // Placeholder
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

void MainMenuConsole::schedulerTest() {
    printInColor("Scheduler has started.\n\n", "yellow");

    // Initialize scheduler with 4 cores
    scheduler = new Scheduler(4);

    // Create and add processes
    for (int i = 1; i <= 10; ++i) {
        String screenName = "process" + std::string((i < 10 ? "0" : "") + std::to_string(i));
        screenManager.screenCreate(screenName);
        scheduler->addProcess(screenManager.screens[screenName]);
    }
}

void MainMenuConsole::schedulerStop() {
    if (scheduler) {
        scheduler->finish();
        delete scheduler;
        scheduler = nullptr;
    }
    printInColor("Scheduler stopped.\n", "green");
}

// TODO: Implement
void MainMenuConsole::reportUtil() {
	std::cout << "'report-util' command recognized. Doing something.\n"; // Placeholder
}

void MainMenuConsole::clear() {
    draw();
}

void MainMenuConsole::exitProgram() {
    printInColor("Toodles!", "yellow");
    std::cout << "\n";
    exit(0);
}
