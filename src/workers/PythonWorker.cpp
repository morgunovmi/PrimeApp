#include <spdlog/spdlog.h>

#include "PythonWorker.h"

namespace prm
{
    [[noreturn]] void PythonWorker::Main()
    {
        spdlog::debug("Started python worker main func");
        py::scoped_interpreter mGuard{};

        auto visitor = [&](auto&& msg)
        { HandleMessage(std::forward<decltype(msg)>(msg)); };
        m_isRunning = true;

        while (m_isRunning)
        {
            std::visit(visitor, m_messageQueue.WaitForMessage());
        }

        spdlog::debug("Python worker main func end");
    }

    void PythonWorker::HandleMessage(PythonWorkerRunString&& runString)
    {
        const uint16_t stringPeekLen = 20;

#ifndef NDEBUG
        spdlog::debug(runString.debugString);
#endif

        spdlog::debug("Attempting to run python string {}...",
                      runString.string.substr(0, stringPeekLen));

        try
        {
            for (const auto& val: runString.strVariables)
            {
                spdlog::debug("{}: {}", val.first, val.second);
                py::globals()[val.first.c_str()] = val.second;
            }

            for (const auto& val: runString.intVariables)
            {
                spdlog::debug("{}: {}", val.first, val.second);
                py::globals()[val.first.c_str()] = val.second;
            }

            for (const auto& val: runString.floatVariables)
            {
                spdlog::debug("{}: {}", val.first, val.second);
                py::globals()[val.first.c_str()] = val.second;
            }

            py::exec(runString.string);
            spdlog::info("String {}... ran successfully!",
                         runString.string.substr(0, stringPeekLen));
        }
        catch (const py::error_already_set& e)
        {
            spdlog::error("Error running python string {}\n\nError: {}",
                          runString.string, e.what());
        }

        try
        {
            auto res_string = py::globals()["result"].cast<std::string>();
            if (!res_string.empty()) { spdlog::info("Result: {}", res_string); }
        }
        catch (const std::exception& e)
        {
        }

        try
        {
            auto plot = py::globals()["plot"].cast<bool>();
            if (plot)
            {
                std::scoped_lock lock{m_textureMutex};
                if (!m_currentTexture.loadFromFile("plot.png"))
                {
                    spdlog::error("Couldn't load plot file");
                }
            }
        }
        catch (const std::exception& e)
        {
        }
    }

    void PythonWorker::HandleMessage(PythonWorkerQuit&& quit)
    {
#ifndef NDEBUG
        spdlog::debug(quit.debugString);
#endif

        m_isRunning = false;
    }
}// namespace prm