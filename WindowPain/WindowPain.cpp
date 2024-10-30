// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <iostream>
#include <functional> // std::function
#include <ctime> // For timestamp
#include <sstream>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>

// Support for other OS
#ifdef _WIN32
#include <Windows.h> // For Windows
#else
#include <unistd.h> // For Linux, macOS
#endif

// Shortcuts
typedef std::string String;

template <typename T1, typename T2, typename T3>
auto clamp(const T1& v, const T2& lo, const T3& hi) -> typename std::common_type<T1, T2, T3>::type {
    using CommonType = typename std::common_type<T1, T2, T3>::type;
    return (v < static_cast<CommonType>(lo)) ? static_cast<CommonType>(lo)
        : (static_cast<CommonType>(hi) < v) ? static_cast<CommonType>(hi)
        : static_cast<CommonType>(v);
}

// Struch Forward Declarations
struct Config {
    int num_cpu = 1;
    std::string scheduler = "fcfs";
    int quantum_cycles = 1;
    int batch_process_freq = 1;
    int min_ins = 1;
    int max_ins = 1;
    int delays_per_exec = 0;
}config;


// ******* CLASSES *********************************************************************************

// Class forward declarations
class ConsoleManager;
class AConsole;
class MainMenuConsole;
class ScreenConsole;
class Screen;
class ScreenManager;
class Scheduler;
enum class ConsoleType { MainMenu, Screen };
enum class SchedulerType { FCFS, RR };

// Function prototypes
void printInColor(const String& text, const String& color);
void commandLoop(ConsoleManager& console);

// Console Manager
class ConsoleManager {
private:
    std::unique_ptr<AConsole> currentConsole;       // pointer to current console
    std::unique_ptr<ScreenManager> screenManager;   // pointer to ScreenManager
    ConsoleType currentConsoleType;                 // current console type
public:
    ConsoleManager() : screenManager(std::make_unique<ScreenManager>(*this)), currentConsoleType(ConsoleType::MainMenu) {}
    ScreenManager& getScreenManager() { return *screenManager; }
    ConsoleType getCurrentConsoleType() { return currentConsoleType; }
    void switchConsole(ConsoleType consoleType);    // switch to a specified console
    void passCommand(const String& command);        // passes command to the current console
};

// Screen Manager
class ScreenManager {
private:
    ConsoleManager& consoleManager;             // reference to the console manager
public:
    std::unordered_map<String, Screen> screens; // list of screens
    String currentScreen = "";                  // current screen displayed
    ScreenManager(ConsoleManager& cm) : consoleManager(cm) {};
    void screenCreate(const String& name);     // create screen
    void screenRestore(const String& name);    // inspect screen
    void screenList();                         // display screen list
};

// Screen
class Screen {
public:
    String name = "Untitled";       // process name saved by user
    int currentLine = 0;            // N/A
    int totalLines = -1;            // N/A
    String timestamp;               // timestamp of when screen was created
    int coreId = -1;                // The core assigned to this process
    bool finished = false;          // Added flag to indicate if process is finished

    Screen() = default;

    Screen(const String& name, int totalLines)
        : name(name), totalLines(totalLines), coreId(-1), finished(false) {}
};



// Abstract Console
class AConsole {
protected:
    std::unordered_map<String, std::function<void()>> commandMap;       // list of commands
    std::unordered_map<String, std::function<void(const String&)>> commandMapWithArgs;  // list of commands w/arguments
public:
    virtual ~AConsole() = default;  // destructor
    virtual void draw() = 0;        // virtual method for drawing the console
    void handleCommand(const String& command); // run command
};



// Main Menu Console
class MainMenuConsole : public AConsole {
private:
    ConsoleManager& consoleManager;   // reference to the console manager
    ScreenManager& screenManager;   // reference to the screen manager
    Scheduler* scheduler = nullptr;



