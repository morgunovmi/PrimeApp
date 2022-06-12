#ifndef SOLAR_H
#define SOLAR_H

#include <array>
#include <mutex>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "frontend/ViewPort.h"

namespace prm
{
    class Renderer
    {
    public:
        Renderer(sf::RenderWindow& window, sf::Time& dt, sf::Texture& texture,
                 std::mutex& mutex)
            : m_window(window),
              m_viewport(static_cast<float>(window.getSize().x),
                         static_cast<float>(window.getSize().y)),
              m_dt(dt), m_currentTexture(texture), m_textureMutex(mutex)
        {
            Init();
        }

        void Render();

        void Display();

    private:
        void Init();

    private:
        sf::RenderWindow& m_window;
        ViewPort m_viewport;

        sf::Texture& m_currentTexture;
        std::mutex& m_textureMutex;

        sf::Time& m_dt;
    };
}// namespace prm

#endif