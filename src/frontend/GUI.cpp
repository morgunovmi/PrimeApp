#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include "frontend/GUI.h"

//TODO Disabled blocks
namespace prm
{
    void GUI::Init()
    {
        ImGui::SFML::Init(m_window);

        auto& io = ImGui::GetIO();
        m_hubballiFont = io.Fonts->AddFontFromFileTTF(
                "./resources/fonts/hubballi-regular.ttf", 20);
        ImGui::SFML::UpdateFontTexture();
    }

    void GUI::PollEvents()
    {
        sf::Event event{};

        while (m_window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type)
            {
                case sf::Event::Closed:
                    m_window.close();
                    break;
                case sf::Event::KeyPressed:
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Escape:
                            m_window.close();
                            break;
                        case sf::Keyboard::LAlt:
                            m_bShowMainMenuBar = !m_bShowMainMenuBar;
                            break;
                        case sf::Keyboard::F1:
                            m_bShowVideoProcessor = !m_bShowVideoProcessor;
                            break;
                        case sf::Keyboard::F2:
                            m_bShowFrameInfoOverlay = !m_bShowFrameInfoOverlay;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void GUI::Update()
    {
        PollEvents();
        ImGui::SFML::Update(m_window, m_dt);

        m_frameTimeQueue.push(static_cast<float>(m_dt.asMicroseconds()));
        if (m_frameTimeQueue.size() > FRAME_QUEUE_SIZE)
        {
            m_frameTimeQueue.pop();
        }
    }

    void GUI::ShowFrameInfoOverlay()
    {
        static int corner = 1;
        auto window_flags = ImGuiWindowFlags_NoDecoration |
                            ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoSavedSettings |
                            ImGuiWindowFlags_NoFocusOnAppearing |
                            ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            const auto PAD = 10.0f;
            const auto* viewport = ImGui::GetMainViewport();
            auto work_pos =
                    viewport->WorkPos;// Use work area to avoid menu-bar/task-bar, if any!
            auto work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD)
                                        : (work_pos.x + PAD);
            window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD)
                                        : (work_pos.y + PAD);
            window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always,
                                    window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f);// Transparent background
        if (ImGui::Begin("FrameInfoOverlay", &m_bShowFrameInfoOverlay,
                         window_flags))
        {
            ImGui::Text("Frame info");
            ImGui::Separator();
            ImGui::Text("Frametime: %d ms\nFPS: %.3f", m_dt.asMilliseconds(),
                        1.f / m_dt.asSeconds());
            ImGui::PlotLines("Frame Times", &m_frameTimeQueue.front(),
                             static_cast<int>(m_frameTimeQueue.size()), 0,
                             nullptr, FLT_MAX, FLT_MAX, ImVec2{100, 40});

            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom", nullptr, corner == -1))
                    corner = -1;
                if (ImGui::MenuItem("Top-left", nullptr, corner == 0))
                    corner = 0;
                if (ImGui::MenuItem("Top-right", nullptr, corner == 1))
                    corner = 1;
                if (ImGui::MenuItem("Bottom-left", nullptr, corner == 2))
                    corner = 2;
                if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3))
                    corner = 3;
                if (ImGui::MenuItem("Close")) m_bShowFrameInfoOverlay = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void GUI::ShowMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Video Processor", "F1",
                                    &m_bShowVideoProcessor))
                {
                }
                if (ImGui::MenuItem("Frame Info", "F2",
                                    &m_bShowFrameInfoOverlay))
                {
                }
                if (ImGui::MenuItem("App Log", nullptr, &m_bShowAppLog)) {}
                ImGui::EndMenu();
            }

            auto oldOpt = m_selectedBackend;
            if (ImGui::BeginMenu("Backend"))
            {
                ImGui::RadioButton("OpenCV", (int*) &m_selectedBackend, 0);
                ImGui::RadioButton("PVCam", (int*) &m_selectedBackend, 1);
                ImGui::EndMenu();
            }

            if (oldOpt != m_selectedBackend)
            {
                switch (m_selectedBackend)
                {
                    case OPENCV:
                        m_backend = std::make_unique<OpencvBackend>(m_backend);
                        break;
                    case PVCAM:
                        m_backend = std::make_unique<PhotometricsBackend>(
                                m_backend);
                        break;
                }
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("Show Help", nullptr,
                                    &m_bShowHelp))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void GUI::ShowAppLog()
    {
        // For the demo: add a debug button _BEFORE_ the normal log window contents
        // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
        // Most of the contents of the window will be added by the log.Draw() call.

        const auto* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
                ImVec2(static_cast<float>(main_viewport->WorkPos.x) +
                               static_cast<float>(m_window.getSize().x) * 0.6f,
                       main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(m_window.getSize().x) * 0.4f,
                       static_cast<float>(m_window.getSize().y) / 2.f));

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("App Log", nullptr, window_flags);
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        m_appLog.Draw("App Log", &m_bShowAppLog);
    }

    void GUI::ShowCameraButtons()
    {
        const auto* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
                ImVec2(static_cast<float>(main_viewport->WorkPos.x) +
                               static_cast<float>(m_window.getSize().x) * 0.6f,
                       main_viewport->WorkPos.y +
                               static_cast<float>(m_window.getSize().y) / 2.f));
        ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(m_window.getSize().x) * 0.4f,
                       static_cast<float>(m_window.getSize().y) / 2.f -
                               main_viewport->WorkPos.y));

        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("Camera Buttons", nullptr, window_flags))
        {
            if (ImGui::Button("Init")) { m_backend->Init(); }

            static auto captureFormat = TIF;
            ImGui::RadioButton("tif", (int*) &captureFormat, 0);
            if (m_selectedBackend != PVCAM)
            {
                ImGui::SameLine();
                ImGui::RadioButton("mp4", (int*) &captureFormat, 1);
            }

            static bool save = false;
            ImGui::Checkbox("Save to file", &save);
            if (ImGui::Button("Live capture"))
            {
                m_backend->LiveCapture(captureFormat, save);
            }

            static int nFrames = 50;
            if (ImGui::Button("Sequence capture"))
            {
                m_backend->SequenceCapture(nFrames, captureFormat, save);
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(m_inputFieldWidth);
            if (ImGui::SliderInt("Number of frames", &nFrames, 0, 1000)) {}
            ImGui::PopItemWidth();


            if (ImGui::Button("Terminate capture"))
            {
                m_backend->TerminateCapture();
            }
        }

        ImGui::End();
    }

    void GUI::Render()
    {
        ImGui::PushFont(m_hubballiFont);
        ShowCameraButtons();
        if (m_bShowMainMenuBar) ShowMainMenuBar();
        if (m_bShowFrameInfoOverlay) ShowFrameInfoOverlay();
        if (m_bShowVideoProcessor) ShowVideoProcessor();
        if (m_bShowAppLog) ShowAppLog();
        if (m_bShowHelp) ShowHelp();

#ifndef NDEBUG
        ImGui::ShowDemoWindow();
#endif

        ImGui::PopFont();
        ImGui::SFML::Render(m_window);
    }

    void GUI::Shutdown() { ImGui::SFML::Shutdown(); }

    void GUI::ShowVideoProcessor()
    {
        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
        window_flags |= ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("Video Processor", nullptr, window_flags))
        {
            static std::string videoPath{};
            ImGui::InputTextWithHint("Video path", ".tif file path",
                                     &videoPath);

            if (ImGui::Button("Load Video"))
            {
                m_videoProcessor.LoadVideo(videoPath);
            }

            static int frameNum = 0;
            static int minMass = 1000;
            static double ecc = 0.5;
            static int size = 5;
            static int diameter = 19;

            ImGui::PushItemWidth(m_inputFieldWidth);

            ImGui::InputInt("frame num", &frameNum);
            ImGui::SameLine();
            HelpMarker("Number of the frame to analyze");

            ImGui::InputInt("min mass", &minMass);
            ImGui::SameLine();
            HelpMarker("The minimum integrated brightness.\n"
                       "This is a crucial parameter for eliminating spurious features.");

            ImGui::InputInt("size", &size);
            ImGui::SameLine();
            HelpMarker("Max radius of gyration of blob's Gaussian-like profile");

            ImGui::InputInt("diameter", &diameter);
            ImGui::SameLine();
            HelpMarker("The feature’s extent in each dimension");

            ImGui::InputDouble("eccentricity", &ecc, 0.0, 0.0, "%.2f");
            ImGui::SameLine();
            HelpMarker("Max eccentricity");
            ImGui::PopItemWidth();

            if (ImGui::Button("Locate on one frame"))
            {
                m_videoProcessor.LocateOneFrame(frameNum, minMass, ecc, size,
                                                diameter);
            }

            static int searchRange = 7;
            static int memory = 10;
            static int minTrajLen = 5;
            static int driftSmoothing = 10;

            static int minDiagSize = 5;
            static int maxDiagSize = 30;
            ImGui::PushItemWidth(m_inputFieldWidth);

            ImGui::InputInt("search range", &searchRange);
            ImGui::SameLine();
            HelpMarker("The maximum distance features can move between frames");

            ImGui::SameLine();
            ImGui::InputInt("memory", &memory);
            ImGui::SameLine();
            HelpMarker("the maximum number of frames during which a feature can vanish,\n"
                       "then reappear nearby, and be considered the same particle");

            ImGui::InputInt("min traj. len", &minTrajLen);
            ImGui::SameLine();
            HelpMarker("minimum number of points (video frames) to survive");

            ImGui::SameLine();
            ImGui::InputInt("drift smooth.", &driftSmoothing);
            ImGui::SameLine();
            HelpMarker("Smooth the drift using a forward-looking\n"
                       "rolling mean over this many frames.");

            ImGui::InputInt("min diag. size", &minDiagSize);
            ImGui::SameLine();
            HelpMarker("Min particle diagonal size to survive");

            ImGui::SameLine();
            ImGui::InputInt("max diag. size", &maxDiagSize);
            ImGui::SameLine();
            HelpMarker("Max particle diagonal size to survive");

            ImGui::PopItemWidth();

            if (ImGui::Button("Find trajectories"))
            {
                m_videoProcessor.LocateAllFrames();
                m_videoProcessor.LinkAndFilter(searchRange, memory, minTrajLen,
                                               driftSmoothing);
                m_videoProcessor.GroupAndPlotTrajectory(minDiagSize,
                                                        maxDiagSize);
            }

            static double fps = 6.66;
            static double scale = 330.0 / 675.0;
            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::InputDouble("fps", &fps, 0.0, 0.0, "%.3f");
            ImGui::SameLine();
            HelpMarker("Framerate of the image sequence capture");

            ImGui::InputDouble("scale", &scale, 0.0, 0.0, "%.3f");
            ImGui::SameLine();
            HelpMarker("Microns per pixel");
            ImGui::PopItemWidth();
            if (ImGui::Button("Plot size distribution"))
            {
                m_videoProcessor.PlotSizeHist(fps, scale);
            }

            if (ImGui::Button("Get particle size"))
            {
                m_videoProcessor.GetSize(fps, scale);
            }

            static std::string pythonQuery{};
            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::InputTextWithHint("Run random python string", "Python Query",
                                     &pythonQuery);
            ImGui::PopItemWidth();

            if (ImGui::Button("Run python query"))
            {
                m_videoProcessor.RunPythonQuery(pythonQuery);
            }
        }

        ImGui::End();
    }

    void GUI::ShowHelp()
    {
        auto window_flags = ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Help", &m_bShowHelp, window_flags))
        {
            ImGui::Text("1. Choose the backend in the Backend section:\n"
                        "   - OpenCV: Captures images from device's webcam\n"
                        "   - PVCam: Captures images from a connected Teledyne camera\n\n"
                        "2. Use the Camera Buttons to capture images:\n"
                        "   - First initialize the camera with the Init button\n"
                        "   - Choose the file format\n"
                        "   - Choose the image acquisition mode and specify number of frames if necessary\n"
                        "       - For pvcam, Live Capture yields better fps \n"
                        "   - Stop the ongoing image acquisition with Terminate Capture\n"
                        "3. Use the Video Processor module from the Windows menu \n"
                        "to analyze the captured image stacks with trackpy\n"
                        "   (Make sure there is no cyrillic in the tif stack path)\n\n"
                        "- To move the image around use the arrow keys\n"
                        "- Zoom it with J and K keys\n");
        }
        ImGui::End();
    }
}// namespace prm