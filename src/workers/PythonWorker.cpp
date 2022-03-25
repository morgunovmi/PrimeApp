#include "spdlog/spdlog.h"

#include "PythonWorker.h"

void PythonWorker::Run() {
    mThread = std::jthread(&PythonWorker::Main, this);
}

[[noreturn]] void PythonWorker::Main() {
    py::scoped_interpreter mGuard{};
    spdlog::debug("Started python worker main func");

    auto visitor = [&](auto &&msg) { HandleMessage(std::forward<decltype(msg)>(msg)); };
    mRunning = true;

    while (mRunning) {
        std::visit(visitor, mWorkerMessageQueue.WaitForMessage());
    }
    spdlog::debug("Python worker main func end");
}

void PythonWorker::HandleMessage(PythonWorkerRunString &&runString) {
    const uint16_t stringPeekLen = 20;

#ifndef NDEBUG
    spdlog::debug(runString.debugString);
#endif

    spdlog::debug("Attempting to run python string {}...", runString.string.substr(0, stringPeekLen));

    try {
        for (const auto &val: runString.strVariables) {
            spdlog::debug("{}: {}", val.first, val.second);
            py::globals()[val.first.c_str()] = val.second;
        }

        for (const auto &val: runString.numVariables) {
            spdlog::debug("{}: {}", val.first, val.second);
            py::globals()[val.first.c_str()] = val.second;
        }

        py::exec(runString.string);
        spdlog::debug("String {}... ran successfully!", runString.string.substr(0, stringPeekLen));
    } catch (const py::error_already_set &e) {
        spdlog::error("Error running python string {}\n\nError: {}", runString.string, e.what());
    }
}

void PythonWorker::HandleMessage(PythonWorkerQuit &&quit) {
#ifndef NDEBUG
    spdlog::debug(runString.debugString);
#endif

    mRunning = false;
}