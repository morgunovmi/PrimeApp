#ifndef PRIME_APP_OPENCVBACKEND_H
#define PRIME_APP_OPENCVBACKEND_H

#include <string_view>

#include <opencv2/opencv.hpp>

#include "Backend.h"

namespace slr {
    struct CvEvent {
        // Mutex that guards all other members
        std::mutex mutex{};
        // Condition that any thread could wait on
        std::condition_variable cond{};
        // A flag that helps with spurious wakeups
        bool flag{false};
    };

    struct OpencvCameraCtx {
        std::unique_ptr<cv::VideoCapture> camera;
        uint16_t framerate;

        CvEvent eofEvent;

        bool isCamOpen;

        bool isCapturing;

        std::unique_ptr<std::jthread> thread{nullptr};
        // Flag to be set to abort thread (used, for example, in multi-camera code samples)
        bool threadAbortFlag{false};
    };

    constexpr std::string_view LIVE_CAPTURE_PREFIX{"LiveCapture_"};
    constexpr std::string_view SEQ_CAPTURE_PREFIX{"SequenceCapture_"};

    const uint16_t CV_DEFAULT_FPS = 30;

    class OpencvBackend : public Backend {
    public:
        OpencvBackend(int argc, char **argv, sf::RenderWindow &window, sf::Texture &currentTexture, sf::Time &dt,
                      std::mutex &mutex) :
                Backend(argc, argv, window, currentTexture, dt, mutex),
                mContext(OpencvCameraCtx{nullptr, CV_DEFAULT_FPS, CvEvent{}, false}) {}

        explicit OpencvBackend(const std::unique_ptr<Backend> &other) : Backend(other),
                                                                        mContext(OpencvCameraCtx{nullptr,
                                                                                                 CV_DEFAULT_FPS,
                                                                                                 CvEvent{},
                                                                                                 false}) {}

        void Init() override;

        void LiveCapture() override;

        void SequenceCapture(uint32_t nFrames) override;

        void TerminateCapture() override;

        ~OpencvBackend() override {
            OpencvBackend::TerminateCapture();
        };

    private:
        void Init_(OpencvCameraCtx &ctx);

        void LiveCapture_(OpencvCameraCtx &ctx);

        void SequenceCapture_(OpencvCameraCtx &ctx, uint16_t nFrames);

        static sf::Image MatToImage(const cv::Mat &mat);

        OpencvCameraCtx mContext;
    };
}

#endif // PRIME_APP_OPENCVBACKEND_H