    void loadConfig(const std::string& filename);
    void printTitle();      // prints the main menu title
    void help();            // list all commands for main menu console
    void initialize();      // N/A
    void screen();          // lists the commands for screen
    void schedulerTest();   // N/A
    void schedulerStop();   // N/A
    void reportUtil();      // N/A
    void clear();           // redraws the screen console
    void exitProgram();     // exits the program

public:
    std::atomic<bool> schedulerRunning{ false };
    std::thread schedulerThread;
    MainMenuConsole(ScreenManager& sm, ConsoleManager& cm) : screenManager(sm), consoleManager(cm) {
        // initializes the command map
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
    };
    void draw() override;   // draws the main menu console
};

class Scheduler {
private:
    std::queue<Screen*> screenQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool finished = false;
    std::vector<std::thread> cores;
    int numCores;
    int nextCore = 0;

    

    SchedulerType schedulerType;
    int quantumCycles;

    void worker(int coreId);
    void executeProcessFCFS(Screen* screen, int coreId);
    void executeProcessRR(Screen* screen, int coreId);
    

public:
    const Config& config; // Now Config is fully defined and can be used
    Scheduler(const Config& config);
    ~Scheduler();
    void addProcess(Screen& screen);
    void finish();
};

// Screen Console
class ScreenConsole : public AConsole {
private:
    ConsoleManager& consoleManager; // reference to the console manager
    ScreenManager& screenManager;   // reference to the screen manager
    void help();            // list all commands for screen console
    void screen();          // lists the commands for screen
    void clear();           // redraws the screen console
    void exitScreen();      // goes back to main menu console
public:
    ScreenConsole(ScreenManager& sm, ConsoleManager& cm) : screenManager(sm), consoleManager(cm) {
        commandMap["help"] = [this]() { help(); };
        commandMap["screen"] = [this]() { screen(); };
        commandMapWithArgs["screen -s"] = [this](const std::string& args) { screenManager.screenCreate(args); };
        commandMapWithArgs["screen -r"] = [this](const std::string& args) { screenManager.screenRestore(args); };
        commandMap["screen -ls"] = [this]() { screenManager.screenList(); };
        commandMap["clear"] = [this]() { clear(); };
        commandMap["exit"] = [this]() { exitScreen(); };
    };
    void draw() override;   // draws the screen console
};

// ******* CLASS FUNCTIONS *********************************************************************************

void AConsole::handleCommand(const String& input) {
    // check first command map
    auto it = commandMap.find(input);
    if (it != commandMap.end()) {
        // execute command
        it->second();
    }
    else {
        std::istringstream iss(input);
        String command;
        String option;
        String args;

        // get the command (first part and second part)
        iss >> command;
        iss >> option;

        if (command == "screen") {
            if (option == "-s" || option == "-r") {
                std::getline(iss >> std::ws, args);
                // check args
                if (args.empty()) {
                    printInColor("Error: An argument is required for the this command.\n", "red");
                }
                else {
                    // check second comman map
                    auto it = commandMapWithArgs.find("screen " + option);
                    if (it != commandMapWithArgs.end()) {
                        // execute command
                        it->second(args);
                    }
                    else {
                        printInColor("Unknown command. Type 'help' for available commands.\n", "red");
                    }
                }
            }
            else {
                printInColor("Unknown command. Type 'help' for available commands.\n", "red");
            }
        }
        else {
            printInColor("Unknown command. Type 'help' for available commands.\n", "red");
        }
    }
}

Scheduler::Scheduler(const Config& config)
    : config(config), finished(false), numCores(config.num_cpu), nextCore(0),
    schedulerType(config.scheduler == "rr" ? SchedulerType::RR : SchedulerType::FCFS),
    quantumCycles(config.quantum_cycles) {

    // Set up threads based on the number of CPUs from the config
    for (int i = 0; i < config.num_cpu; ++i) {
        cores.emplace_back(&Scheduler::worker, this, i);
    }
}

