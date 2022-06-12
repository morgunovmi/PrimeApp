#include "frontend/ViewPort.h"

namespace prm
{
    void ViewPort::Update(float dt)
    {
        sf::Vector2f acceleration{};

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            acceleration.x -= VIEW_ACCEL_AMOUNT * m_zoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            acceleration.x += VIEW_ACCEL_AMOUNT * m_zoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            acceleration.y -= VIEW_ACCEL_AMOUNT * m_zoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            acceleration.y += VIEW_ACCEL_AMOUNT * m_zoomLevel;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
        {
            zoom(VIEW_ZOOM_FACTOR);
            m_zoomLevel *= VIEW_ZOOM_FACTOR;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            zoom(1.0f / VIEW_ZOOM_FACTOR);
            m_zoomLevel /= VIEW_ZOOM_FACTOR;
        }

        m_moveVelocity += acceleration;

        const auto fracOfMaxSpeed =
                norm(m_moveVelocity) / (VIEW_SPEED_CAP * m_zoomLevel);

        if (fracOfMaxSpeed > 1) { m_moveVelocity /= fracOfMaxSpeed; }
        move(m_moveVelocity * dt);

        m_moveVelocity *= VIEW_DAMP_FACTOR;
    }
}// namespace prm