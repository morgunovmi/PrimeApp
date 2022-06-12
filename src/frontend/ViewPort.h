#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

namespace prm
{
    const float VIEW_ACCEL_AMOUNT = 25.f;
    const float VIEW_DAMP_FACTOR = 0.96f;
    const float VIEW_SPEED_CAP = 800.f;
    const float VIEW_ZOOM_FACTOR = 1.1f;

    class ViewPort : public sf::View
    {
    public:
        ViewPort(float width, float height)
            : sf::View(sf::FloatRect(sf::Vector2f{0, 0},
                                     sf::Vector2f{width, height})),
              mMoveVelocity(), mZoomSpeed(), mZoomLevel(1.f)
        {
        }

        void Update(float dt);

        float GetZoomLevel() const { return mZoomLevel; }

        const sf::Vector2f& GetMoveVelocity() const { return mMoveVelocity; }

        float GetZoomSpeed() const { return mZoomSpeed; }

        void SetMoveVelocity(const sf::Vector2f& moveVelocity)
        {
            mMoveVelocity = moveVelocity;
        }

        void SetZoomSpeed(float zoomSpeed)
        {
            mZoomSpeed = zoomSpeed;
        }

        void SetZoomLevel(float zoomLevel)
        {
            mZoomLevel = zoomLevel;
        }

    private:

        static float norm(const sf::Vector2f& vec) {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y);
        }

        sf::Vector2f mMoveVelocity;
        float mZoomSpeed;
        float mZoomLevel;
    };
}// namespace prm