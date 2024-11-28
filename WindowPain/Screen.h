#ifndef SCREEN_H
#define SCREEN_H

#include "Utils.h"
#include <string>
#include <stdexcept>
#include <iostream>

class Screen {
public:
    enum class State {
        New,
        Ready,
        Waiting,
        Running,
        Finished
    };

private:
    State state;       // Current state of the screen
    String name;       // Process name
    int currentLine;   // Current line being executed
    int totalLines;    // Total lines of instruction
    String timestamp;  // Timestamp of process creation
    int coreId;        // Core ID assigned to this process (-1 if unassigned)

public:
    Screen()
        : state(State::New), name("Unnamed"), currentLine(0), totalLines(0), coreId(-1) {}

    Screen(const String& name, int totalLines)
        : state(State::New), name(name), currentLine(0), totalLines(totalLines), coreId(-1) {}



    // Getters
    State getState() const { return state; }
    String getStateString() const {
        switch (state) {
        case State::New: return "New";
        case State::Ready: return "Ready";
        case State::Waiting: return "Waiting";
        case State::Running: return "Running";
        case State::Finished: return "Finished";
        default: return "Unknown";
        }
    }

        String getName() const { return name; }
        int getCurrentLine() const { return currentLine; }
        int getTotalLines() const { return totalLines; }
        String getTimestamp() const { return timestamp; }
        int getCoreId() const { return coreId; }


        //Setters
        void setName(const String& newName) { name = newName; }
        void setCoreId(int id) { coreId = id; }
        void setCurrentLine(int line) { currentLine = line; }
        void setTotalLines(int lines) { totalLines = lines; }
        void setTimestamp(const String& newTimestamp) { timestamp = newTimestamp; }

        void setNewState() { state = State::New; }
        void setReadyState() { state = State::Ready; }
        void setWaitingState() { state = State::Waiting; }
        void setRunningState() { state = State::Running; }
        void setFinishedState() { state = State::Finished; }
};

#endif // SCREEN_H
