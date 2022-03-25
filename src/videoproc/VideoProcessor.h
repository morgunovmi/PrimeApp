#ifndef PRIME_APP_VIDEOPROCESSOR_H
#define PRIME_APP_VIDEOPROCESSOR_H

#include <pybind11/embed.h>

#include "workers/PythonWorker.h"
#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace py::literals;

class VideoProcessor {
public:
    VideoProcessor() : mMessageQueue(), mPythonWorker(1, mMessageQueue) {
        mPythonWorker.Run();
        Init();
    }

    void Test(const std::string& path, const std::string& file);

    ~VideoProcessor() { spdlog::info("Killing video processor"); mMessageQueue.Send(PythonWorkerQuit{}); }

private:
    void Init();

    MessageQueue<PythonWorkerMessage> mMessageQueue;

    PythonWorker mPythonWorker;
};

#endif //PRIME_APP_VIDEOPROCESSOR_H
