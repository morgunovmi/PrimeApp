#ifndef PRIME_APP_CAMERAWORKER_H
#define PRIME_APP_CAMERAWORKER_H

#include "Worker.h"
#include "messages/messages.h"

class CameraWorker : public Worker<CameraWorkerMessage> {
public:
    CameraWorker(int id, MessageQueue<CameraWorkerMessage> &queue, std::atomic<bool> &isCapturing) : Worker(id, queue),
                                                                                                     mIsCapturing(isCapturing) {}

private:
    [[noreturn]] void Main() override;

    std::atomic<bool> &mIsCapturing;

    void HandleMessage(CameraWorkerInit &&init);

    void HandleMessage(CameraWorkerLiveCapture &&liveCapture);

    void HandleMessage(CameraWorkerSequenceCapture &&sequenceCapture);
};

#endif //PRIME_APP_CAMERAWORKER_H
