#include "frontend/GUI.h"

#include "imgui-SFML.h"
#include "imgui.h"

#include <future>

namespace slr {
    bool GUI::Init() {
        ImGui::SFML::Init(mWindow);

        auto& io = ImGui::GetIO();
        mHubballiFont = io.Fonts->AddFontFromFileTTF("./resources/fonts/hubballi-regular.ttf", 20);
        ImGui::SFML::UpdateFontTexture();

        return true;
    }

    void GUI::PollEvents() {
        sf::Event event{};

        while (mWindow.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type) {
                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::LAlt:
                            mShowMainMenuBar = !mShowMainMenuBar;
                            break;
                        case sf::Keyboard::F1:
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

        mFrameTimeQueue.push(mDt.asMicroseconds());
        if (mFrameTimeQueue.size() > FRAME_QUEUE_SIZE) {
            mFrameTimeQueue.pop();
        }
    }

    void GUI::ShowFrameInfoOverlay() {
        static int corner = 1;
        auto& io = ImGui::GetIO();
        auto window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            const auto PAD = 10.0f;
            const auto* viewport = ImGui::GetMainViewport();
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
        if (ImGui::Begin("FrameInfoOverlay", &mShowFrameInfoOverlay, window_flags))
        {
            ImGui::Text("Frame info");
            ImGui::Separator();
            ImGui::Text("Frametime: %d ms\nFPS: %.3f", mDt.asMilliseconds(), 1.f / mDt.asSeconds());
            ImGui::PlotLines("Frame Times", &mFrameTimeQueue.front(), static_cast<int>(mFrameTimeQueue.size()),
                             0, nullptr, FLT_MAX, FLT_MAX, ImVec2{100, 40});

            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom",       nullptr, corner == -1)) corner = -1;
                if (ImGui::MenuItem("Top-left",     nullptr, corner == 0)) corner = 0;
                if (ImGui::MenuItem("Top-right",    nullptr, corner == 1)) corner = 1;
                if (ImGui::MenuItem("Bottom-left",  nullptr, corner == 2)) corner = 2;
                if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3)) corner = 3;
                if (ImGui::MenuItem("Close")) mShowFrameInfoOverlay = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void GUI::ShowMainMenuBar() {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Frame Info", "F1", &mShowFrameInfoOverlay)) {}
                if (ImGui::MenuItem("App Log", "F2", &mShowAppLog)) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void GUI::ShowViewport() {
        const auto* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(mWindow.getSize().x * 0.6, mWindow.getSize().y));

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

        const auto* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + mWindow.getSize().x * 0.6, main_viewport->WorkPos.y));
        ImGui::SetNextWindowSize(ImVec2(mWindow.getSize().x * 0.4, mWindow.getSize().y / 2));

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

    void GUI::ShowActionButtons() {
        const auto* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + mWindow.getSize().x * 0.6, main_viewport->WorkPos.y + mWindow.getSize().y / 2));
        ImGui::SetNextWindowSize(ImVec2(mWindow.getSize().x * 0.4, mWindow.getSize().y / 2 - main_viewport->WorkPos.y));

        auto window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        window_flags |= ImGuiWindowFlags_UnsavedDocument;

        if (ImGui::Begin("Action Buttons", nullptr, window_flags)) {
            if (ImGui::Button("Init") && !mIsInit) {
                if (std::async(&Backend::InitAndOpenOneCamera, &mBackend).get()) {
                    mAppLog.AddLog("Successful init\n");
                    mIsInit = true;
                } else {
                    mAppLog.AddLog("Failed to init\n");
                }
            }

            if (ImGui::Button("Live capture") && mIsInit && !mIsCapturing) {
                mIsCapturing = true;
                std::async(&Backend::LiveCapture, &mBackend);
            }

            if (ImGui::Button("Terminate capture") && mIsInit) {
                mBackend.TerminateCapture();
                mIsCapturing = false;
            }
        }

        ImGui::End();
    }

    void GUI::Render() {
        ImGui::PushFont(mHubballiFont);
        if (mShowMainMenuBar) ShowMainMenuBar();
        if (mShowFrameInfoOverlay) ShowFrameInfoOverlay();
        if (mShowAppLog) ShowAppLog();
        ShowActionButtons();

        ImGui::ShowDemoWindow();

        ImGui::PopFont();
        ImGui::SFML::Render(mWindow);
    }

    void GUI::Shutdown() {
        ImGui::SFML::Shutdown();
    }
}