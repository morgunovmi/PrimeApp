#ifndef PRIME_APP_OPENCVBACKEND_H
#define PRIME_APP_OPENCVBACKEND_H

#include <opencv2/opencv.hpp>
#include <string_view>

#include "Backend.h"

namespace prm
{
    struct CvEvent
    {
        // Mutex that guards all other members
        std::mutex mutex{};
        // Condition that any thread could wait on
        std::condition_variable cond{};
        // A flag that helps with spurious wakeups
        bool flag{false};
    };

    struct OpencvCameraCtx
    {
        std::unique_ptr<cv::VideoCapture> camera;
        uint16_t framerate;

        CvEvent eofEvent;

        bool isCamOpen;

        bool isCapturing;

        std::unique_ptr<std::jthread> thread{nullptr};
        // Flag to be set to abort thread (used, for example, in multi-camera code samples)
        bool threadAbortFlag{false};
    };

    const uint16_t CV_DEFAULT_FPS = 30;

    class OpencvBackend : public Backend
    {
    public:
        OpencvBackend(int argc, char** argv, sf::RenderWindow& window,
                      sf::Texture& currentTexture, sf::Time& dt,
                      std::mutex& mutex)
            : Backend(argc, argv, window, currentTexture, dt, mutex),
              m_context(OpencvCameraCtx{nullptr, CV_DEFAULT_FPS, CvEvent{},
                                        false})
        {
        }

        explicit OpencvBackend(const std::unique_ptr<Backend>& other)
            : Backend(other), m_context(OpencvCameraCtx{nullptr, CV_DEFAULT_FPS,
                                                        CvEvent{}, false})
        {
        }

        void Init() override;

        void LiveCapture(SAVE_FORMAT format, bool save) override;

        void SequenceCapture(uint32_t nFrames, SAVE_FORMAT format,
                             bool save) override;

        void TerminateCapture() override;

        ~OpencvBackend() override { OpencvBackend::TerminateCapture(); };

    private:
        void Init_(OpencvCameraCtx& ctx);

        void Capture_(OpencvCameraCtx& ctx, SAVE_FORMAT format,
                      int32_t nFrames, bool save);

        static sf::Image MatToImage(const cv::Mat& mat);

        OpencvCameraCtx m_context;
    };
}// namespace prm

#endif// PRIME_APP_OPENCVBACKEND_H