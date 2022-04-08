#ifndef PRIME_APP_PYTHONWORKER_H
#define PRIME_APP_PYTHONWORKER_H

#include <thread>
#include "pybind11/embed.h"

#include "Worker.h"
#include "messages/messages.h"
#include "messages/MessageQueue.h"

namespace py = pybind11;
using namespace py::literals;

class PythonWorker : public Worker<PythonWorkerMessage> {
public:
    PythonWorker(int id, MessageQueue<PythonWorkerMessage> &queue) : Worker(id, queue) {}

private:
    [[noreturn]] void Main() override;

    void HandleMessage(PythonWorkerRunString&& runString);

    void HandleMessage(PythonWorkerQuit &&quit);
};

#endif //PRIME_APP_PYTHONWORKER_H