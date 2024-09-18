// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <iostream>
#include <functional> // std::function
#include <ctime> // For timestamp
#include <sstream>

#ifdef _WIN32
#include <Windows.h> // For windows
#else
#include <unistd.h> // For Linux, macOS
#endif

// Screen struct to hold screen information
struct Screen {
    std::string processName;
    int currentLine;
    int totalLines;
    std::string timestamp;
};

std::unordered_map<std::string, Screen> screens;
std::string currentScreen = "";


// Function prototypes for good practice
void initialize();
void showMainMenu();
void schedulerTest();
void schedulerStop();
void reportUtil();
void clearScreen();
void exitProgram();
void screenCreate(const std::string& name);
void screenRestore(const std::string& name);
void handleScreenCommands(const std::string& input);
void screenList();

/*
 * printInColor: Prints a string in the desired color using cross-platform ANSI codes.
 *
 * @text: string of text to be printed
 * @color: string of the chosen color
 */
void printInColor(const std::string& text, const std::string& color) {
    // Define ANSI escape color codes
    static std::unordered_map<std::string, std::string> colorMap = {
        {"black", "\033[30m"},
        {"red", "\033[31m"},
        {"green", "\033[32m"},
        {"yellow", "\033[33m"},
        {"blue", "\033[34m"},
        {"magenta", "\033[35m"},
        {"cyan", "\033[36m"},
        {"white", "\033[37m"},
        {"bright_black", "\033[90m"},
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
    static std::unordered_map<std::string, WORD> winColorMap = {
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

/*
 * printTitle: Prints the ASCII art of the commandline's title from the .txt file.
 */
void printTitle()
{
    // open file
    std::ifstream file("..\\WindowPain\\TitleASCII.txt");

    // check if file exists
    if (!file) {
        std::cerr << "Error: Could not open the file." << std::endl;
    }

    // print file
    std::string line;
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

/*
 * help: Prints the available commands to the user.
 */
void help() {
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

void initialize() {
    std::cout << "'initialize' command recognized. Doing something." << std::endl;
}

/*
 * screen: Prints the screen commands to the user.
 */
void screen() {
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

void schedulerTest() {
    std::cout << "'scheduler-test' command recognized. Doing something." << std::endl;
}

void schedulerStop() {
    std::cout << "'scheduler-stop' command recognized. Doing something." << std::endl;
}

void reportUtil() {
    std::cout << "'report-util' command recognized. Doing something." << std::endl;
}

/*
 * clearScreen: Clears the screen and re-prints the title.
 */
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    if (currentScreen.empty()) {
        printTitle();
    }
    else {
        screenRestore(currentScreen);
    }
}

/*
 * exitProgram: Exits the program.
 */
void exitProgram() {
    printInColor("Toodles!", "yellow");
	std::cout << "\n";
    exit(0);
}

/*
 * screenList: Lists all the active screens.
 */
void screenList() {
    if (screens.empty()) {
        printInColor("No active screens.\n", "red");
        return;
    }

    std::cout << "Active screens:\n";
    for (const auto& screen : screens) {
        // Mark the currently active screen (if any)
        if (screen.first == currentScreen) {
            std::cout << screen.first << " (attached)\n";
        } else {
            std::cout << screen.first << "\n";
        }
    }
}

/*
 * screenCreate: Creates a new screen with placeholder information.
 * 
 * @name: name of the screen
 */
void screenCreate(const std::string& name) {
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
    strftime(timestamp, sizeof(timestamp), "%m/%d/%Y, %I:%M:%S %p", &ltm);

    // Create a new screen
    Screen newScreen;
    newScreen.processName = name;
    newScreen.currentLine = 0;
    newScreen.totalLines = 100; // Placeholder value
    newScreen.timestamp = timestamp;

    // Add to map and update current screen
    screens[name] = newScreen;
    currentScreen = name;
    
    // Render this screen
    screenRestore(name);
}

/*
 * screenRestore: Restores an existing screen by its name.
 * 
 * @name: name of the screen
 */
void screenRestore(const std::string& name) {
    auto it = screens.find(name);
    if (it == screens.end()) {
        printInColor("No screen found with this name.\n", "red");
        return;
    }

    // Set current screen
    currentScreen = name;

    // Clear the console before displaying screen information
    system("cls");

    // Fetch the screen and display its information
    Screen& screen = it->second;

    std::cout << "Screen Name: " << screen.processName << "\n";
    std::cout << "Current Line: " << screen.currentLine << " / " << screen.totalLines << "\n";
    std::cout << "Timestamp: " << screen.timestamp << "\n";
}

/*
 * handleScreenCommands: Parses and handles screen commands
 * 
 * @input: command input string
 */
void handleScreenCommands(const std::string& input) {
    // Parse command
    if (input.find("screen -s ") == 0) {
        std::string screenName = input.substr(10); // Get screen name
        screenCreate(screenName);
    } else if (input.find("screen -r ") == 0) {
        std::string screenName = input.substr(10); // Get screen name
        screenRestore(screenName);
    } else if (input == "screen -ls") {
        // List all screens
        screenList();
    } else {
        printInColor("Unknown command. Type 'help' for available commands.\n", "red");
    }
}

/*
 * readCommand: Checks whether the input is a recognized command.
 * 
 * @command: string input from user
 */
void readCommand(const std::string& command) {
    // Exit command for screen and main menu prompts
    if (command == "exit") {
        if (!currentScreen.empty()) {
            // Set currentScreen to empty to exit the current screen and go back to main menu
            currentScreen.clear();
            clearScreen();
        } else {
            // Exit if its in main menu
            exitProgram();
        }
        return; // Return just in case it didn't exit as expected
    }

    // Screen command checker
	if (command.find("screen -s ") == 0 || command.find("screen -r ") == 0 || command == "screen -ls") {
        handleScreenCommands(command);
        return;
    }

    // Unordered map to pair command with its corresponding function
    std::unordered_map<std::string, std::function<void()>> commandMap = {
        {"help", help},
		{"initialize", initialize},
		{"screen", screen},
        {"scheduler-test", schedulerTest},
        {"scheduler-stop", schedulerStop},
        {"report-util", reportUtil},
        {"clear", clearScreen}
        // "exit" is now handled above
    };

    // Check if command exists
    auto it = commandMap.find(command);
    if (it != commandMap.end()) {
        // Call the corresponding function
        it->second();
    }
    else {
        printInColor("Unknown command. Type 'help' for available commands.\n", "red");
    }
}

/*
 * showHelp: Prints the available commands to the user.
 */
void showHelp() {
	std::cout << "Available commands: ";
	printInColor("initialize", "green");
	std::cout << ", ";
	printInColor("screen", "green");
	std::cout << ", ";
	printInColor("scheduler-test", "green");
	std::cout << ", ";
	printInColor("scheduler-stop", "green");
	std::cout << ", ";
	printInColor("report-util", "green");
	std::cout << ", ";
	printInColor("clear", "green");
	std::cout << ", ";
	printInColor("exit", "green");
	std::cout << ".\n";
}

/*
 * commandLoop: Prompts the user to enter a command until exited.
 */
void commandLoop() {
    std::string input;

    // Command loop
    while (true) {
        if (currentScreen.empty()) {
            // Main Menu prompt
            printInColor("Enter a command: ", "cyan");
        } else {
            // screen-specific prompt
            printInColor("[" + currentScreen + "]$ ", "cyan");
        }

        std::getline(std::cin, input);  // Get input from user

        // Acknowledge command
        readCommand(input);
    }
}


int main()
{
    printTitle();

    commandLoop();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
