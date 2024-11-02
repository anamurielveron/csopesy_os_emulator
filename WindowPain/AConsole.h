#ifndef ACONSOLE_H
#define ACONSOLE_H

#include <unordered_map>
#include <functional>
#include <string>

class AConsole {
protected:
    std::unordered_map<std::string, std::function<void()>> commandMap;
    std::unordered_map<std::string, std::function<void(const std::string&)>> commandMapWithArgs;
public:
    virtual ~AConsole() = default;
    virtual void draw() = 0;
    void handleCommand(const std::string& command);
};

#endif // ACONSOLE_H