#include "Utils.h"
#include "ConsoleManager.h"
#include "AConsole.h"
#include "ScreenManager.h"
#include "MainMenuConsole.h"
#include "Scheduler.h"
#include "ScreenConsole.h"
#include "Screen.h"

ConsoleManager::ConsoleManager()
    : screenManager(std::make_unique<ScreenManager>(*this)), currentConsoleType(ConsoleType::MainMenu) {}

ScreenManager& ConsoleManager::getScreenManager() { return *screenManager; }

ConsoleType ConsoleManager::getCurrentConsoleType() { return currentConsoleType; }

void ConsoleManager::switchConsole(ConsoleType consoleType) {
    switch (consoleType) {
    case ConsoleType::MainMenu:
        currentConsoleType = ConsoleType::MainMenu;
        currentConsole = std::make_unique<MainMenuConsole>(*screenManager, *this);
        break;
    case ConsoleType::Screen:
        currentConsoleType = ConsoleType::Screen;
        currentConsole = std::make_unique<ScreenConsole>(*screenManager, *this);
        break;
    }
    currentConsole->draw();
}

void ConsoleManager::passCommand(const String& command) {
    currentConsole->handleCommand(command);
}