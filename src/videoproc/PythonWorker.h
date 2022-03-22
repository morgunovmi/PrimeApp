#ifndef PRIME_APP_PYTHONWORKER_H
#define PRIME_APP_PYTHONWORKER_H

#include <thread>
#include <pybind11/embed.h>

#include "messages/messages.h"
#include "messages/MessageQueue.h"

namespace py = pybind11;
using namespace py::literals;

class PythonWorker {
public:
    PythonWorker(int id, MessageQueue<PythonWorkerMessage> &queue) :
            mId(id), mRunning(false), mQueue(queue) {}

    void Run();

private:
    [[noreturn]] void Main();

    void HandleMessage(PythonWorkerEnvInit&& envInit);
    void HandleMessage(PythonWorkerVideoInit&& videoInit);
    void HandleMessage(PythonWorkerQuit&& quit);

    const int mId;
    bool mRunning;

    std::jthread mWorkerThread;

    MessageQueue<PythonWorkerMessage> &mQueue;
};

#endif //PRIME_APP_PYTHONWORKER_H