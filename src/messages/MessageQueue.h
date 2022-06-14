#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include <spdlog/spdlog.h>

namespace prm
{
    /**
     * Thread safe queue for communication between threads using generic messages
     *
     * @tparam T Message type of the queue (should be an std::variant of all available message types)
     */
    template<typename T>
    class MessageQueue
    {
    public:
        /**
         * Sends the given message to the queue
         *
         * @param msg Message to send
         */
        void Send(T&& msg);

        /**
         * Waits for a message to arrive
         *
         * @return The received message
         */
        [[nodiscard]] T WaitForMessage();

        /**
         * Clears the queue
         *
         * @return number of items cleared
         */
        int Clear();

        /**
         * Checks if the queue is empty
         *
         * @return true if the queue is empty
         */
        [[nodiscard]] bool Empty();

        /**
         * Gives the current number of items in the queue
         *
         * @return Number of items in the queue
         */
        [[nodiscard]] auto Size();

    private:
        /// Underlying queue
        std::queue<T> m_queue;
        /// Condition variable on which to wait
        std::condition_variable m_condVar;
        /// Mutex for queue synchronisation
        std::mutex m_mutex;
    };

    template<typename T>
    void MessageQueue<T>::Send(T&& msg)
    {
        {
            spdlog::debug("Sending message");
            std::scoped_lock lock(m_mutex);
            m_queue.emplace(std::move(msg));
        }

        m_condVar.notify_all();
    }

    template<typename T>
    [[nodiscard]] T MessageQueue<T>::WaitForMessage()
    {
        std::unique_lock lock(m_mutex);
        m_condVar.wait(lock, [&] { return !m_queue.empty(); });

        spdlog::debug("received message!");
        auto msg = m_queue.front();
        m_queue.pop();

        return msg;
    }

    template<typename T>
    int MessageQueue<T>::Clear()
    {
        std::scoped_lock lock(m_mutex);
        auto messages_removed = 0;

        while (!m_queue.empty())
        {
            m_queue.pop();
            ++messages_removed;
        }

        return messages_removed;
    }

    template<typename T>
    [[nodiscard]] bool MessageQueue<T>::Empty()
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

    template<typename T>
    [[nodiscard]] auto MessageQueue<T>::Size()
    {
        std::scoped_lock lock(m_mutex);
        return m_queue.size();
    }
}// namespace prm