#include "frontend/Renderer.h"

namespace prm
{
    void Renderer::Init() { m_window.setFramerateLimit(60); }

    void Renderer::Render()
    {
        m_window.clear(sf::Color::Black);

        std::scoped_lock lock(m_textureMutex);
        m_window.draw(sf::Sprite(m_currentTexture));
    }

    void Renderer::Display() { m_window.display(); }
}// namespace prm