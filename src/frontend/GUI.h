#ifndef SOLAR_GUI_H
#define SOLAR_GUI_H

#include <queue>

#include <SFML/Graphics.hpp>

#include "backend/BackendOption.h"
#include "backend/PhotometricsBackend.h"
#include "misc/Log.h"
#include "videoproc/VideoProcessor.h"

namespace prm
{
    const uint16_t FRAME_QUEUE_SIZE = 60;

    class GUI
    {
    public:
        GUI(sf::RenderWindow& window, sf::Time& dt,
            std::unique_ptr<Backend>& backend, BackendOption& curr,
            VideoProcessor& videoproc, Log& log)
            : m_window(window), m_dt(dt), m_frameTimeQueue(),
              m_bShowMainMenuBar(true), m_bShowFrameInfoOverlay(false),
              m_bShowAppLog(true), m_bShowVideoProcessor(false),
              m_backend(backend), m_selectedBackend(curr),
              m_videoProcessor(videoproc), m_appLog(log), m_hubballiFont()
        {
        }

        bool Init();

        void Update();

        void Render();

        void Shutdown();

    private:
        void PollEvents();

        void ShowMainMenuBar();

        void ShowFrameInfoOverlay();

        void ShowVideoProcessor();

        void ShowViewport();

        void ShowAppLog();

        void ShowCameraButtons();

    private:
        sf::RenderWindow& m_window;
        sf::Time& m_dt;

        std::queue<float> m_frameTimeQueue;

        std::unique_ptr<Backend>& m_backend;
        BackendOption& m_selectedBackend;

        VideoProcessor& m_videoProcessor;
        Log& m_appLog;

        ImFont* m_hubballiFont;

        bool m_bShowMainMenuBar;
        bool m_bShowFrameInfoOverlay;
        bool m_bShowAppLog;
        bool m_bShowVideoProcessor;

        const uint16_t m_inputFieldWidth = 150;
    };
}// namespace prm

#endif//SOLAR_GUI_H