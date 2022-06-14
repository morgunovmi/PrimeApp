#pragma once

#include <queue>

#include <SFML/Graphics.hpp>

#include "backend/BackendOption.h"
#include "backend/PhotometricsBackend.h"
#include "misc/Log.h"
#include "videoproc/VideoProcessor.h"

namespace prm
{
    /// Size of the queue that stores recent frame times
    const uint16_t FRAME_QUEUE_SIZE = 60;

    /**
     * Class that defines the GUI functionality
     * Uses imgui and is drawn on top of the basic SFML stuff
     */
    class GUI
    {
    public:
        GUI(sf::RenderWindow& window, sf::Time& dt,
            std::unique_ptr<Backend>& backend, BackendOption& curr,
            VideoProcessor& videoproc, Log& log)
            : m_window(window), m_dt(dt), m_frameTimeQueue(),
              m_bShowMainMenuBar(true), m_bShowFrameInfoOverlay(false),
              m_bShowAppLog(true), m_bShowVideoProcessor(false),
              m_bShowHelp(true), m_backend(backend), m_selectedBackend(curr),
              m_videoProcessor(videoproc), m_appLog(log), m_hubballiFont()
        {
        }

        /**
         * Initializes imgui-SFML
         */
        void Init();

        /**
         * Updates the GUI each frame
         */
        void Update();

        /**
         * Renders the GUI
         */
        void Render();

        /**
         * Shuts down imgui-SFML
         */
        void Shutdown();

    private:
        /**
         * Handles imgui-SFML events like buttons and keyboard and mouse events
         */
        void PollEvents();

        /**
         * Draws the main menu bar at the top of the window
         */
        void ShowMainMenuBar();

        /**
         * Draws the frame time info plot in separate window
         */
        void ShowFrameInfoOverlay();

        /**
         * Draws the Video Processor window with all the image analysis functionality
         */
        void ShowVideoProcessor();

        /**
         * Draws log with all the application output
         */
        void ShowAppLog();

        /**
         * Draws window with camera related functionality
         */
        void ShowCameraButtons();

        void ShowHelp();

    private:
        /// Reference to the SFML render window
        sf::RenderWindow& m_window;
        /// Delta time last frame
        sf::Time& m_dt;

        /// Queue with recent frame times
        std::queue<float> m_frameTimeQueue;

        /// unique_ptr to the current camera backend
        std::unique_ptr<Backend>& m_backend;
        /// Currently selected backend from the BackendOption enum
        BackendOption& m_selectedBackend;

        /// Reference to the VideoProcessor instance
        VideoProcessor& m_videoProcessor;
        /// Reference to the app log
        Log& m_appLog;

        /// Pointer to a fancy font
        ImFont* m_hubballiFont;

        bool m_bShowMainMenuBar;
        bool m_bShowFrameInfoOverlay;
        bool m_bShowAppLog;
        bool m_bShowVideoProcessor;
        bool m_bShowHelp;

        /// Width for input fields in the GUI
        const uint16_t m_inputFieldWidth = 150;
    };
}// namespace prm