Scheduler::~Scheduler() {
    finished = true;
    cv.notify_all();
    for (auto& core : cores) {
        core.join();
    }
}


void Scheduler::worker(int coreId) {
    while (true) {
        if (finished) return;

        Screen* screen = nullptr;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] {
                return finished || !screenQueue.empty();
                });

            if (finished && screenQueue.empty()) {
                return;
            }

            if (nextCore == coreId && !screenQueue.empty()) {
                screen = screenQueue.front();
                screenQueue.pop();

                // Update the next core in a round-robin fashion
                nextCore = (nextCore + 1) % numCores;
            }
            else {
                continue;
            }
        }

        if (screen) {
            if (schedulerType == SchedulerType::FCFS) {
                executeProcessFCFS(screen, coreId);
            }
            else if (schedulerType == SchedulerType::RR) {
                executeProcessRR(screen, coreId);
            }
        }
    }
}

// FCFS: Complete execution of each screen process in a single pass
void Scheduler::executeProcessFCFS(Screen* screen, int coreId) {
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt");

    for (int i = 0; i < screen->totalLines; ++i) {
        if (screen->currentLine >= screen->totalLines) { // Extra safety check
            screen->finished = true;
            logFile.close();
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        // Get timestamp
        time_t now = time(0);
        tm ltm;

#ifdef _WIN32
        localtime_s(&ltm, &now);
#else
        localtime_r(&now, &ltm);
#endif
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

        // Write log entry
        logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
        logFile.flush();
        screen->currentLine++;
    }

    logFile.close();
    screen->finished = true;
}

// RR: Process each screen with quantum-based execution
void Scheduler::executeProcessRR(Screen* screen, int coreId) {
    screen->coreId = coreId;
    std::ofstream logFile(screen->name + ".txt", std::ios::app);

    int executedLines = 0;
    while (executedLines < screen->totalLines) {
        int linesToProcess = min(quantumCycles, screen->totalLines - executedLines);

        for (int i = 0; i < linesToProcess; ++i) {
            if (screen->currentLine >= screen->totalLines) { // Extra safety check
                screen->finished = true;
                logFile.close();
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
            // Get timestamp
            time_t now = time(0);
            tm ltm;

#ifdef _WIN32
            localtime_s(&ltm, &now);
#else
            localtime_r(&now, &ltm);
#endif
            char timestamp[25];
            strftime(timestamp, sizeof(timestamp), "(%m/%d/%Y %I:%M:%S %p)", &ltm);

            // Write log entry
            logFile << timestamp << " Core:" << coreId << " \"Hello world from " << screen->name << "!\"\n";
            logFile.flush();
            screen->currentLine++;
        }

        executedLines += linesToProcess;

        if (executedLines < screen->totalLines) {
            std::unique_lock<std::mutex> lock(queueMutex);
            screenQueue.push(screen);  // Requeue the process for the next quantum
            cv.notify_one();
            break;  // Yield control to other processes
        }
    }

    if (executedLines >= screen->totalLines) {
        logFile.close();
        screen->finished = true;
    }
}

void Scheduler::addProcess(Screen& screen) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        screenQueue.push(&screen);
    }
    cv.notify_one(); // Notify a waiting worker thread
}

