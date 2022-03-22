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

    void Test() {
        mMessageQueue.Send(PythonWorkerVideoInit{.path = R"(C:\Users\Max\Desktop\Samples\)", .file = R"(a1.tif)"});
    }

    ~VideoProcessor() { mMessageQueue.Send(PythonWorkerQuit{}); }

private:
    void Init() { mMessageQueue.Send(PythonWorkerEnvInit{}); }

    MessageQueue<PythonWorkerMessage> mMessageQueue;

    PythonWorker mPythonWorker;
};

#endif //PRIME_APP_VIDEOPROCESSOR_H
