#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <variant>
#include <vector>

#include <imgui.h>

/**
 * This file specifies the messages that can be sent to the running PythonWorker through a MessageQueue
 */

namespace prm
{
    // PythonWorker

    /// Max length of the debug string in the messages
    const uint16_t MAX_DEBUG_STR_LEN = 256;

    /// Message with info for a python call
    struct PythonWorkerRunString
    {
#ifndef NDEBUG
        const char debugString[MAX_DEBUG_STR_LEN] = "Run python string message";
#endif

        std::string string{}; ///< Python string to call
        std::vector<std::pair<std::string, std::string>> strVariables{}; ///< String variables to send to pybind
        std::vector<std::pair<std::string, int>> intVariables{}; ///< Integer variables to send to pybind
        std::vector<std::pair<std::string, double>> floatVariables{}; ///< Floating point variables to send to pybind
    };

    /// Message to quit for the python worker
    struct PythonWorkerQuit
    {
#ifndef NDEBUG
        const char debugString[MAX_DEBUG_STR_LEN] =
                "Python worker quit message";
#endif
    };

    /// Variant that groups all the possible python worker messages
    using PythonWorkerMessage =
            std::variant<PythonWorkerRunString, PythonWorkerQuit>;
}// namespace prm