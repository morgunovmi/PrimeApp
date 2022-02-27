#include "backend/Backend.h"

#include "imgui-SFML.h"

#include "master.h"
#include "pvcam.h"

#include <iostream>


namespace slr {
    bool Backend::Init() {
        std::scoped_lock lock(mPvcamMutex);

        if (pl_pvcam_init() != PV_OK) {
            mAppLog.AddLog("Failed to init cameras\n");
            return false;
        }
        return true;
    }

    bool Backend::EnumerateCameras() {
        std::scoped_lock lock(mPvcamMutex);

        int16 nrOfCameras = 0;
        if (PV_OK != pl_cam_get_total(&nrOfCameras)) {
            mAppLog.AddLog("Error getting number of cameras\n");
            return false;
        }
        mAppLog.AddLog("Found %d cameras\n", nrOfCameras);

        char camName[CAM_NAME_LEN];
        for (int16 n = 0; n < nrOfCameras; ++n) {
            if (PV_OK != pl_cam_get_name(n, camName)) {

                mAppLog.AddLog("Error getting name of %dth camera\n", n);
                return false;
            }
            mAppLog.AddLog("Camera %d: %s", n, camName);
        }
        return true;
    }

    bool Backend::Uninit() {
        std::scoped_lock lock(mPvcamMutex);

        if (pl_pvcam_uninit() != PV_OK) {
            mAppLog.AddLog("Failed to uninit cameras\n");
            return false;
        }
        return true;
    }

    void Backend::Update() {
        mDt = mDeltaClock.restart();
        PollInput();
    }

    void Backend::PollInput() {
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
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}
