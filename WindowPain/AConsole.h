#ifndef ACONSOLE_H
#define ACONSOLE_H

#include "Utils.h"
#include <unordered_map>
#include <functional>
#include <string>

// Abstract Console
class AConsole {
protected:
    std::unordered_map<String, std::function<void()>> commandMap;       // list of commands
    std::unordered_map<String, std::function<void(const String&)>> commandMapWithArgs;  // list of commands w/arguments
public:
    virtual ~AConsole() = default;  // destructor
    virtual void draw() = 0;        // virtual method for drawing the console
    void handleCommand(const String& command); // run command
};

#endif // ACONSOLE_H