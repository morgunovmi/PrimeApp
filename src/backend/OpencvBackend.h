#pragma once

#include <opencv2/opencv.hpp>
#include <string_view>

#include "Backend.h"

namespace prm
{
    /// Struct that groups camera related info
    struct OpencvCameraCtx
    {
        /// unique_ptr to the OpenCV VideoCapture device(camera)
        std::unique_ptr<cv::VideoCapture> camera;
        /// Target capture framerate
        uint16_t framerate;

        /// Specifies if the camera is open
        bool isCamOpen;

        /// Specifies if camera is capturing
        bool isCapturing;

        ///  unique_ptr to a worker jthread, that hanldes the capture
        std::unique_ptr<std::jthread> thread{nullptr};
    };

    /// Default capture framerate
    const uint16_t CV_DEFAULT_FPS = 30;

    /**
     * Backend implementation for OpenCV cameras
     * Connects to the device webcam and captures image sequence or video from it
     */
    class OpencvBackend : public Backend
    {
    public:
        OpencvBackend(int argc, char** argv, sf::RenderWindow& window,
                      sf::Texture& currentTexture, sf::Time& dt,
                      std::mutex& mutex)
            : Backend(argc, argv, window, currentTexture, dt, mutex),
              m_context(OpencvCameraCtx{nullptr, CV_DEFAULT_FPS,
                                        false})
        {
            Init();
        }

        /**
         * Explicit "copy" constructor from a unique_ptr
         *
         * @param other unique_ptr to a Backend from which to construct a new one
         */
        explicit OpencvBackend(const std::unique_ptr<Backend>& other)
            : Backend(other), m_context(OpencvCameraCtx{nullptr, CV_DEFAULT_FPS,
                                                        false})
        {
            Init();
        }

        /**
         * Handles generic camera initialization
         */
        void Init() override;

        /**
         * Captures live image sequence
         *
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void LiveCapture(SAVE_FORMAT format, bool save) override;

        /**
         * Captures image sequence of specified length
         *
         * @param nFrames Image sequence length
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void SequenceCapture(uint32_t nFrames, SAVE_FORMAT format,
                             bool save) override;

        /**
         * Stops the ongoing capture process
         */
        void TerminateCapture() override;

        ~OpencvBackend() override { OpencvBackend::TerminateCapture(); };

    private:
        /**
         * Internal init function to send to the worker thread
         *
         * @param ctx Camera context to initialize
         */
        void Init_(OpencvCameraCtx& ctx);

        /**
         * Generic internal capture function to send to the worker thread,
         * handles common functionality of live and sequence capture
         *
         * @param ctx Context of the capturing camera
         * @param format Save file format
         * @param nFrames Image sequence length (-1 for live capture)
         * @param save Specifies whether to save the captured sequence
         */
        void Capture_(OpencvCameraCtx& ctx, SAVE_FORMAT format,
                      int32_t nFrames, bool save);

        /**
         * Helper function to convert an OpenCV Mat to an SFML Image
         *
         * @param mat Mat to convert
         * @return Resulting SFML Image
         */
        static sf::Image MatToImage(const cv::Mat& mat);

        /// Current camera context
        OpencvCameraCtx m_context;
    };
}// namespace prm