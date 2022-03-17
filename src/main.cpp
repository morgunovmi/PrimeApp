#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <pybind11/embed.h>

#include "misc/Log.h"
#include <fmt/format.h>
#include "misc/LogSink.h"
#include "frontend/App.h"

namespace py = pybind11;

int main(int argc, char** argv) {
    slr::Log log;

    auto sink = std::make_shared<LogSinkMt>(log);
    auto MyLogger = std::make_shared<spdlog::logger>("MyLogger", sink);

#ifndef NDEBUG
    MyLogger->set_level(spdlog::level::trace);
#else
    MyLogger->set_level(spdlog::level::off);
#endif

    spdlog::set_default_logger(MyLogger);

    {
        py::scoped_interpreter guard{};  // start the interpreter and keep it alive

        spdlog::info("Hello, World! (From Python)");

        spdlog::info("Resutlt from python : {}",
                   py::module::import("math").attr("sqrt")(2).cast<double>());
    }

    const auto width = 1400;
    const auto height = 600;

    sf::ContextSettings settings{};
    settings.antialiasingLevel = 8;

    slr::App system{ argc, argv, width, height, settings, log };
    system.Run();

    return EXIT_SUCCESS;
}