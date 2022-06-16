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
            : m_window(sf::VideoMode{width, height}, "Prime App",
                       sf::Style::Default, settings),
              m_deltaClock(), m_dt(), m_currentTexture(), m_textureMutex(),
              m_renderer(m_window),
              m_backend(std::make_unique<OpencvBackend>(argc, argv, m_window,
                                                        m_currentTexture, m_dt,
                                                        m_textureMutex)),
              m_selectedBackend(OPENCV),
              m_gui(m_window, m_dt, m_backend, m_selectedBackend,
                    m_videoProcessor, log, m_currentTexture, m_textureMutex),
              m_videoProcessor(m_currentTexture, m_textureMutex)
        {
            sf::Image icon{};
            if (!icon.loadFromFile("./resources/images/cam_icon.jpg"))
            {
                spdlog::error("Failed to load app icon");
            }
            else
            {
                m_window.setIcon(icon.getSize().x, icon.getSize().y,
                                 icon.getPixelsPtr());
            }
        }

        /**
         * Runs the main application loop, where each loop corresponds to one frame
         */
        void Run();

    private:
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