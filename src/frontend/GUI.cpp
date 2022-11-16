#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>

#include "../../vendor/ImGuiFileDialog/ImGuiFileDialog.h"
#include "frontend/GUI.h"
#include "misc/Meta.h"
#include "utils/FileUtils.h"
#include "utils/MySerial.h"

//TODO Disabled blocks
namespace prm
{
    void GUI::Init()
    {
        ImGui::SFML::Init(m_window);

        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        m_hubballiFont = io.Fonts->AddFontFromFileTTF(
                "./resources/fonts/Montserrat-Light.ttf", 23);
        ImGui::SFML::UpdateFontTexture();

        auto& style = ImGui::GetStyle();
        style.WindowPadding = {10.f, 10.f};
        style.FrameRounding = 6.0f;
        style.GrabRounding = 12.0f;
        style.FramePadding = ImVec2(12, 5);
        style.ItemSpacing = ImVec2(10, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        style.ScrollbarSize = 20.f;
        style.GrabMinSize = 18.f;

        style.WindowRounding = 12.f;
        style.ScrollbarRounding = 0.f;

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

        const auto setupPath = std::filesystem::path{"setup.json"};
        if (std::filesystem::exists(setupPath))
        {
            if (auto ifs = std::ifstream{setupPath})
            {
                const auto j = nlohmann::json::parse(ifs);

                if (j.contains("savePath"))
                {
                    j.at("savePath").get_to(m_videoSavePath);
                    m_backend->SetDirPath(m_videoSavePath);
                }
                if (j.contains("loadPath"))
                {
                    j.at("loadPath").get_to(m_videoLoadPath);
                }
            }
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
                        case sf::Keyboard::F3:
                            m_bShowImageViewer = !m_bShowImageViewer;
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
                if (ImGui::MenuItem("Image Viewer", "F3", &m_bShowImageViewer))
                {
                }
                if (ImGui::MenuItem("Laser Controller", nullptr,
                                    &m_bShowSerial))
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
            if (ImGui::Button("Init Camera")) { m_backend->Init(); }

            ImGui::Dummy({0.f, 10.f});

            ImGui::Text("Capture parameters");
            ImGui::Separator();

            static auto captureFormat = DIR;
            if (m_selectedBackend != PVCAM)
            {
                ImGui::RadioButton("tif", (int*) &captureFormat, 0);
                ImGui::SameLine();
                ImGui::RadioButton("mp4", (int*) &captureFormat, 1);
            }

            static bool save = false;
            if (m_selectedBackend == PVCAM)
            {
                ImGui::BeginGroup();

                const char* items[] = {"1x1", "2x2"};
                static int currentBinning = 0;
                ImGui::PushItemWidth(m_inputFieldWidth);
                ImGui::Combo("Binning factor", &currentBinning, items,
                             IM_ARRAYSIZE(items));

                const char* items_lens[] = {"x10", "x20"};
                static int currentLens = X20;
                ImGui::Combo("Lens", &currentLens, items_lens,
                             IM_ARRAYSIZE(items));
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::PopItemWidth();

                ImGui::BeginGroup();
                static int exposureTime = 10;
                ImGui::PushItemWidth(m_inputFieldWidth);
                if (ImGui::SliderInt("Exposure time, ms", &exposureTime, 5,
                                     100))
                {
                }
                ImGui::PopItemWidth();

                static bool showRoi = false;
                ImGui::Checkbox("Select ROI", &showRoi);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Click to bring up a ROI selector window");
                }
                static float minX = 0.0, minY = 0.0, maxX = 1.0, maxY = 1.0;
                if (showRoi) { ShowROISelector(minX, minY, maxX, maxY); }


                if (dynamic_cast<PhotometricsBackend*>(m_backend.get())
                            ->m_isPvcamInitialized)
                {
                    auto* ctx =
                            dynamic_cast<PhotometricsBackend*>(m_backend.get())
                                    ->GetCurrentCameraContext();

                    ctx->region.s1 = minX * ctx->sensorResX;
                    ctx->region.s2 = maxX * (ctx->sensorResX - 1);
                    ctx->region.p1 = minY * ctx->sensorResY;
                    ctx->region.p2 = maxY * (ctx->sensorResY - 1);

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
                ImGui::EndGroup();
            }

            ImGui::Dummy({0.f, 10.f});
            ImGui::Text("File saving");
            ImGui::Separator();

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

            ImGui::Dummy({0.f, 10.f});
            ImGui::Text("Capture control");
            ImGui::Separator();

            if (ImGui::Button("Live capture", {200.f, 40.f}))
            {
                m_backend->LiveCapture(captureFormat, save);
            }

            static int nFrames = 50;
            if (ImGui::Button("Sequence capture", {200.f, 40.f}))
            {
                m_backend->SequenceCapture(nFrames, captureFormat, save);
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(m_inputFieldWidth);
            if (ImGui::SliderInt("Number of frames", &nFrames, 0, 1000)) {}
            ImGui::PopItemWidth();

            if (ImGui::Button("Terminate capture", {200.f, 40.f}))
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
        if (m_bShowImageViewer) ShowImageViewer();
        if (m_bShowAppLog) ShowAppLog();
        if (m_bShowHelp) ShowHelp();
        if (m_bShowSerial) ShowSerialPort();

#ifndef NDEBUG
        ImGui::ShowDemoWindow();
#endif

        ImGui::PopFont();
        ImGui::SFML::Render(m_window);
    }

    void GUI::Shutdown()
    {
        if (auto ofs = std::ofstream{"setup.json", std::ios_base::trunc})
        {
            ofs << nlohmann::json{{"savePath", m_videoSavePath},
                                  {"loadPath", m_videoLoadPath}}
                            .dump(4);
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
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Choose the tif stack to analyze");
            }

            static bool isFileLoaded = false;

            static double fps = 6.66;
            static int currentLens = X10;
            static int currentBinning = ONE;
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
                    isFileLoaded = true;
                    spdlog::info("Tif file selected: {}", videoPath);

                    const auto metaPath =
                            std::string{m_videoLoadPath + "\\meta.json"};
                    if (!std::filesystem::exists(
                                std::filesystem::path{metaPath}))
                    {
                        spdlog::info("No metadata found");
                    }
                    else
                    {
                        using nlohmann::json;
                        spdlog::info("Found video capture metadata");
                        if (auto ifs = std::ifstream{metaPath})
                        {
                            TifStackMeta meta{};
                            meta = json::parse(ifs).get<TifStackMeta>();
                            fps = meta.fps;
                            currentBinning = meta.binning;
                            currentLens = meta.lens;
                        }
                        else
                        {
                            spdlog::error("But couldn't parse it");
                        }
                    }
                }

                ImGuiFileDialog::Instance()->Close();

                m_videoProcessor.LoadVideo(videoPath);
                m_videoProcessor.LocateOneFrame(frameNum, minMass, ecc, size,
                                                diameter);
            }

            ImGui::Dummy({0.f, 10.f});
            ImGui::Text("Feature locating");
            ImGui::Separator();

            ImGui::BeginGroup();
            static bool isUpdateNeeded = false;
            ImGui::PushItemWidth(m_inputFieldWidth);

            ImGui::InputInt("frame num", &frameNum, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Number of the frame to analyze");
            }

            ImGui::InputInt("min mass", &minMass, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The minimum integrated brightness.\n"
                       "This is a crucial parameter for eliminating spurious "
                       "features.");
            }

            ImGui::InputInt("size", &size, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip(
                    "Max radius of gyration of blob's Gaussian-like profile");
            }
            ImGui::EndGroup();
            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::InputInt("diameter", &diameter, 0);
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The feature's extent in each dimension");
            }

