#include <algorithm>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>

#include "../../vendor/ImGuiFileDialog/ImGuiFileDialog.h"
#include "frontend/GUI.h"
#include "misc/Meta.h"
#include "utils/FileUtils.h"

//TODO Disabled blocks
namespace prm
{
    void GUI::Init()
    {
        ImGui::SFML::Init(m_window);

        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        m_hubballiFont = io.Fonts->AddFontFromFileTTF(
                "./resources/fonts/hubballi-regular.ttf", 20);
        ImGui::SFML::UpdateFontTexture();

        ImGui::GetStyle().FrameRounding = 4.0f;
        ImGui::GetStyle().GrabRounding = 4.0f;

        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] =
                ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] =
                ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] =
                ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] =
                ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] =
                ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        const auto setupPath = std::filesystem::path{"setup.txt"};
        if (std::filesystem::exists(setupPath))
        {
            const auto strings = FileUtils::Tokenize(
                    FileUtils::ReadFileToString("setup.txt"));

            if (!strings.empty()) { m_videoSavePath = strings[0]; }
            if (strings.size() == 2) { m_videoLoadPath = strings[1]; }
        }
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
                if (ImGui::MenuItem("Viewport", nullptr, &m_bShowViewport)) {}
                if (ImGui::MenuItem("Camera Buttons", nullptr,
                                    &m_bShowCameraButtons))
                {
                }
                if (ImGui::MenuItem("Image Info", nullptr, &m_bShowImageInfo))
                {
                }
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
                if (ImGui::MenuItem("Show Help", nullptr, &m_bShowHelp)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void GUI::ShowViewport()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_HorizontalScrollbar;
        if (ImGui::Begin("Viewport", &m_bShowViewport, windowFlags))
        {
            std::scoped_lock lock{m_textureMutex};
            ImGui::Image(m_currentTexture);
        }
        ImGui::End();
    }

    void GUI::ShowAppLog()
    {
        // For the demo: add a debug button _BEFORE_ the normal log window contents
        // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
        // Most of the contents of the window will be added by the log.Draw() call.

        ImGui::Begin("App Log", &m_bShowAppLog);
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        m_appLog.Draw("App Log", &m_bShowAppLog);
    }

    void GUI::ShowCameraButtons()
    {
        if (ImGui::Begin("Camera Buttons", &m_bShowCameraButtons))
        {
            if (ImGui::Button("Init")) { m_backend->Init(); }

            static auto captureFormat = DIR;
            if (m_selectedBackend != PVCAM)
            {
                ImGui::RadioButton("tif", (int*) &captureFormat, 0);
                ImGui::SameLine();
                ImGui::RadioButton("mp4", (int*) &captureFormat, 1);
            }

            if (m_selectedBackend == PVCAM)
            {
                static int exposureTime = 10;
                ImGui::PushItemWidth(m_inputFieldWidth);
                if (ImGui::SliderInt("Exposure time, ms", &exposureTime, 5,
                                     100))
                {
                }
                ImGui::PopItemWidth();

                ImGui::SameLine();
                const char* items[] = {"1x1", "2x2"};
                static int currentBinning = 0;
                ImGui::PushItemWidth(m_inputFieldWidth);
                ImGui::Combo("Binning factor", &currentBinning, items,
                             IM_ARRAYSIZE(items));

                const char* items_lens[] = {"x10", "x20"};
                static int currentLens = X20;
                ImGui::Combo("Lens", &currentLens, items_lens,
                             IM_ARRAYSIZE(items));
                ImGui::PopItemWidth();
                ImGui::SameLine();
                HelpMarker("Lens type");
                ImGui::PopItemWidth();

                if (dynamic_cast<PhotometricsBackend*>(m_backend.get())
                            ->m_isPvcamInitialized)
                {
                    auto* ctx =
                            dynamic_cast<PhotometricsBackend*>(m_backend.get())
                                    ->GetCurrentCameraContext();
                    ctx->exposureTime = exposureTime;
                    ctx->lens = static_cast<Lens>(currentLens);
                    switch (currentBinning)
                    {
                        case 0:
                            ctx->region.pbin = 1;
                            ctx->region.sbin = 1;
                            break;
                        case 1:
                            ctx->region.pbin = 2;
                            ctx->region.sbin = 2;
                            break;
                        default:
                            spdlog::error(
                                    "Undefined binning factor encountered");
                    }
                }
            }

            static bool save = false;
            ImGui::Checkbox("Save to file", &save);
            ImGui::SameLine();

            if (ImGui::Button("Choose save directory"))
                ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseSaveDir", "Choose a Directory", nullptr,
                        m_videoSavePath.empty() ? "." : m_videoSavePath);

            if (ImGuiFileDialog::Instance()->Display("ChooseSaveDir"))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    auto dirPath =
                            ImGuiFileDialog::Instance()->GetCurrentPath();

                    const auto vidPath = std::filesystem::path{dirPath};
                    if (!std::filesystem::exists(vidPath))
                    {
                        spdlog::error(
                                "No such directory, please check the path");
                        return;
                    }

                    m_backend->SetDirPath(dirPath);
                    m_videoSavePath = dirPath;
                    spdlog::info("Saving to: {}", dirPath);
                }

                ImGuiFileDialog::Instance()->Close();
            }
            ImGui::SameLine();
            std::string helpString{"Right now saving to: "};
            helpString.append(m_backend->GetDirPath());
            HelpMarker(helpString.c_str());

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
        ImGui::DockSpaceOverViewport();

        if (m_bShowCameraButtons) ShowCameraButtons();
        if (m_bShowImageInfo) ShowImageInfo();
        if (m_bShowMainMenuBar) ShowMainMenuBar();
        if (m_bShowViewport) ShowViewport();
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

    void GUI::Shutdown()
    {
        if (auto ofs = std::ofstream{"setup.txt", std::ios_base::trunc})
        {
            ofs << m_videoSavePath << '\n' << m_videoLoadPath;
        }

        ImGui::SFML::Shutdown();
    }

    void GUI::ShowVideoProcessor()
    {
        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
        window_flags |= ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("Video Processor", &m_bShowVideoProcessor,
                         window_flags))
        {
            static std::string videoPath{};

            static int frameNum = 0;
            static int minMass = 1000;
            static double ecc = 0.5;
            static int size = 5;
            static int diameter = 19;

            if (ImGui::Button("Choose video file"))
            {
                ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileDlgKey", "Choose File", ".tif",
                        m_videoLoadPath.empty() ? "." : m_videoLoadPath);
            }

            static bool metaFound = true;
            TifStackMeta meta{};
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
            {
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    videoPath = ImGuiFileDialog::Instance()->GetFilePathName();

                    const auto vidPathStd = std::filesystem::path{videoPath};
                    if (!std::filesystem::exists(vidPathStd))
                    {
                        spdlog::error("No such file, please check the path");
                        return;
                    }

                    m_videoLoadPath =
                            ImGuiFileDialog::Instance()->GetCurrentPath();
                    spdlog::info("Tif file selected: {}", videoPath);

                    const auto metaPath =
                            std::string{m_videoLoadPath + "\\meta.json"};
                    if (!std::filesystem::exists(
                                std::filesystem::path{metaPath}))
                    {
                        spdlog::info("No metadata found");
                        metaFound = false;
                    }
                    else
                    {
                        using nlohmann::json;
                        spdlog::info("Found video capture metadata");
                        if (auto ifs = std::ifstream{metaPath})
                        {
                            meta = json::parse(ifs).get<TifStackMeta>();
                            metaFound = true;
                        }
                        else
                        {
                            spdlog::error("But couldn't parse it");
                            metaFound = false;
                        }
                    }
                }

                ImGuiFileDialog::Instance()->Close();

                m_videoProcessor.LoadVideo(videoPath);
                m_videoProcessor.LocateOneFrame(frameNum, minMass, ecc, size,
                                                diameter);
            }

            static bool isUpdateNeeded = false;
            ImGui::PushItemWidth(m_inputFieldWidth);

            ImGui::InputInt("frame num", &frameNum, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            ImGui::SameLine();
            HelpMarker("Number of the frame to analyze");

            ImGui::InputInt("min mass", &minMass, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;

            ImGui::SameLine();
            HelpMarker("The minimum integrated brightness.\n"
                       "This is a crucial parameter for eliminating spurious "
                       "features.");

            ImGui::InputInt("size", &size, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            ImGui::SameLine();
            HelpMarker(
                    "Max radius of gyration of blob's Gaussian-like profile");

            ImGui::InputInt("diameter", &diameter, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            ImGui::SameLine();
            HelpMarker("The feature’s extent in each dimension");

            ImGui::InputDouble("eccentricity", &ecc, 0.0, 0.0, "%.2f");
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            ImGui::SameLine();
            HelpMarker("Max eccentricity");
            ImGui::PopItemWidth();

            if (isUpdateNeeded)
            {
                m_videoProcessor.LocateOneFrame(frameNum, minMass, ecc, size,
                                                diameter);
                isUpdateNeeded = false;
            }

            if (ImGui::Button("Locate on all frames"))
            {
                m_videoProcessor.LocateAllFrames();
            }
            ImGui::SameLine();
            HelpMarker("Use this after fine tuning all previous params");

            static int searchRange = 7;
            static int memory = 10;
            static int minTrajLen = 5;
            static int driftSmoothing = 10;

            static int minDiagSize = 5;
            static int maxDiagSize = 30;
            ImGui::PushItemWidth(m_inputFieldWidth);

            ImGui::InputInt("search range", &searchRange, 0);
            ImGui::SameLine();
            HelpMarker("The maximum distance features can move between frames");

            ImGui::SameLine();
            ImGui::InputInt("memory", &memory, 0);
            ImGui::SameLine();
            HelpMarker("the maximum number of frames during which a feature "
                       "can vanish,\n"
                       "then reappear nearby, and be considered the same "
                       "particle");

            ImGui::InputInt("min traj. len", &minTrajLen, 0);
            ImGui::SameLine();
            HelpMarker("minimum number of points (video frames) to survive");

            ImGui::SameLine();
            ImGui::InputInt("drift smooth.", &driftSmoothing, 0);
            ImGui::SameLine();
            HelpMarker("Smooth the drift using a forward-looking\n"
                       "rolling mean over this many frames.");

            ImGui::InputInt("min diag. size", &minDiagSize, 0);
            ImGui::SameLine();
            HelpMarker("Min particle diagonal size to survive");

            ImGui::SameLine();
            ImGui::InputInt("max diag. size", &maxDiagSize, 0);
            ImGui::SameLine();
            HelpMarker("Max particle diagonal size to survive");

            ImGui::PopItemWidth();

            static bool ifNumFrames = false;
            static int numFrames = 30;
            if (ImGui::Button("Find trajectories"))
            {
                m_videoProcessor.LinkAndFilter(searchRange, memory, minTrajLen,
                                               driftSmoothing);
                m_videoProcessor.GroupAndPlotTrajectory(minDiagSize,
                                                        maxDiagSize);
            }

            /*
            ImGui::SameLine();
            ImGui::Checkbox("Specify number of frames", &ifNumFrames);
            if (ifNumFrames)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(m_inputFieldWidth);
                ImGui::SliderInt("##", &numFrames, 0, 1000);
                ImGui::PopItemWidth();
            }
             */

            static double fps = metaFound ? meta.fps : 6.66;
            static double scale = 330.0 / 675.0;

            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::InputDouble("fps", &fps, 0.0, 0.0, "%.3f");
            ImGui::SameLine();
            HelpMarker("Framerate of the image sequence capture");

            const char* items[] = {"x10", "x20"};
            static int currentLens = metaFound ? meta.lens : X10;
            ImGui::Combo("Lens", &currentLens, items, IM_ARRAYSIZE(items));
            ImGui::PopItemWidth();
            ImGui::SameLine();
            HelpMarker("Lens type");

            ImGui::SameLine();
            const char* itemsBinning[] = {"1x1", "2x2"};
            static int currentBinning = metaFound ? meta.binning : ONE;
            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::Combo("Binning factor", &currentBinning, itemsBinning,
                         IM_ARRAYSIZE(itemsBinning));
            ImGui::PopItemWidth();
            ImGui::SameLine();
            HelpMarker("Binning used during capture");

            switch (currentLens)
            {
                case 0:
                    scale = 330 / 306.6;
                    break;
                case 1:
                    scale = 330.0 / 675.0;
                    break;
            }
            scale *= currentBinning == 1 ? 2 : 1;

            if (ImGui::Button("Plot size distribution"))
            {
                m_videoProcessor.PlotSizeHist(fps, scale);
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
        auto window_flags =
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Help", &m_bShowHelp, window_flags))
        {
            ImGui::Text(
                    "1. Choose the backend in the Backend section:\n"
                    "   - OpenCV: Captures images from device's webcam\n"
                    "   - PVCam: Captures images from a connected Teledyne "
                    "camera\n\n"
                    "2. Use the Camera Buttons to capture images:\n"
                    "   - First initialize the camera with the Init button\n"
                    "   - Choose the file format\n"
                    "   - Choose if and where to save the file\n"
                    "   - Choose the image acquisition mode and specify number "
                    "of frames if necessary\n"
                    "       - For pvcam, Live Capture yields better fps \n"
                    "   - Stop the ongoing image acquisition with Terminate "
                    "Capture\n\n"
                    "3. Use the Video Processor module from the Windows menu \n"
                    "to analyze the captured image stacks with trackpy\n"
                    "   (Make sure there is no cyrillic in the tif stack "
                    "path)\n");
        }
        ImGui::End();
    }

    void GUI::ShowImageInfo()
    {
        auto window_flags =
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Image Info", &m_bShowImageInfo, window_flags))
        {
            if (m_selectedBackend == PVCAM)
            {
                auto* backend =
                        dynamic_cast<PhotometricsBackend*>(m_backend.get());
                const auto& counts =
                        dynamic_cast<PhotometricsBackend*>(m_backend.get())
                                ->m_brightnessCounts;

                ImGui::Text("Min: %hu Max: %hu", backend->m_minCurrentValue,
                            backend->m_maxCurrentValue);

                static int currentBitsIdx = 2;
                int maxVal = 0;
                switch (currentBitsIdx)
                {
                    case 0:
                        maxVal = UCHAR_MAX;
                        break;
                    case 1:
                        maxVal = 4095;
                        break;
                    case 2:
                        maxVal = USHRT_MAX;
                        break;
                }

                ImGui::DragIntRange2("Brightness range",
                                     &backend->m_minDisplayValue,
                                     &backend->m_maxDisplayValue, 5, 0, maxVal,
                                     "Min: %d", "Max: %d");

                ImGui::SameLine();
                ImGui::PushItemWidth(m_inputFieldWidth);
                const char* items[] = {"8 bit", "12 bit", "16 bit"};
                ImGui::Combo("Bit depth", &currentBitsIdx, items,
                             IM_ARRAYSIZE(items));
                ImGui::PopItemWidth();

                if (ImGui::Button("Full Scale"))
                {
                    backend->m_minDisplayValue = 0;
                    backend->m_maxDisplayValue = maxVal;
                }

                ImGui::SameLine();
                if (ImGui::Button("Auto stretch"))
                {
                    backend->m_minDisplayValue = backend->m_minCurrentValue;
                    backend->m_maxDisplayValue = backend->m_maxCurrentValue;
                }
            }
        }
        ImGui::End();
    }
}// namespace prm