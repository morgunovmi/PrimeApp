#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "misc/Log.h"
#include "misc/LogSink.h"
#include "frontend/App.h"

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

    const auto width = 1400;
    const auto height = 600;

    sf::ContextSettings settings{};
    settings.antialiasingLevel = 8;

    slr::App system{ argc, argv, width, height, settings, log };
    system.Run();

    return EXIT_SUCCESS;
}