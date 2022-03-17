#include "frontend/Renderer.h"

namespace slr {
    void Renderer::Init() {
        mWindow.setFramerateLimit(60);
    }

    void Renderer::Render() {
        mWindow.clear(sf::Color::Black);

        std::scoped_lock lock(mTextureMutex);
        mWindow.draw(sf::Sprite(mCurrentTexture));
    }

    void Renderer::Display() {
        mWindow.display();
    }
}