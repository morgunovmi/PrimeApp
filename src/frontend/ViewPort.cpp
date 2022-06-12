#include "frontend/ViewPort.h"

namespace prm {
    void ViewPort::Update(float dt) {
        sf::Vector2f acceleration{};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            acceleration.x -= VIEW_ACCEL_AMOUNT * mZoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            acceleration.x += VIEW_ACCEL_AMOUNT * mZoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            acceleration.y -= VIEW_ACCEL_AMOUNT * mZoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            acceleration.y += VIEW_ACCEL_AMOUNT * mZoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
            zoom(VIEW_ZOOM_FACTOR);
            mZoomLevel *= VIEW_ZOOM_FACTOR;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
            zoom(1.0f / VIEW_ZOOM_FACTOR);
            mZoomLevel /= VIEW_ZOOM_FACTOR;
        }

        mMoveVelocity += acceleration;

        const auto fracOfMaxSpeed = norm(mMoveVelocity) / (VIEW_SPEED_CAP * mZoomLevel);

        if (fracOfMaxSpeed > 1) {
            mMoveVelocity /= fracOfMaxSpeed;
        }
        move(mMoveVelocity * dt);

        mMoveVelocity *= VIEW_DAMP_FACTOR;
    }
}