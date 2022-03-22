#ifndef SOLAR_H
#define SOLAR_H

#include <array>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "misc/Log.h"

namespace slr {
    class Renderer {
    public:
        Renderer(sf::RenderWindow &window, sf::Time &dt,
                 sf::Texture &texture, std::mutex &mutex) :
                mWindow(window), mDt(dt), mCurrentTexture(texture), mTextureMutex(mutex) {
            Init();
        }

        void Render();

        void Display();

    private:
        void Init();

    private:
        sf::RenderWindow &mWindow;

        sf::Texture &mCurrentTexture;
        std::mutex &mTextureMutex;

        sf::Time &mDt;
    };
}

#endif