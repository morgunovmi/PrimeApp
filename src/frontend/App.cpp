#include "frontend/App.h"

namespace slr {
    void App::Run() {
        mGUI.Init();
        mVideoProc.Test();
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