#ifndef SOLAR_APP_H
#define SOLAR_APP_H

#include "SFML/Graphics.hpp"

#include "Renderer.h"
#include "backend/Backend.h"
#include "frontend/GUI.h"
#include "misc/Log.h"

namespace slr {
    class App {
    public:
        App(uint16_t width, uint16_t height, const sf::ContextSettings& settings) :
                mWindowWidth(width), mWindowHeight(height), mSettings(settings),
                mWindow(sf::VideoMode{width, height},
                        "Prime App", sf::Style::Close, settings),
                mDeltaClock(), mDt(),
                mRenderer(mWindow, mDt, mAppLog),
                mBackend(mWindow, mDeltaClock, mDt, mAppLog),
                mGUI(mWindow, mDt, mBackend, mAppLog),
                mAppLog() {}

        void Init();
        void Run();

    private:
        uint16_t                        mWindowWidth, mWindowHeight;
        sf::ContextSettings             mSettings;

        sf::RenderWindow                mWindow;

        sf::Clock                       mDeltaClock;
        sf::Time                        mDt;

        Renderer                        mRenderer;
        Backend                         mBackend;
        GUI                             mGUI;

        Log                             mAppLog;
    };
}

#endif //SOLAR_APP_H