void Scheduler::finish() {
    finished = true;
    cv.notify_all(); // Notify all threads to finish execution
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

void MainMenuConsole::loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }

    std::string parameter;

    while (file >> parameter) {
        if (parameter == "num-cpu") {
            int value;
            file >> value;
            config.num_cpu = clamp(value, 1, 128);
        }
        else if (parameter == "scheduler") {
            std::string schedulerValue;
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
            config.quantum_cycles = clamp(value, 1, 4294967296);
        }
        else if (parameter == "batch-process-freq") {
            int value;
            file >> value;
            config.batch_process_freq = clamp(value, 1, 4294967296);
        }
        else if (parameter == "min-ins") {
            int value;
            file >> value;
            config.min_ins = clamp(value, 1, 4294967296);
        }
        else if (parameter == "max-ins") {
            int value;
            file >> value;
            config.max_ins = clamp(value, 1, 4294967296);
        }
        else if (parameter == "delays-per-exec") {
            int value;
            file >> value;
            config.delays_per_exec = clamp(value, 0, 4294967296);
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
        printInColor("Error: " + std::string(e.what()) + "\n", "red");
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
    printInColor("Hi.\n", "red");
    scheduler = new Scheduler(config);

    printInColor("Initialization complete.\n", "green");
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
    std::cout << "'report-util' command recognized. Doing something.\n";
}

void MainMenuConsole::clear() {
    draw();
}

void MainMenuConsole::exitProgram() {
    printInColor("Toodles!", "yellow");
    std::cout << "\n";
    exit(0);
}

void MainMenuConsole::draw() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printTitle();
}

void MainMenuConsole::schedulerTest() {
    // Ensure only one instance of the scheduler runs at a time
    if (schedulerRunning) {
        printInColor("Scheduler is already running.\n", "red");
        return;
    }

    printInColor("Scheduler has started.\n\n", "yellow");
    schedulerRunning = true;

    // Clear all existing log files before starting
    for (const auto& screenEntry : screenManager.screens) {
        std::ofstream logFile(screenEntry.first + ".txt", std::ios::trunc);
        if (logFile.is_open()) {
            logFile.close();
        }
        else {
            std::cerr << "Error: Could not clear file " << screenEntry.first << ".txt\n";
        }
    }

    // Initialize the scheduler with the configuration
    scheduler = new Scheduler(config);

    // Create a new thread for the scheduler's background operation
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

            // Apply delay if delays_per_exec is specified
            if (config.delays_per_exec > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.delays_per_exec));
            }

            cycleCounter++;
        }

        printInColor("Scheduler background process stopped.\n", "green");
        });

    // Detach the thread to let it run in the background
    schedulerThread.detach();
}

