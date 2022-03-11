#ifndef PRIME_APP_OPENCVBACKEND_H
#define PRIME_APP_OPENCVBACKEND_H

#include "Backend.h"
#include <opencv2/opencv.hpp>

namespace slr {
class OpencvBackend : public Backend {
public:
    OpencvBackend(int argc, char **argv, sf::RenderWindow& window, sf::Clock& deltaClock, sf::Time& dt, sf::Texture& currentTexture, Log& appLog, std::mutex& mutex) :
        Backend(argc, argv, window, currentTexture, deltaClock, dt, appLog, mutex), mIsOpened(false) {}

    void Init() override;

    void LiveCapture() override;

    void TerminateCapture() override;

    ~OpencvBackend() override { OpencvBackend::TerminateCapture(); };

private:
    cv::VideoCapture mCamera;
    std::atomic<bool> mIsOpened;
};
}

#endif // PRIME_APP_OPENCVBACKEND_H
