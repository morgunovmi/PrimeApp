#include "CameraWorker.h"

[[noreturn]] void CameraWorker::Main() {
    spdlog::debug("Started Camera worker main func");

    auto visitor = [&](auto &&msg) { HandleMessage(std::forward<decltype(msg)>(msg)); };
    mRunning = true;

    while (mRunning) {
        std::visit(visitor, mWorkerMessageQueue.WaitForMessage());
    }

    spdlog::debug("Camera worker main func end");
}

void CameraWorker::HandleMessage(CameraWorkerInit &&init) {}

void CameraWorker::HandleMessage(CameraWorkerLiveCapture &&liveCapture) {}

void CameraWorker::HandleMessage(CameraWorkerSequenceCapture &&sequenceCapture) {}