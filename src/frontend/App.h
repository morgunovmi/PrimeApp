#ifndef SOLAR_APP_H
#define SOLAR_APP_H

#include <variant>

#include <SFML/Graphics.hpp>

#include "Renderer.h"
#include "backend/Backend.h"
#include "backend/OpencvBackend.h"
#include "backend/PhotometricsBackend.h"
#include "frontend/GUI.h"
#include "misc/Log.h"

namespace slr {
    class App {
    public:
        App(int argc, char** argv, uint16_t width, uint16_t height, const sf::ContextSettings& settings, Log& log) :
                mWindowWidth(width), mWindowHeight(height), mSettings(settings),
                mWindow(sf::VideoMode{width, height},
                        "Prime App", sf::Style::Close, settings),
                mDeltaClock(), mDt(), mCurrentTexture(), mTextureMutex(),
                mRenderer(mWindow, mDt,  mCurrentTexture, mTextureMutex),
                mBackend( argc, argv, mWindow, mCurrentTexture, mDeltaClock, mDt, mTextureMutex ),
                mGUI(mWindow, mDt, mBackend, log) {}

        void Init();
        void Run();

    private:
        uint16_t                        mWindowWidth, mWindowHeight;
        sf::ContextSettings             mSettings;

        sf::RenderWindow                mWindow;

        sf::Clock                       mDeltaClock;
        sf::Time                        mDt;

        Renderer                        mRenderer;
        OpencvBackend                   mBackend;
        GUI                             mGUI;

        sf::Texture                     mCurrentTexture;
        std::mutex                      mTextureMutex;
    };
}

#endif //SOLAR_APP_H
