#include "frontend/App.h"

namespace prm
{
    void App::Run()
    {
        m_gui.Init();
        while (m_window.isOpen())
        {
            m_dt = m_deltaClock.restart();
            m_gui.Update();
            m_renderer.Render();
            m_gui.Render();
            m_renderer.Display();
        }
        m_gui.Shutdown();
    }
}// namespace prm