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
}

void PythonWorker::HandleMessage(PythonWorkerRunString&& runString) {
    spdlog::debug(runString.debugString);

    for (const auto & val : runString.strVariables) {
        py::globals()[val.first.c_str()] = val.second;
    }

    for (const auto & val : runString.numVariables) {
        py::globals()[val.first.c_str()] = val.second;
    }

    try {
        py::exec(runString.string);
    } catch (const py::error_already_set& e) {
        spdlog::error("Error running python string {}\n\nError: {}", runString.string, e.what());
    }
}

void PythonWorker::HandleMessage(PythonWorkerQuit &&quit) {
    spdlog::debug(quit.debugString);
    mRunning = false;
}