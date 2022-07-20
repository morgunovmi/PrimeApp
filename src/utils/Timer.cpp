#include "Timer.h"

Timer::Timer()
    : last_resume(clock_t::now()), inter_duration(0), is_counting(true)
{
}

void Timer::pause()
{
    if (is_counting)
    {
        inter_duration += (clock_t::now() - last_resume);

        is_counting = false;
    }
}

void Timer::resume()
{
    if (!is_counting)
    {
        last_resume = clock_t::now();
        is_counting = true;
    }
}

double Timer::stop()
{
    if (is_counting) inter_duration += (clock_t::now() - last_resume);

    auto duration = std::chrono::duration<double>(inter_duration).count();

    return duration;
}