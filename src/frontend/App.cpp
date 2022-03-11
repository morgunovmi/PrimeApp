#include "frontend/App.h"

namespace slr {
    void App::Run() {
        mGUI.Init();
        while (mWindow.isOpen()) {
            mBackend.Update();
            mGUI.Update();
            mRenderer.Render();
            mGUI.Render();
            mRenderer.Display();
        }
        mGUI.Shutdown();
    }
}