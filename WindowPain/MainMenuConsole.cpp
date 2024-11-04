#include "MainMenuConsole.h"
#include "ConsoleManager.h"
#include "ScreenManager.h"
#include "Screen.h"
#include "Utils.h"
#include "Config.h"

#include <fstream>
#include <iostream>
#include <random>

MainMenuConsole::MainMenuConsole(ScreenManager& sm, ConsoleManager& cm)
    : screenManager(sm), consoleManager(cm), scheduler(nullptr) {
    // initializes the command map
    commandMap["help"] = [this]() { help(); };
    commandMap["initialize"] = [this]() { initialize(); };
    commandMap["screen"] = [this]() { screen(); };
    commandMapWithArgs["screen -s"] = [this](const String& args) {
        screenManager.screenCreate(args);
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

void MainMenuConsole::loadConfig(const String& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }

    String parameter;

    while (file >> parameter) {
        if (parameter == "num-cpu") {
            int value;
            file >> value;
            config.num_cpu = clamp(value, 1, 128);
        }
        else if (parameter == "scheduler") {
            String schedulerValue;
            file >> schedulerValue;
            if (schedulerValue == "fcfs" || schedulerValue == "rr") {
                config.scheduler = schedulerValue;
            }
            else {
                throw std::runtime_error("Invalid scheduler value.");
            }
        }
        else if (parameter == "quantum-cycles") {
            int value;
            file >> value;
            config.quantum_cycles = clamp(value, 1, 4294967296); // [1, 2^32]
        }
        else if (parameter == "batch-process-freq") {
            int value;
            file >> value;
            config.batch_process_freq = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "min-ins") {
            int value;
            file >> value;
            config.min_ins = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "max-ins") {
            int value;
            file >> value;
            config.max_ins = clamp(value, 1, 4294967296); // [1, 2^32
        }
        else if (parameter == "delays-per-exec") {
            int value;
            file >> value;
            config.delays_per_exec = clamp(value, 0, 4294967296); // [0, 2^32
        }
        else {
            std::cerr << "Unknown parameter in config file: " << parameter << std::endl;
        }
    }

    file.close();
}

void MainMenuConsole::initialize() {
    try {
        loadConfig("config.txt");
    }
    catch (const std::exception& e) {
        printInColor("Error: " + String(e.what()) + "\n", "red");
        return;
    }

    std::cout << "Configuration Loaded:\n";
    std::cout << "Number of CPUs: " << config.num_cpu << "\n";
    std::cout << "Scheduler: " << config.scheduler << "\n";
    std::cout << "Quantum Cycles: " << config.quantum_cycles << "\n";
    std::cout << "Batch Process Frequency: " << config.batch_process_freq << "\n";
    std::cout << "Minimum Instructions: " << config.min_ins << "\n";
    std::cout << "Maximum Instructions: " << config.max_ins << "\n";
    std::cout << "Delays per Exec: " << config.delays_per_exec << "\n";

    if (scheduler) {
        delete scheduler;
    }
    scheduler = new Scheduler(config);

    printInColor("Initialization complete.\n\n", "green");
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

void MainMenuConsole::reportUtil() {
    screenManager.screenList("reportUtil");
}

void MainMenuConsole::schedulerTest() {
    // Ensure only one instance of the scheduler runs at a time
    if (schedulerRunning) {
        printInColor("Scheduler is already running.\n", "red");
        return;
    }

    printInColor("Scheduler has started.\n\n", "yellow");
    schedulerRunning = true;

    // Clear all existing log files before starting (Just in case there are files with the exact name process)
    for (const auto& screenEntry : screenManager.screens) {
        std::ofstream logFile(screenEntry.first + ".txt", std::ios::trunc);
        if (logFile.is_open()) {
            logFile.close();
        }
        else {
            std::cerr << "Error: Could not clear file " << screenEntry.first << ".txt\n";
        }
    }

    // Initialize the scheduler with the config
    scheduler = new Scheduler(config);

    // Scheduler's background operation
    schedulerThread = std::thread([this]() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(config.min_ins, config.max_ins);

        int cycleCounter = 0;

        // Background scheduler loop
        while (schedulerRunning) {
            // Add a new process at intervals defined by batch_process_freq
            if (cycleCounter % config.batch_process_freq == 0) {
                String screenName = "process" + std::to_string(cycleCounter / config.batch_process_freq);
                int instructionCount = dist(gen);

                // Create a new screen (process) and set its instruction count
                screenManager.screenCreate(screenName);
                screenManager.screens[screenName].totalLines = instructionCount;

                // Add the new process to the scheduler
                scheduler->addProcess(screenManager.screens[screenName]);
            }

            // Apply delay
            if (config.delays_per_exec > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.delays_per_exec));
            }

            cycleCounter++;
        }

        printInColor("Scheduler background process stopped.\n\n", "yellow");
        });

    // Detach the thread to let it run in the background
    schedulerThread.detach();
}

void MainMenuConsole::schedulerStop() {
    if (schedulerRunning) {
        // Stop the background scheduler loop
        schedulerRunning = false;
        if (scheduler) {
            scheduler->finish();
            delete scheduler;
            scheduler = nullptr;
        }
    }
    else {
        printInColor("Scheduler is not running.\n\n", "red");
    }
}

void MainMenuConsole::clear() {
    draw();
}

void MainMenuConsole::exitProgram() {
    printInColor("Toodles!", "yellow");
    std::cout << "\n";
    exit(0);
}