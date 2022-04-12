#include "frontend/GUI.h"

#include "imgui-SFML.h"
#include "imgui.h"
#include "imgui_stdlib.h"

//TODO Disabled blocks
namespace slr {
    bool GUI::Init() {
        ImGui::SFML::Init(mWindow);

        auto &io = ImGui::GetIO();
        mHubballiFont = io.Fonts->AddFontFromFileTTF("./resources/fonts/hubballi-regular.ttf", 20);
        ImGui::SFML::UpdateFontTexture();

        return true;
    }

    void GUI::PollEvents() {
        sf::Event event{};

        while (mWindow.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type) {
                case sf::Event::Closed:
                    mWindow.close();
                    break;
                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            mWindow.close();
                            break;
                        case sf::Keyboard::LAlt:
                            mShowMainMenuBar = !mShowMainMenuBar;
                            break;
                        case sf::Keyboard::F1:
                            mShowVideoProcessor = !mShowVideoProcessor;
                            break;
                        case sf::Keyboard::F2:
                            mShowFrameInfoOverlay = !mShowFrameInfoOverlay;
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

    void GUI::Update() {
        PollEvents();
        ImGui::SFML::Update(mWindow, mDt);

        mFrameTimeQueue.push(static_cast<float>(mDt.asMicroseconds()));
        if (mFrameTimeQueue.size() > FRAME_QUEUE_SIZE) {
            mFrameTimeQueue.pop();
        }
    }

    void GUI::ShowFrameInfoOverlay() {
        static int corner = 1;
        auto window_flags =
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1) {
            const auto PAD = 10.0f;
            const auto *viewport = ImGui::GetMainViewport();
            auto work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            auto work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("FrameInfoOverlay", &mShowFrameInfoOverlay, window_flags)) {
            ImGui::Text("Frame info");
            ImGui::Separator();
            ImGui::Text("Frametime: %d ms\nFPS: %.3f", mDt.asMilliseconds(), 1.f / mDt.asSeconds());
            ImGui::PlotLines("Frame Times", &mFrameTimeQueue.front(), static_cast<int>(mFrameTimeQueue.size()),
                             0, nullptr, FLT_MAX, FLT_MAX, ImVec2{100, 40});

            if (ImGui::BeginPopupContextWindow()) {
                if (ImGui::MenuItem("Custom", nullptr, corner == -1)) corner = -1;
                if (ImGui::MenuItem("Top-left", nullptr, corner == 0)) corner = 0;
                if (ImGui::MenuItem("Top-right", nullptr, corner == 1)) corner = 1;
                if (ImGui::MenuItem("Bottom-left", nullptr, corner == 2)) corner = 2;
                if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3)) corner = 3;
                if (ImGui::MenuItem("Close")) mShowFrameInfoOverlay = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void GUI::ShowMainMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Windows")) {
                if (ImGui::MenuItem("Video Processor", "F1", &mShowVideoProcessor)) {}
                if (ImGui::MenuItem("Frame Info", "F2", &mShowFrameInfoOverlay)) {}
                if (ImGui::MenuItem("App Log", nullptr, &mShowAppLog)) {}
                ImGui::EndMenu();
            }

            auto oldOpt = mCurrBackend;
            if (ImGui::BeginMenu("Backend")) {
                ImGui::RadioButton("OpenCV", (int *) &mCurrBackend, 0);
                ImGui::RadioButton("PVCam", (int *) &mCurrBackend, 1);
                ImGui::EndMenu();
            }

            if (oldOpt != mCurrBackend) {
                switch (mCurrBackend) {
                    case OPENCV:
                        mBackend = std::make_unique<OpencvBackend>(mBackend);
                        break;
                    case PVCAM:
                        mBackend = std::make_unique<PhotometricsBackend>(mBackend);
                        break;
                }
            }
            ImGui::EndMainMenuBar();
        }
    }

    void GUI::ShowViewport() {
        const auto *main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(mWindow.getSize().x) * 0.6f,
                                        static_cast<float>(mWindow.getSize().y)));

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    }

    void GUI::ShowAppLog() {
        // For the demo: add a debug button _BEFORE_ the normal log window contents
        // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
        // Most of the contents of the window will be added by the log.Draw() call.

        const auto *main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
                ImVec2(static_cast<float>(main_viewport->WorkPos.x) + static_cast<float>(mWindow.getSize().x) * 0.6f,
                       main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(mWindow.getSize().x) * 0.4f, static_cast<float>(mWindow.getSize().y) / 2.f));

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("App Log", nullptr, window_flags);
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        mAppLog.Draw("App Log", &mShowAppLog);
    }

    void GUI::ShowCameraButtons() {
        const auto *main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
                ImVec2(static_cast<float>(main_viewport->WorkPos.x) + static_cast<float>(mWindow.getSize().x) * 0.6f,
                       main_viewport->WorkPos.y + static_cast<float>(mWindow.getSize().y) / 2.f));
        ImGui::SetNextWindowSize(ImVec2(static_cast<float>(mWindow.getSize().x) * 0.4f,
                                        static_cast<float>(mWindow.getSize().y) / 2.f - main_viewport->WorkPos.y));

        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("Camera Buttons", nullptr, window_flags)) {
            if (ImGui::Button("Init") && !mIsInit) {
                mBackend->Init();
//                mIsInit = true;
            }

            if (ImGui::Button("Live capture")/* && mIsInit && !mIsCapturing*/) {
//                mIsCapturing = true;
                mBackend->LiveCapture();
            }

            static int nFrames = 100;
            if (ImGui::Button("Sequence capture")/* && mIsInit && !mIsCapturing*/) {
                //                mIsCapturing = true;
                mBackend->SequenceCapture(nFrames);
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(mInputFieldWidth);
            if (ImGui::SliderInt("Number of frames", &nFrames, 0, 1000)) {}
            ImGui::PopItemWidth();

            if (ImGui::Button("Terminate capture") /* && mIsInit && mIsCapturing*/) {
                mBackend->TerminateCapture();
//                mIsCapturing = false;
            }
        }

        ImGui::End();
    }

    void GUI::Render() {
        ImGui::PushFont(mHubballiFont);
        ShowCameraButtons();
        if (mShowMainMenuBar) ShowMainMenuBar();
        if (mShowFrameInfoOverlay) ShowFrameInfoOverlay();
        if (mShowVideoProcessor) ShowVideoProcessor();
        if (mShowAppLog) ShowAppLog();

        ImGui::ShowDemoWindow();

        ImGui::PopFont();
        ImGui::SFML::Render(mWindow);
    }

    void GUI::Shutdown() {
        ImGui::SFML::Shutdown();
    }

    void GUI::ShowVideoProcessor() {
        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize;

        if (ImGui::Begin("Video Processor", nullptr, window_flags)) {
            static std::string videoPath{};
            ImGui::InputTextWithHint("Video path", ".tif file path", &videoPath);

            if (ImGui::Button("Load Video")) {
                mVideoProcessor.LoadVideo(videoPath);
            }

            static int frameNum = 0;
            static int minMass = 1000;
            static double ecc = 0.5;
            static int size = 5;
            static int diameter = 19;

            ImGui::PushItemWidth(mInputFieldWidth);
            ImGui::InputInt("frame num", &frameNum);
            ImGui::InputInt("min mass", &minMass);
            ImGui::InputInt("size", &size);
            ImGui::InputInt("diameter", &diameter);
            ImGui::InputDouble("eccentricity", &ecc, 0.0, 0.0, "%.2f");
            ImGui::PopItemWidth();

            if (ImGui::Button("Locate on one frame")) {
                mVideoProcessor.LocateOneFrame(frameNum, minMass, ecc, size, diameter);
            }

            static int searchRange = 7;
            static int memory = 10;
            static int minTrajLen = 5;
            static int driftSmoothing = 10;

            static int minDiagSize = 5;
            static int maxDiagSize = 30;
            ImGui::PushItemWidth(mInputFieldWidth);
            ImGui::InputInt("search range", &searchRange);
            ImGui::SameLine();
            ImGui::InputInt("memory", &memory);
            ImGui::InputInt("min traj. len", &minTrajLen);
            ImGui::SameLine();
            ImGui::InputInt("drift smooth.", &driftSmoothing);
            ImGui::InputInt("min diag. size", &minDiagSize);
            ImGui::SameLine();
            ImGui::InputInt("max diag. size", &maxDiagSize);
            ImGui::PopItemWidth();

            if (ImGui::Button("Find trajectories")) {
                mVideoProcessor.LocateAllFrames();
                mVideoProcessor.LinkAndFilter(7, 10, 5, 10);
                mVideoProcessor.GroupAndPlotTrajectory(5, 30);
            }
        }

        ImGui::End();
    }
}