void MainMenuConsole::schedulerStop() {
    if (schedulerRunning) {
        // Set the flag to stop the background scheduler loop
        schedulerRunning = false;
        if (scheduler) {
            scheduler->finish();
            delete scheduler;
            scheduler = nullptr;
        }
        printInColor("Scheduler stopped.\n", "green");
    }
    else {
        printInColor("Scheduler is not running.\n", "red");
    }
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

void ScreenManager::screenCreate(const String& name) {
    if (screens.find(name) != screens.end()) {
        printInColor("Screen already exists with this name.\n", "red");
        return;
    }

    time_t now = time(0);
    tm ltm;
#ifdef _WIN32
    localtime_s(&ltm, &now);
#else
    localtime_r(&now, &ltm);
#endif
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%m/%d/%Y %I:%M:%S %p", &ltm);

    // Create a new screen
    Screen newScreen(name, 100);
    newScreen.timestamp = timestamp;

    // Add to map and update current screen
    screens[name] = newScreen;
    currentScreen = name;
}

void ScreenManager::screenRestore(const String& name) {
    auto it = screens.find(name);
    if (it == screens.end()) {
        printInColor("No screen found with this name.\n", "red");
        return;
    }

    // Set current screen
    currentScreen = name;

    // Switch to screen console
    consoleManager.switchConsole(ConsoleType::Screen);
}

void ScreenManager::screenList() {

    std::cout << "\n---------------------------------------\n";
    std::cout << "Running processess:\n";
    
    int cnt_running = 0;
    if (!screens.empty()) {
        for (const auto& screen : screens) {
            if (!screen.second.finished && screen.second.coreId != -1) {
                cnt_running++;
                std::cout << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Core: " << std::setw(3) << std::left << screen.second.coreId << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_running == 0) {
        printInColor("No running processes.\n", "gray");
    }

    std::cout << "\nFinished processess:\n";

    int cnt_finished= 0;
    if(!screens.empty()) {
        for (const auto& screen : screens) {
            if (screen.second.finished) {
                cnt_finished++;
                std::cout << std::setw(10) << std::left << screen.first << "   "
                    << "(" << screens[screen.first].timestamp << ")    "
                    << "Finished" << std::left << "   "
                    << screen.second.currentLine << " / " << screen.second.totalLines << "\n";
            }
        }
    }
    if (cnt_finished == 0) {
        printInColor("No finished processes.\n", "gray");
    }

    std::cout << "---------------------------------------\n\n";
}

void ConsoleManager::switchConsole(ConsoleType consoleType) {
    switch (consoleType) {
    case ConsoleType::MainMenu:
        currentConsoleType = ConsoleType::MainMenu;
        currentConsole = std::make_unique<MainMenuConsole>(*screenManager, *this);
        break;
    case ConsoleType::Screen:
        currentConsoleType = ConsoleType::Screen;
        currentConsole = std::make_unique<ScreenConsole>(*screenManager, *this);
        break;
    }
    currentConsole->draw();
}

void ConsoleManager::passCommand(const String& command) {
    currentConsole->handleCommand(command);
}

// ******* HELPER FUNCTIONS *********************************************************************************

void printInColor(const String& text, const String& color) {
    // Define ANSI escape color codes
    static std::unordered_map<String, String> colorMap = {
        {"black", "\033[30m"},
        {"red", "\033[31m"},
        {"green", "\033[32m"},
        {"yellow", "\033[33m"},
        {"blue", "\033[34m"},
        {"magenta", "\033[35m"},
        {"cyan", "\033[36m"},
        {"white", "\033[37m"},
        {"gray", "\033[90m"},
        {"bright_red", "\033[91m"},
        {"bright_green", "\033[92m"},
        {"bright_yellow", "\033[93m"},
        {"bright_blue", "\033[94m"},
        {"bright_magenta", "\033[95m"},
        {"bright_cyan", "\033[96m"},
        {"bright_white", "\033[97m"},
        {"reset", "\033[0m"}
    };

    // Use ANSI codes to print in color if on Unix-based system
#ifdef _WIN32
    static std::unordered_map<String, WORD> winColorMap = {
        {"black", 0},
        {"blue", FOREGROUND_BLUE},
        {"green", FOREGROUND_GREEN},
        {"cyan", FOREGROUND_BLUE | FOREGROUND_GREEN},
        {"red", FOREGROUND_RED},
        {"magenta", FOREGROUND_RED | FOREGROUND_BLUE},
        {"yellow", FOREGROUND_RED | FOREGROUND_GREEN},
        {"white", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE},
        {"gray", FOREGROUND_INTENSITY},
        {"bright_blue", FOREGROUND_BLUE | FOREGROUND_INTENSITY},
        {"bright_green", FOREGROUND_GREEN | FOREGROUND_INTENSITY},
        {"bright_cyan", FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY},
        {"bright_red", FOREGROUND_RED | FOREGROUND_INTENSITY},
        {"bright_magenta", FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY},
        {"bright_yellow", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY},
        {"bright_white", FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY}
    };

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    WORD defaultColor = consoleInfo.wAttributes;

    auto it = winColorMap.find(color);
    if (it != winColorMap.end()) {
        SetConsoleTextAttribute(hConsole, it->second);
    }
    else {
        std::cout << "Error: Unknown color name.\n";
        return;
    }
    std::cout << text;
    SetConsoleTextAttribute(hConsole, defaultColor);

#else
// Print the text with ANSI color codes on non-Windows systems
    auto it = colorMap.find(color);
    if (it != colorMap.end()) {
        std::cout << it->second << text << colorMap["reset"];
    }
    else {
        std::cout << text; // Fallback if color not found
    }
#endif
}

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


// ******* MAIN *********************************************************************************

int main() {
    ConsoleManager consoleManager;
    consoleManager.switchConsole(ConsoleType::MainMenu);

    

    commandLoop(consoleManager);

    return 0;
}