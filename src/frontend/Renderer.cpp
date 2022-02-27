#include "frontend/Renderer.h"

namespace slr {
    void Renderer::Init() {
        mWindow.setFramerateLimit(60);
    }

    void Renderer::Render() {
        mWindow.clear(sf::Color::Black);
    }

    void Renderer::Display() {
        mWindow.display();
    }
}
