#include "frontend/App.h"

namespace slr {
    void App::Run() {
        mGUI.Init();

        while (mWindow.isOpen()) {
            mDt = mDeltaClock.restart();
            mGUI.Update();
            mRenderer.Render();
            mGUI.Render();
            mRenderer.Display();
        }
        mGUI.Shutdown();
    }
}