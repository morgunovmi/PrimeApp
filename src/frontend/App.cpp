#include "frontend/App.h"

namespace slr {
    void App::Run() {
        mGUI.Init();

        mVideoProc.LoadVideo(R"(C:\Users\Max\Desktop\Samples\)", R"(a1.tif)");
        mVideoProc.LocateOneFrame(10, 1000, 0.5, 5, 19);
        mVideoProc.LocateAllFrames();
        mVideoProc.LinkAndFilter(7, 10, 5, 10);
        mVideoProc.GroupAndPlotTrajectory(5, 30);

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