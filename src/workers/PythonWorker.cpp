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

/*
void PythonWorker::HandleMessage(PythonWorkerVideoInit &&videoInit) {
    spdlog::debug("Video init func");

    py::globals()["name"] = "Jeff";
    py::globals()["path"] = videoInit.path;
    py::globals()["file"] = videoInit.file;
    py::exec(R"(

)");
}

void PythonWorker::HandleMessage(PythonWorkerLocateOne &&locateOne) {
    spdlog::debug("Locating one");

    try {
        py::exec(R"(
minm = 1e3 #1e2 = 100
ecc = 0.5
vid.minmass = minm
mass = minm
size = 5
diametr = 19

#Эта функция для подбора параметров на одном кадре. Изменяем параметры (в основном, mass),
#пока картинка не станет хорошей, и только тогда запускаем locate_all
vid.locate_1_frame(10, ecc, mass, size, diametr)
)");
    } catch (py::error_already_set &e) {
        spdlog::error("Error {}", e.what());
    }
}
 */

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