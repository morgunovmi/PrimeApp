#ifndef SOLAR_GUI_H
#define SOLAR_GUI_H

#include <queue>

#include <SFML/Graphics.hpp>
#include "misc/Log.h"
#include "backend/PhotometricsBackend.h"

namespace slr {
    const uint16_t FRAME_QUEUE_SIZE = 60;

    class GUI {
    public:
        GUI(sf::RenderWindow& window, sf::Time& dt, Backend& backend, sf::Texture& currentTexture, Log& log) : mWindow(window), mDt(dt),
            mFrameTimeQueue(),
            mShowMainMenuBar(true), mShowFrameInfoOverlay(false), mShowAppLog(true),
            mBackend(backend), mAppLog(log), mHubballiFont(), mCurrentTexture(currentTexture), mIsInit(false), mIsCapturing(false) {}

        bool Init();

        void Update();

        void Render();

        void Shutdown();

    private:
        void PollEvents();

        void ShowMainMenuBar();
        void ShowFrameInfoOverlay();
        void ShowViewport();
        void ShowAppLog();
        void ShowActionButtons();

        void launch();

    private:
        sf::RenderWindow&   mWindow;
        sf::Time&           mDt;

        std::queue<float>   mFrameTimeQueue;

        Backend&            mBackend;
        Log&                mAppLog;

        ImFont*             mHubballiFont;

        bool                mShowMainMenuBar;
        bool                mShowFrameInfoOverlay;
        bool                mShowAppLog;

        sf::Texture&        mCurrentTexture;

        bool                mIsInit;
        bool                mIsCapturing;
    };
}

#endif //SOLAR_GUI_H