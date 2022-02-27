#ifndef SOLAR_GAME_H
#define SOLAR_GAME_H

#include <SFML/Graphics.hpp>

#include <mutex>

#include "misc/Log.h"

namespace slr {
    class Backend {
    public:
        Backend(sf::RenderWindow& window, sf::Clock& deltaClock, sf::Time& dt, Log& appLog) :
            mWindow(window), mDeltaClock(deltaClock), mDt(dt), mAppLog(appLog), mPvcamMutex() {}

        bool Init();

        bool EnumerateCameras();

        bool Uninit();

        void Update();

    private:
        void PollInput();

    private:
        sf::RenderWindow&                       mWindow;

        sf::Clock&                              mDeltaClock;
        sf::Time&                               mDt;

        Log&                                    mAppLog;

        std::mutex                              mPvcamMutex;
    };
}

#endif //SOLAR_GAME_H
