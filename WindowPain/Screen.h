#ifndef SCREEN_H
#define SCREEN_H

#include "Utils.h"
#include <string>

enum class ScreenState { Ready, Running, Waiting, Finished };

class Screen {
public:
    String name;        // process name saved by user
    int currentLine;    // current line of instruction
    int totalLines;     // total lines of instruction
    String timestamp;   // timestamp of when screen was created
    int coreId;         // The core assigned to this process
    bool finished;      // Added flag to indicate if process is finished
    ScreenState state;     // The current state of the screen

    Screen();
    Screen(const String& name, int totalLines);

    void setState(ScreenState newState);
    ScreenState getState() const;
    String getStateString() const;
};

#endif // SCREEN_H