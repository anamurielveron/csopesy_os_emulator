#include "AConsole.h"
#include "Utils.h"

#include <sstream>

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
                        printInColor("Unknown command. Type 'help' for available commands.\n\n", "red");
                    }
                }
            }
            else {
                printInColor("Unknown command. Type 'help' for available commands.\n\n", "red");
            }
        }
        else {
            printInColor("Unknown command. Type 'help' for available commands.\n\n", "red");
        }
    }
}
