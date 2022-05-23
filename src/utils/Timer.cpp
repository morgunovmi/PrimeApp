#include <spdlog/spdlog.h>

#include "Timer.h"

Timer::Timer()
    : last_resume(clock_t::now()), inter_duration(0), is_counting(true)
{
    spdlog::info("Timer started");
}

void Timer::pause()
{
    if (is_counting)
    {
        inter_duration += (clock_t::now() - last_resume);

        auto duration = std::chrono::duration<double>(inter_duration).count();
        spdlog::info("Timer stopped\nCurrent time: {}", duration);

        is_counting = false;
    }
}

void Timer::resume()
{
    if (!is_counting)
    {
        last_resume = clock_t::now();
        spdlog::info("Timer resumed");
        is_counting = true;
    }
}

double Timer::stop()
{
    if (is_counting) inter_duration += (clock_t::now() - last_resume);

    auto duration = std::chrono::duration<double>(inter_duration).count();
    spdlog::info("Timer stopped\nTotal time: {} seconds", duration);

    return duration;
}