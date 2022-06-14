#include "frontend/Renderer.h"

namespace prm
{
    void Renderer::Render()
    {
        m_viewport.Update(m_dt.asSeconds());
        m_window.clear(sf::Color::Black);

        std::scoped_lock lock(m_textureMutex);
        m_window.draw(sf::Sprite(m_currentTexture));
        m_window.setView(m_viewport);
    }

    void Renderer::Display() { m_window.display(); }
}// namespace prm