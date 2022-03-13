#ifndef PRIME_APP_LOG_SINK_H
#define PRIME_APP_LOG_SINK_H

#include "spdlog/sinks/base_sink.h"
#include "misc/Log.h"

#include <mutex>

template <typename Mutex>
class LogSink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit LogSink(slr::Log& log) : spdlog::sinks::base_sink<Mutex>(), mLog(log) {}

private:
    slr::Log& mLog;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
        // msg.raw contains pre formatted log

        // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        mLog.AddLog(fmt::to_string(formatted).c_str());
    }

    void flush_() override {

    }
};

using LogSinkMt = LogSink<std::mutex>;

#endif // PRIME_APP_LOG_SINK_H
