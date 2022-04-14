#ifndef PRIME_APP_WORKER_H
#define PRIME_APP_WORKER_H

#include <thread>
#include <variant>

#include "messages/MessageQueue.h"

template<typename MessageType>
class Worker {
public:
    Worker(int id, MessageQueue<MessageType> &queue) : mId(id), mRunning(false), mWorkerMessageQueue(queue) {}

    virtual void Run() {
        mThread = std::jthread(&Worker::Main, this);
    }

    virtual ~Worker() = default;

    const int mId;

protected:
    virtual void Main() {}

    bool mRunning;

    std::jthread mThread;

    MessageQueue<MessageType> &mWorkerMessageQueue;
};

#endif //PRIME_APP_WORKER_H
