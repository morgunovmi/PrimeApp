#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

#include <spdlog/spdlog.h>

namespace prm
{
    template<typename T>
    class MessageQueue
    {
    public:
        void Send(T&& msg);

        [[nodiscard]] T WaitForMessage();

        int Clear();

        [[nodiscard]] bool Empty();

        [[nodiscard]] auto Size();

    private:
        std::queue<T> m_queue;
        std::condition_variable m_condVar;
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

#endif