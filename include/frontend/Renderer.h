#ifndef SOLAR_H
#define SOLAR_H

#include <array>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "misc/Log.h"

namespace slr {
    class Renderer {
    public:
        Renderer(sf::RenderWindow& window, sf::Time& dt,
                Log& appLog) :
                mWindow(window), mDt(dt), mAppLog(appLog) {
            Init();
        }

        void Render();

        void Display();

    private:
        void Init();

    private:
        sf::RenderWindow&           mWindow;

        sf::Time&                   mDt;

        Log&                        mAppLog;
    };
}

#endif