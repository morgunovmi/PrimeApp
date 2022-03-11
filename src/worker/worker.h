#ifndef WORKER_H
#define WORKER_H

#include <mutex>
#include <thread>
#include <vector>

#include "messages/MessageQueue.h"
#include "messages/messages.h"
#include "backend/PhotometricsBackend.h"

class Worker {
public:
    Worker(slr::Backend& backend, int id, MessageQueue<WorkerMessage>& workerMessageQueue, MessageQueue<SupervisorMessage>& supervisorMessageQueue) :
        mBackend(backend), mMutex(), mId(id), mRunning(false), mWorkerMessageQueue(workerMessageQueue), mSupervisorMessageQueue(supervisorMessageQueue) {}

    void Run();

private:
    void Main();

    void HandleMessage(WorkerLiveCapture&& liveCapture);
    void HandleMessage(WorkerQuit&& quit);

private:
    slr::Backend& mBackend;

    std::mutex mMutex;

    const int mId;
    bool mRunning;

    std::jthread mThread;

    MessageQueue<WorkerMessage>& mWorkerMessageQueue;
    MessageQueue<SupervisorMessage>& mSupervisorMessageQueue;
};

#endif