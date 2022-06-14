#pragma once

#include <chrono>

/**
 * Basic timer class for code profiling
 */
class Timer {
public:
    using clock_t = std::chrono::steady_clock;
    using timepoint_t = clock_t::time_point;

    Timer();

    /**
     * Pauses the currently running timer
     */
    void pause();

    /**
     * Resumes the timer if it's paused
     */
    void resume();

    /**
     * Stops the timer and gives the total time
     * @return The total elapsed time in seconds
     */
    double stop();

private:
    /// Last timer resume time point
    timepoint_t last_resume;
    /// Current elapsed time
    std::chrono::steady_clock::duration inter_duration;
    /// Flag that shows if the timer is running
    bool is_counting;
};