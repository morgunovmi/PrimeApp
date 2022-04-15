#ifndef PRIME_APP_WORKER_H
#define PRIME_APP_WORKER_H

#include <thread>
#include <variant>

#include "messages/MessageQueue.h"

template<typename MessageType>
class Worker
{
public:
    Worker(int id, MessageQueue<MessageType>& queue)
        : m_id(id), m_isRunning(false), m_messageQueue(queue)
    {
    }

    virtual void Run() { m_thread = std::jthread(&Worker::Main, this); }

    virtual ~Worker() = default;

    const int m_id;

protected:
    virtual void Main() {}

    bool m_isRunning;

    std::jthread m_thread;

    MessageQueue<MessageType>& m_messageQueue;
};

#endif//PRIME_APP_WORKER_H