            ImGui::InputDouble("eccentricity", &ecc, 0.0, 0.0, "%.2f");
            if (ImGui::IsItemDeactivatedAfterEdit()) isUpdateNeeded = true;
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Max eccentricity");
            }
            ImGui::PopItemWidth();
            ImGui::EndGroup();

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

            ImGui::Dummy({0.f, 10.f});
            ImGui::Text("Feature linking");
            ImGui::Separator();

            static int searchRange = 7;
            static int memory = 10;
            static int minTrajLen = 5;
            static int driftSmoothing = 10;

            static int minDiagSize = 5;
            static int maxDiagSize = 30;
            ImGui::PushItemWidth(m_inputFieldWidth);


            ImGui::BeginGroup();
            ImGui::InputInt("search range", &searchRange, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The maximum distance features can move between frames");
            }

            ImGui::InputInt("memory", &memory, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("The maximum number of frames during which a feature "
                       "can vanish,\n"
                       "then reappear nearby, and be considered the same "
                       "particle");
            }

            ImGui::InputInt("min traj. len", &minTrajLen, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Minimum number of points (video frames) to survive");
            }
            ImGui::EndGroup();

            ImGui::SameLine();
            ImGui::BeginGroup();
            ImGui::InputInt("drift smoothing", &driftSmoothing, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Smooth the drift using a forward-looking\n"
                       "rolling mean over this many frames.");
            }

            ImGui::InputInt("min diag. size", &minDiagSize, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Min particle diagonal size to survive");
            }

            ImGui::InputInt("max diag. size", &maxDiagSize, 0);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Max particle diagonal size to survive");
            }
            ImGui::EndGroup();

            ImGui::PopItemWidth();

            static bool ifNumFrames = false;
            static int numFrames = 30;
            static bool plotTrajectories = true;
            if (ImGui::Button("Link"))
            {
                m_videoProcessor.LinkAndFilter(searchRange, memory, minTrajLen,
                                               driftSmoothing);
                if (plotTrajectories)
                {
                    m_videoProcessor.GroupAndPlotTrajectory(minDiagSize,
                                                            maxDiagSize);
                }
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Estimates particle trajectories between frames");
            }
            ImGui::SameLine();
            ImGui::Checkbox("Plot trajectories", &plotTrajectories);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Check if you want to see the found trajectories\n(takes some time)");
            }

            ImGui::Dummy({0.f, 10.f});
            ImGui::Text("Particle size calculation");
            ImGui::Separator();

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

            static double scale = 330.0 / 675.0;

            ImGui::BeginGroup();
            ImGui::PushItemWidth(m_inputFieldWidth);
            const char* items[] = {"x10", "x20"};
            ImGui::Combo("Lens", &currentLens, items, IM_ARRAYSIZE(items));
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Lens used on the camera during capture");
            }

            const char* itemsBinning[] = {"1x1", "2x2"};
            ImGui::Combo("Binning factor", &currentBinning, itemsBinning,
                         IM_ARRAYSIZE(itemsBinning));
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Binning used during capture");
            }
            ImGui::EndGroup();

            ImGui::SameLine();

            ImGui::InputDouble("fps", &fps, 0.0, 0.0, "%.3f");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Framerate of the captured stack");
            }
            ImGui::PopItemWidth();

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

            static int num_bins = 700;
            if (ImGui::Button("Plot size distribution"))
            {
                m_videoProcessor.PlotSizeHist(fps, scale, num_bins);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Plots a histogram with particle size distribution params");
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::SliderInt("Num bins", &num_bins, 0, 1000);
            ImGui::PopItemWidth();
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Number of bins in the histogram");
            }

            ImGui::Dummy({0.f, 10.f});
            if (ImGui::CollapsingHeader("Misc"))
            {
                static std::string pythonQuery{};
                ImGui::PushItemWidth(m_inputFieldWidth);
                ImGui::InputTextWithHint("Run random python string",
                                         "Python Query", &pythonQuery);
                ImGui::PopItemWidth();

                if (ImGui::Button("Run python query"))
                {
                    m_videoProcessor.RunPythonQuery(pythonQuery);
                }
            }

            if (isFileLoaded &&
                std::chrono::duration<double, std::milli>{
                        std::chrono::high_resolution_clock::now() -
                        m_lastMetaSave}
                                .count() > 10000)
            {
                const auto vpMeta = VideoProcessorMeta{
                        .minMass = minMass,
                        .ecc = ecc,
                        .size = size,
                        .diameter = diameter,
                        .searchRange = searchRange,
                        .memory = memory,
                        .minTrajLen = minTrajLen,
                        .driftSmoothing = driftSmoothing,
                        .minDiagSize = minDiagSize,
                        .maxDiagSize = maxDiagSize,
                        .lens = static_cast<Lens>(currentLens),
                        .binning = static_cast<Binning>(currentBinning),
                        .scale = scale,
                        .fps = fps};

                if (auto ofs = std::ofstream{m_videoLoadPath +
                                             "\\video_processor_meta.json"})
                {
                    ofs << json{vpMeta}.dump(4);
                    m_lastMetaSave = std::chrono::high_resolution_clock::now();
                }
            }
        }

        ImGui::End();
    }

    void GUI::ShowROISelector(float& minX, float& minY, float& maxX,
                              float& maxY)
    {

        ImGui::SetNextWindowSize(ImVec2{350, 400});
        if (ImGui::Begin("ROI", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Select Region of interest for capture");
            ImGui::DragFloatRange2("X range", &minX, &maxX, 0.02f, 0.0f, 1.0f,
                                   "%.2f");
            ImGui::DragFloatRange2("Y range", &minY, &maxY, 0.02f, 0.0f, 1.0f,
                                   "%.2f");
            ImGui::Separator();

            ImVec2 canvas_p0 = ImGui::
                    GetCursorScreenPos();// ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::
                    GetContentRegionAvail();// Resize canvas to what's available
            canvas_sz.x = canvas_sz.y;
            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x,
                                      canvas_p0.y + canvas_sz.y);

            // Draw border and background color
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(canvas_p0, canvas_p1,
                                     IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1,
                               IM_COL32(255, 255, 255, 255));

            ImVec2 roiP0{canvas_p0.x + minX * (canvas_p1.x - canvas_p0.x),
                         canvas_p0.y + minY * (canvas_p1.y - canvas_p0.y)};

            ImVec2 roiP1{canvas_p0.x + maxX * (canvas_p1.x - canvas_p0.x),
                         canvas_p0.y + maxY * (canvas_p1.y - canvas_p0.y)};
            draw_list->AddRect(roiP0, roiP1, IM_COL32(0, 255, 0, 255));

            ImGui::End();
        }
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
                    "   - First initialize the camera with the Init button if "
                    "autoinit failed\n"
                    "   - Choose if and where to save the file\n"
                    "   - Choose the image acquisition mode and specify number "
                    "of frames if necessary\n"
                    "       - For pvcam, Live Capture yields better fps \n"
                    "   - Stop the ongoing image acquisition with Terminate "
                    "Capture\n\n"
                    "3. Use the Video Processor module from the Windows menu \n"
                    "to analyze the captured image stacks with trackpy\n");
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

                ImGui::Text("Brightness range control");
                ImGui::Separator();
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
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Set brightness limits to the whole range\n of the current bit depth");
                }

                ImGui::SameLine();
                if (ImGui::Button("Auto stretch"))
                {
                    backend->m_minDisplayValue = backend->m_minCurrentValue;
                    backend->m_maxDisplayValue = backend->m_maxCurrentValue;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Stretch the brightness limits to the values on the frame");
                }

                ImGui::Dummy({0.f, 5.f});
                ImGui::Text("Current frame values");
                ImGui::Separator();
                ImGui::TextColored({0.f, 0.7, 0.f, 1.f}, "Min: %hu Max: %hu", backend->m_minCurrentValue,
                            backend->m_maxCurrentValue);
            }
        }
        ImGui::End();
    }

    void GUI::ShowImageViewer()
    {
        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
        window_flags |= ImGuiWindowFlags_NoResize;

        static int maxImages = 100;
        if (ImGui::Begin("Image Viewer", &m_bShowImageViewer, window_flags))
        {
            static std::string videoPath{};
            if (ImGui::Button("Choose video file"))
            {
                ImGuiFileDialog::Instance()->OpenDialog(
                        "ChooseFileDlgKeyViewer", "Choose File", ".tif",
                        m_videoLoadPath.empty() ? "." : m_videoLoadPath);
            }

            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKeyViewer"))
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

                    spdlog::info("Tif file selected: {}", videoPath);
                }

                ImGuiFileDialog::Instance()->Close();

                m_imageViewer.LoadImageStack(videoPath, maxImages);
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(m_inputFieldWidth);
            ImGui::DragInt("Max loaded images", &maxImages, 1.0f, 1, 1000);

            if (ImGui::Button("Update Image")) { m_imageViewer.UpdateImage(); }

            static bool processAllFrames = false;
            ImGui::Checkbox("Process all frames", &processAllFrames);
            ImGui::SameLine();
            HelpMarker("By default processes only frame 0");

            static int topHatSize = 15;
            if (ImGui::Button("Top hat filter"))
            {
                m_imageViewer.TopHatFilter(topHatSize, processAllFrames);
            }
            ImGui::SameLine();
            ImGui::DragInt("Filter size top hat", &topHatSize, 1.0f, 1, 50);

            /*
            static int medianSize = 5;
            if (ImGui::Button("Median filter"))
            {
                m_imageViewer.MedianFilter(medianSize, processAllFrames);
            }
            ImGui::SameLine();
            ImGui::DragInt("Filter size median", &medianSize, 2.0f, 1, 5);
             */

            if (ImGui::Button("Median filter"))
            {
                m_imageViewer.ScuffedMedianFilter(processAllFrames);
            }

            if (ImGui::Button("Reset Image")) { m_imageViewer.ResetImage(); }

            static int frameNum = 0;
            if (m_imageViewer.m_isImageLoaded)
            {
                ImGui::DragInt("Frane number", &frameNum, 1.0f, 0,
                               m_imageViewer.GetNumImages() - 1);
                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    m_imageViewer.SelectImage(frameNum);
                }
            }
            ImGui::PopItemWidth();

            if (ImGui::Button("Save"))
            {
                const auto pathStd = std::filesystem::path{videoPath};

                const auto savePath = fmt::format(
                        "{}\\{}_mod{}", pathStd.parent_path().string(),
                        pathStd.stem().string(), pathStd.extension().string());

                m_imageViewer.SaveImage(savePath);
            }
        }
        ImGui::End();
    }

    void GUI::ShowSerialPort()
    {
        if (ImGui::Begin("Laser Controller", &m_bShowImageViewer))
        {
            ULONG size = 10;
            std::vector<ULONG> coms(size);
            ULONG found = 0;

            GetCommPorts(coms.data(), size, &found);
            ::ranges::sort(coms.begin(), coms.end());
            auto portStrs = coms |
                            ::ranges::views::filter([](auto comNum)
                                                    { return comNum != 0; }) |
                            ::ranges::views::transform(
                                    [](auto comNum)
                                    { return fmt::format("COM{}", comNum); }) |
                            ::ranges::to<std::vector<std::string>>();

            const int numPorts = static_cast<int>(portStrs.size());
            static int currentPort = numPorts - 1;

            static MySerial serial{};

            ImGui::PushItemWidth(m_inputFieldWidth);
            if (numPorts > 0)
            {
                Combo("Port", &currentPort, portStrs, portStrs.size());
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Choose the port the laser is connected to");
                }

                ImGui::SameLine();
                if (ImGui::Button("Connect"))
                {
                    serial.Connect(
                            fmt::format("\\\\.\\{}", portStrs[currentPort]),
                            CBR_9600);

                    if (serial.IsConnected())
                    {
                        spdlog::info("Connected to {} successfully",
                                    portStrs[currentPort]);
                    }
                    else
                    {
                        spdlog::error("Couldn't connect to {}",
                                    portStrs[currentPort]);
                    }
                }
            }

            if (!serial.IsConnected())
            {
                ImGui::TextColored({0.7f, 0.f, 0.f, 1.f}, "Please connect the laser\n");
                ImGui::BeginDisabled();
            }

            static std::string toSend{};
            ImGui::InputTextWithHint("Send to serial port", nullptr, &toSend);
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Send string from 0 to 255 to change laser brightness");
            }
            if (ImGui::IsItemDeactivatedAfterEdit() && !toSend.empty())
            {
                spdlog::info("{}", serial.IsConnected());
                if (serial.WriteSerialPort(toSend))
                {
                    spdlog::info("Sent \"{}\"", toSend);
                }
                else
                {
                    spdlog::error("Couldn't send string");
                }
            }
            ImGui::PopItemWidth();

            if (!serial.IsConnected())
            {
                ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }
}// namespace prm