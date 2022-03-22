#include <vector>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <fmt/format.h>

#include "misc/Log.h"
#include "misc/LogSink.h"
#include "frontend/App.h"

int main(int argc, char **argv) {
    try {
        slr::Log log;

        auto sink = std::make_shared<LogSinkMt>(log);

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[Console sink] [%^%l%$] %v");

        std::vector<spdlog::sink_ptr> sinks{sink, console_sink};
        auto MyLogger = std::make_shared<spdlog::logger>("MyLogger", sinks.begin(), sinks.end());

#ifndef NDEBUG
        MyLogger->set_level(spdlog::level::trace);
#else
        MyLogger->set_level(spdlog::level::info);
#endif
        spdlog::set_default_logger(MyLogger);

        const auto width = 1400;
        const auto height = 600;
        const auto antialiasingLevel = 8;

        sf::ContextSettings settings{};
        settings.antialiasingLevel = antialiasingLevel;

        slr::App system{argc, argv, width, height, settings, log};
        system.Run();
    } catch (std::exception &e) {
        fmt::print("Error {}", e.what());
    }

    return EXIT_SUCCESS;
}