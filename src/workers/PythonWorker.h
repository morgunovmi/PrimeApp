#ifndef PRIME_APP_PYTHONWORKER_H
#define PRIME_APP_PYTHONWORKER_H

#include <pybind11/embed.h>
#include <thread>

#include "Worker.h"
#include "messages/MessageQueue.h"
#include "messages/messages.h"

namespace py = pybind11;
using namespace py::literals;

namespace prm
{
    class PythonWorker : public Worker<PythonWorkerMessage>
    {
    public:
        PythonWorker(int id, MessageQueue<PythonWorkerMessage>& queue)
            : Worker(id, queue)
        {
        }

    private:
        [[noreturn]] void Main() override;

        void HandleMessage(PythonWorkerRunString&& runString);

        void HandleMessage(PythonWorkerQuit&& quit);
    };
}// namespace prm

#endif//PRIME_APP_PYTHONWORKER_H