#ifndef SCREEN_H
#define SCREEN_H

#include <string>

class Screen {
public:
    std::string name;
    int currentLine;
    int totalLines;
    std::string timestamp;
    int coreId;
    bool finished;

    Screen();
    Screen(const std::string& name, int totalLines);
};

#endif // SCREEN_H
