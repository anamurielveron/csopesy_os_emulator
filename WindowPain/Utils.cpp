#include "Utils.h"
#include <unordered_map>
#include <iostream>

#ifdef _WIN32
#include <Windows.h> // For Windows
#else
#include <unistd.h> // For Linux, macOS
#endif

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
