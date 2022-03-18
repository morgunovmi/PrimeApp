#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "misc/Log.h"
#include <fmt/format.h>
#include "misc/LogSink.h"
#include "frontend/App.h"


int main(int argc, char **argv) {
    try {
        slr::Log log;

        auto sink = std::make_shared<LogSinkMt>(log);
        auto MyLogger = std::make_shared<spdlog::logger>("MyLogger", sink);

#ifndef NDEBUG
        MyLogger->set_level(spdlog::level::trace);
#else
        MyLogger->set_level(spdlog::level::off);
#endif
        spdlog::set_default_logger(MyLogger);

        const auto width = 1400;
        const auto height = 600;
        const auto antialiasingLevel = 8;

        sf::ContextSettings settings{};
        settings.antialiasingLevel = antialiasingLevel;

        slr::App system{argc, argv, width, height, settings, log};
        system.Run();
    } catch (std::exception& e) {
        fmt::print("Error {}", e.what());
    }

    return EXIT_SUCCESS;
}