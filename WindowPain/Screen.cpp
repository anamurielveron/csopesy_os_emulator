#include "Utils.h"
#include "Screen.h"

Screen::Screen() : name("Untitled"), currentLine(0), totalLines(-1), coreId(-1), state(ScreenState::Ready) {}

Screen::Screen(const String& name, int totalLines)
    : name(name), currentLine(0), totalLines(totalLines), coreId(-1), state(ScreenState::Ready) {}

void Screen::setState(ScreenState newState) {
    state = newState;
}

ScreenState Screen::getState() const {
    return state;
}

String Screen::getStateString() const {
    switch (state) {
    case ScreenState::Ready: return "Ready";
    case ScreenState::Running: return "Running";
    case ScreenState::Waiting: return "Waiting";
    case ScreenState::Finished: return "Finished";
    default: return "Unknown";
    }
}
