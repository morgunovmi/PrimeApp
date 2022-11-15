#include <fmt/format.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

#include "frontend/App.h"
#include "misc/Log.h"
#include "misc/LogSink.h"

int main(int argc, char** argv)
{
    try
    {
        prm::Log log;

        auto sink = std::make_shared<LogSinkMt>(log);

        auto console_sink =
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                "logs/app_log.txt");

        std::vector<spdlog::sink_ptr> sinks{sink, console_sink, file_sink};
        auto MyLogger = std::make_shared<spdlog::logger>(
                "MyLogger", sinks.begin(), sinks.end());
#ifndef NDEBUG                
        MyLogger->set_pattern(">> [%T] {%t} (%^%l%$) %v <<");
#else
        MyLogger->set_pattern(">> [%T] (%^%l%$) %v <<");
#endif

#ifndef NDEBUG
        MyLogger->set_level(spdlog::level::trace);
#else
        MyLogger->set_level(spdlog::level::info);
#endif
        spdlog::set_default_logger(MyLogger);

        const auto width = 1600;
        const auto height = 800;
        const auto antialiasingLevel = 8;

        sf::ContextSettings settings{};
        settings.antialiasingLevel = antialiasingLevel;

        prm::App app{argc, argv, width, height, settings, log};
        app.Run();
    }
    catch (std::exception& e)
    {
        fmt::print("Error {}", e.what());
    }

    return EXIT_SUCCESS;
}