#ifndef SOLAR_H
#define SOLAR_H

#include <array>
#include <mutex>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>

#include "frontend/ViewPort.h"

namespace prm
{
    /**
     * Class for drawing stuff to the SFML window
     */
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
            m_window.setFramerateLimit(60);
        }

        /**
         * Draws a texture to the SFML window each frame
         */
        void Render();

        /**
         * Displays everything drawn to the SFML window on the display
         */
        void Display();

    private:
        /// Reference to the SFML render window
        sf::RenderWindow& m_window;
        /// ViewPort instance that describes what region of the image is displayed
        ViewPort m_viewport;

        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;

        /// Delta time for last frame
        sf::Time& m_dt;
    };
}// namespace prm

#endif