#include "frontend/Renderer.h"

namespace prm
{
    void Renderer::Render() { m_window.clear(sf::Color::Black); }

    void Renderer::Display() { m_window.display(); }
}// namespace prm