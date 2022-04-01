#ifndef MESSAGES_H
#define MESSAGES_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <variant>
#include <string>

#include "imgui.h"

// PythonWorker

const uint16_t MAX_DEBUG_STR_LEN = 256;

struct PythonWorkerRunString {
#ifndef NDEBUG
    const char debugString[MAX_DEBUG_STR_LEN] = "Run python string message";
#endif

    std::string string{};
    std::vector<std::pair<std::string, std::string>> strVariables{};
    std::vector<std::pair<std::string, int>> intVariables{};
    std::vector<std::pair<std::string, double>> floatVariables{};
};

struct PythonWorkerQuit {
#ifndef NDEBUG
    const char debugString[MAX_DEBUG_STR_LEN] = "Python worker quit message";
#endif
};

using PythonWorkerMessage = std::variant<
        PythonWorkerRunString,
        PythonWorkerQuit
        >;

#endif