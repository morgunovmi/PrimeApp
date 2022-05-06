#ifndef SOLAR_APP_H
#define SOLAR_APP_H

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

        void Init();

        void Run();

    private:
        uint16_t m_windowWidth, m_windowHeight;
        sf::ContextSettings m_contextSettings;

        sf::RenderWindow m_window;

        sf::Clock m_deltaClock;
        sf::Time m_dt;

        std::unique_ptr<Backend> m_backend;
        BackendOption m_selectedBackend;

        Renderer m_renderer;
        GUI m_gui;
        VideoProcessor m_videoProcessor;

        sf::Texture m_currentTexture;
        std::mutex m_textureMutex;
    };
}// namespace prm

#endif//SOLAR_APP_H
