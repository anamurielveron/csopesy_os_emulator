#ifndef SCREEN_H
#define SCREEN_H

#include "Utils.h"
#include <string>

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
    State state;

public:
    String name;        // process name saved by user
    int currentLine;    // current line of instruction
    int totalLines;     // total lines of instruction
    String timestamp;   // timestamp of when screen was created
    int coreId;         // The core assigned to this process

    Screen();
    Screen(const String& name, int totalLines);

    void setNewState() { state = State::New; };
    void setReadyState() { state = State::Ready; };
    void setWaitingState() { state = State::Waiting; };
    void setRunningState() { state = State::Running; };
    void setFinishedState() { state = State::Finished; };

    State getState() const { return state; };
    String getStateString() const {
        switch (state) {
        case State::New:
            return "New";
        case State::Ready:
            return "Ready";
        case State::Waiting:
            return "Waiting";
        case State::Running:
            return "Running";
        case State::Finished:
            return "Finished";
        default:
            return "Unknown";
        }
    };
};

#endif // SCREEN_H