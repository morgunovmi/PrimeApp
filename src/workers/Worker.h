#pragma once

#include <thread>
#include <variant>

#include "messages/MessageQueue.h"

namespace prm
{
    /**
     * Generic class for a worker that does work in a separate thread
     * @tparam MessageType Message type for communication with the worker through message queues
     */
    template<typename MessageType>
    class Worker
    {
    public:
        Worker(int id, MessageQueue<MessageType>& queue)
            : m_id(id), m_isRunning(false), m_messageQueue(queue)
        {
        }

        /**
         * Launches the worker thread with the Main function
         */
        virtual void Run() { m_thread = std::jthread(&Worker::Main, this); }

        virtual ~Worker() = default;

        /// Worker id
        const int m_id;

    protected:
        /**
         * Main worker function that runs commands sent to the worker through message queues
         */
        virtual void Main() {}

        /// Specifies if the worker is currently running
        bool m_isRunning;

        /// Worker thread where it does all the jobs
        std::jthread m_thread;

        /// Reference to the message queue for communication with the main thread
        MessageQueue<MessageType>& m_messageQueue;
    };
}// namespace prm