#pragma once

#include <SFML/Graphics.hpp>

#include "Renderer.h"
#include "backend/Backend.h"
#include "backend/BackendOption.h"
#include "backend/OpencvBackend.h"
#include "backend/PhotometricsBackend.h"
#include "frontend/GUI.h"
#include "misc/Log.h"
#include "videoproc/VideoProcessor.h"

namespace prm
{
    /**
     * Main application class that stores all the app data and runs the main app loop
     */
    class App
    {
    public:
        App(int argc, char** argv, uint16_t width, uint16_t height,
            const sf::ContextSettings& settings, Log& log)
            : m_windowWidth(width), m_windowHeight(height),
              m_contextSettings(settings),
              m_window(sf::VideoMode{width, height}, "Prime App",
                       sf::Style::Close, settings),
              m_deltaClock(), m_dt(), m_currentTexture(), m_textureMutex(),
              m_renderer(m_window, m_dt, m_currentTexture, m_textureMutex),
              m_backend(std::make_unique<OpencvBackend>(argc, argv, m_window,
                                                        m_currentTexture, m_dt,
                                                        m_textureMutex)),
              m_selectedBackend(OPENCV),
              m_gui(m_window, m_dt, m_backend, m_selectedBackend,
                    m_videoProcessor, log),
              m_videoProcessor(m_currentTexture, m_textureMutex)
        {
        }

        /**
         * Runs the main application loop, where each loop corresponds to one frame
         */
        void Run();

    private:
        /// Render window width and height
        uint16_t m_windowWidth, m_windowHeight;
        /// SFML context settings for the app (currently only antialiasing used)
        sf::ContextSettings m_contextSettings;

        /// SFML Render Window to draw the image to
        sf::RenderWindow m_window;

        /// SFML Clock that keeps track of app time
        sf::Clock m_deltaClock;
        /// Delta time last frame
        sf::Time m_dt;

        /// unique_ptr to the current camera backend
        std::unique_ptr<Backend> m_backend;
        /// Currently selected backend from the BackendOption enum
        BackendOption m_selectedBackend;

        /// Renderer that draws all stuff to the window
        Renderer m_renderer;
        /// App GUI instance
        GUI m_gui;
        /// Video Processor that handles NTA related image analysis
        VideoProcessor m_videoProcessor;

        /// SFML Texture to draw on screen this frame
        sf::Texture m_currentTexture;
        /// mutex for texture synchronisation
        std::mutex m_textureMutex;
    };
}// namespace prm