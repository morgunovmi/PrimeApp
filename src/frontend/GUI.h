#ifndef SOLAR_GUI_H
#define SOLAR_GUI_H

#include <queue>

#include <SFML/Graphics.hpp>

#include "misc/Log.h"
#include "backend/PhotometricsBackend.h"
#include "videoproc/VideoProcessor.h"

namespace slr {
    const uint16_t FRAME_QUEUE_SIZE = 60;

    class GUI {
    public:
        GUI(sf::RenderWindow &window, sf::Time &dt, std::unique_ptr<Backend> &backend,
            VideoProcessor &videoproc, Log &log) : mWindow(window), mDt(dt),
                                                   mFrameTimeQueue(),
                                                   mShowMainMenuBar(
                                                           true),
                                                   mShowFrameInfoOverlay(
                                                           false),
                                                   mShowAppLog(
                                                           true),
                                                   mShowVideoProcessor(false),
                                                   mBackend(
                                                           backend),
                                                   mVideoProcessor(videoproc),
                                                   mAppLog(log),
                                                   mHubballiFont(),
                                                   mIsInit(false),
                                                   mIsCapturing(
                                                           false) {}

        bool Init();

        void Update();

        void Render();

        void Shutdown();

    private:
        void PollEvents();

        void ShowMainMenuBar();

        void ShowFrameInfoOverlay();

        void ShowVideoProcessor();

        void ShowViewport();

        void ShowAppLog();

        void ShowCameraButtons();

    private:
        sf::RenderWindow &mWindow;
        sf::Time &mDt;

        std::queue<float> mFrameTimeQueue;

        std::unique_ptr<Backend> &mBackend;
        VideoProcessor &mVideoProcessor;
        Log &mAppLog;

        ImFont *mHubballiFont;

        bool mShowMainMenuBar;
        bool mShowFrameInfoOverlay;
        bool mShowAppLog;
        bool mShowVideoProcessor;

        bool mIsInit;
        bool mIsCapturing;

        const uint16_t mInputFieldWidth = 150;
    };
}

#endif //SOLAR_GUI_H