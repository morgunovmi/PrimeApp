#pragma once

#include <queue>

#include <SFML/Graphics.hpp>
#include <SimpleSerial.h>

#include "backend/BackendOption.h"
#include "backend/ImageViewer.h"
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
            VideoProcessor& videoproc, ImageViewer& imageViewer, Log& log,
            sf::Texture& texture, std::mutex& mutex)
            : m_window(window), m_dt(dt), m_frameTimeQueue(),
              m_bShowMainMenuBar(true), m_bShowFrameInfoOverlay(false),
              m_bShowAppLog(true), m_bShowVideoProcessor(false),
              m_bShowImageViewer(false), m_bShowHelp(false),
              m_bShowViewport(true), m_bShowCameraButtons(true),
              m_bShowImageInfo(true), m_backend(backend),
              m_selectedBackend(curr), m_imageViewer(imageViewer),
              m_videoProcessor(videoproc), m_appLog(log), m_hubballiFont(),
              m_currentTexture(texture), m_textureMutex(mutex),
              m_bShowSerial(false)
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
         * Draws the latest captured image from the camera
         */
        void ShowViewport();

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

        /**
         * Draws window with image brightness control
         */
        void ShowImageInfo();

        void ShowImageViewer();

        /**
         * Draws window with help text
         */
        void ShowHelp();

        /**
         * Draws serial port control window
         */
        void ShowSerialPort();

        /**
         * Draws window with Region Of Interes selection sliders
         *
         * @param minX Min x coordinate of ROI
         * @param minY Min y coordinate of ROI
         * @param maxX Max x coordinate of ROI
         * @param maxY Max y coordinate of ROI
         */
        void ShowROISelector(float& minX, float& minY, float& maxX,
                             float& maxY);

    private:
        /**
         * Helper to display a little (?) mark which shows a tooltip when hovered.
         *
         * @param desc CString to display
         */
        static void HelpMarker(const char* desc)
        {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(desc);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }

        bool Combo(const char* label, int* current_item,
                   const std::vector<std::string>& items, int items_count,
                   int height_in_items = -1)
        {
            return ImGui::Combo(
                    label, current_item,
                    [](void* data, int idx, const char** out_text)
                    {
                        *out_text =
                                (*(const std::vector<std::string>*) data)[idx]
                                        .c_str();
                        return true;
                    },
                    (void*) &items, items_count, height_in_items);
        }

        /// Reference to the SFML render window
        sf::RenderWindow& m_window;
        /// Delta time last frame
        sf::Time& m_dt;

        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;

        /// Queue with recent frame times
        std::queue<float> m_frameTimeQueue;

        /// unique_ptr to the current camera backend
        std::unique_ptr<Backend>& m_backend;
        /// Currently selected backend from the BackendOption enum
        BackendOption& m_selectedBackend;

        /// Reference to the VideoProcessor instance
        VideoProcessor& m_videoProcessor;

        ImageViewer& m_imageViewer;
        /// Reference to the app log
        Log& m_appLog;

        /// Pointer to a fancy font
        ImFont* m_hubballiFont;

        bool m_bShowCameraButtons;
        bool m_bShowImageInfo;
        bool m_bShowMainMenuBar;
        bool m_bShowViewport;
        bool m_bShowFrameInfoOverlay;
        bool m_bShowAppLog;
        bool m_bShowVideoProcessor;
        bool m_bShowImageViewer;
        bool m_bShowHelp;
        bool m_bShowSerial;

        /// Width for input fields in the GUI
        const uint16_t m_inputFieldWidth = 150;

        /// Timepoint for last video processor metadata save
        std::chrono::high_resolution_clock::time_point m_lastMetaSave;

        /// Last save path folder used for video capture
        std::string m_videoSavePath{};
        /// Last load path folder used in video processor
        std::string m_videoLoadPath{};
    };
}// namespace prm