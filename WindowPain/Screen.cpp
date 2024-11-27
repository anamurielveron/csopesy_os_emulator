#include "Utils.h"
#include "Screen.h"

Screen::Screen()
    : name("Untitled"), currentLine(0), totalLines(-1), coreId(-1), state(State::New) {}

Screen::Screen(const String& name, int totalLines)
    : name(name), currentLine(0), totalLines(totalLines), coreId(-1), state(State::New) {}
