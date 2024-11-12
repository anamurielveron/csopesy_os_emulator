#ifndef SCREEN_H
#define SCREEN_H

#include "Utils.h"
#include <string>

class Screen {
public:
    String name;        // process name saved by user
    int currentLine;    // current line of instruction
    int totalLines;     // total lines of instruction
    String timestamp;   // timestamp of when screen was created
    int coreId;         // the core assigned to this process
    int startFrame;     // starting frame for memory allocated
    bool finished;      // Added flag to indicate if process is finished

    Screen();
    Screen(const String& name, int totalLines);
};

#endif // SCREEN_H