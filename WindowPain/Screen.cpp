#include "Screen.h"

Screen::Screen() : name("Untitled"), currentLine(0), totalLines(-1), coreId(-1), finished(false) {}

Screen::Screen(const std::string& name, int totalLines)
    : name(name), currentLine(0), totalLines(totalLines), coreId(-1), finished(false) {}
