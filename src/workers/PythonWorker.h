#pragma once

#include <pybind11/embed.h>
#include <thread>

#include "Worker.h"
#include "messages/MessageQueue.h"
#include "messages/messages.h"

namespace py = pybind11;
using namespace py::literals;

namespace prm
{
    /**
     * Worker class that runs python commands in the embedded interpreter in a separate thread
     */
    class PythonWorker : public Worker<PythonWorkerMessage>
    {
    public:
        PythonWorker(int id, MessageQueue<PythonWorkerMessage>& queue,
                     sf::Texture& texture, std::mutex& mutex)
            : Worker(id, queue), m_currentTexture(texture),
              m_textureMutex(mutex)
        {
        }

    private:
        /**
         * Main function that runs the commands sent to the worker through message queues
         */
        [[noreturn]] void Main() override;

        /**
         * Handles the message with a python string in it
         *
         * @param runString Message with info about the python query
         */
        void HandleMessage(PythonWorkerRunString&& runString);

        /**
         * Handles the WorkerQuit message (kills the worker)
         *
         * @param quit Empty quit struct
         */
        void HandleMessage(PythonWorkerQuit&& quit);

        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;
    };
}// namespace prm