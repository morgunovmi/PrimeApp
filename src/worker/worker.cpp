#include "worker.h"

void Worker::Main() {
    auto visitor = [&](auto&& msg) { HandleMessage(std::move(msg)); };
    mRunning = true;

    while (mRunning) {
        std::visit(visitor, mWorkerMessageQueue.WaitForMessage());
    }
}

void Worker::HandleMessage(WorkerLiveCapture&& liveCapture) {
    auto result = mBackend.LiveCapture();
}

void Worker::HandleMessage(WorkerQuit&& quit) {
    mBackend.TerminateCapture();
    mRunning = false;
}