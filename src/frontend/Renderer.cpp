#include "frontend/Renderer.h"

namespace slr {
    void Renderer::Init() {
        mWindow.setFramerateLimit(60);
    }

    void Renderer::Render() {
        std::scoped_lock lock(mTextureMutex);

        mWindow.clear(sf::Color::Black);
        mWindow.draw(sf::Sprite(mCurrentTexture));
    }

    void Renderer::Display() {
        mWindow.display();
    }
}
