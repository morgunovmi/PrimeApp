#ifndef PRIME_APP_OPENCVBACKEND_H
#define PRIME_APP_OPENCVBACKEND_H

#include <string_view>

#include <opencv2/opencv.hpp>

#include "Backend.h"

namespace slr {
constexpr std::string_view LIVE_CAPTURE_PREFIX{"LiveCapture_"};
constexpr std::string_view SEQ_CAPTURE_PREFIX{"SequenceCapture_"};

class OpencvBackend : public Backend {
public:
    OpencvBackend(int argc, char **argv, sf::RenderWindow& window, sf::Texture& currentTexture, sf::Time& dt,  std::mutex& mutex) :
        Backend(argc, argv, window, currentTexture, dt, mutex), mIsOpened(false), mFramerate(30) {}

    void Init() override;

    void LiveCapture() override;

    void SequenceCapture(uint32_t nFrames) override;

    void TerminateCapture() override;

    ~OpencvBackend() override { OpencvBackend::TerminateCapture(); };

private:
    static sf::Image MatToImage(const cv::Mat& mat);

private:
    cv::VideoCapture mCamera;
    uint16_t         mFramerate;

    std::atomic<bool> mIsOpened;
};
}

#endif // PRIME_APP_OPENCVBACKEND_H
