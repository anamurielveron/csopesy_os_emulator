// WindowPain.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <unordered_map>

/*
 * setConsoleColor: changes console text to a specified color
 * 
 * @color: chosen text color
 */
void setConsoleColor(WORD color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

/* 
 * printInColor: Prints a string in the desired color
 * 
 * @text: string of text to be printed
 * @color: string of the chosen color
 */
void printInColor(const std::string& text, const std::string& color) {
    // define color codes
    static std::unordered_map<std::string, WORD> colorMap = {
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

    // save default console color
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    WORD defaultColor = consoleInfo.wAttributes;

    // set chosen color
    auto it = colorMap.find(color);
    if (it != colorMap.end()) {
        setConsoleColor(it->second);
    }
    else {
        printInColor("Error: Unknown color name.", "red");
        std::cout << "\n";
        return;
    }

    // print the text
    std::cout << text;

    // reset to default color
    setConsoleColor(defaultColor);
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
 * readCommand: Checks whether the input is a recognized command.
 * 
 * @command: string input from user
 */
void readCommand(const std::string& command) {
    // unordered map to pair command with acknowledgment messages
    std::unordered_map<std::string, std::string> commandMap = {
        {"help", ""},
        {"initialize", "'initialize' command recognized. Doing something.\n"},
        {"screen", "'screen' command recognized. Doing something.\n"},
        {"scheduler-test", "'scheduler-test' command recognized. Doing something.\n"},
        {"scheduler-stop", "'scheduler-stop' command recognized. Doing something.\n"},
        {"report-util", "'report-util' command recognized. Doing something.\n"},
        {"clear", "'clear' command recognized. Doing something.\n"},
        {"exit", ""}
    };

    // check if command exists
    auto it = commandMap.find(command);
    if (it != commandMap.end()) {
        // print corresponding message
        std::cout << it->second;
    }
    else {
        printInColor("Unknown command. Type 'help' for available commands.", "red");
        std::cout << "\n";
    }
}

/*
 * commandLoop: Prompts the user to enter a command until exited.
 */
void commandLoop() {
    std::string input;

    // command loop
    while (true) {
        // std::cout << "Enter a command: ";
        printInColor("Enter a command: ", "cyan");
        std::getline(std::cin, input);  // get the input from user

        // acknowledge command
        readCommand(input);

        
        if (input == "exit") {
            printInColor("Toodles!", "yellow");
            break;
        }
        else if (input == "clear") { // could be made into a separate function lol
            system("cls");
            printTitle();
        }
        else if (input == "help") { // could also be made into a separate function
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
