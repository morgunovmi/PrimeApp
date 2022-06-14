#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

namespace prm
{
    /// Acceleration on key press amount
    const float VIEW_ACCEL_AMOUNT = 25.f;
    /// Velocity damping amount
    const float VIEW_DAMP_FACTOR = 0.96f;
    /// Max view move speed
    const float VIEW_SPEED_CAP = 800.f;
    /// Factor for view zooming
    const float VIEW_ZOOM_FACTOR = 1.1f;

    /**
     * Class that defines what area to display in the SFML render window
     * Has functionality to move that area and zoom it
     */
    class ViewPort : public sf::View
    {
    public:
        ViewPort(float width, float height)
            : sf::View(sf::FloatRect(sf::Vector2f{0, 0},
                                     sf::Vector2f{width, height})),
              m_moveVelocity(), m_zoomSpeed(), m_zoomLevel(1.f)
        {
        }

        /**
         * Updates the view each frame
         *
         * @param dt Delta time in seconds last frame
         */
        void Update(float dt);

    private:
        /**
         * Helper function to calculate the length of a 2d vector
         *
         * @param vec 2d vector to calculate the length of
         * @return length of the vector
         */
        static float norm(const sf::Vector2f& vec)
        {
            return std::sqrt(vec.x * vec.x + vec.y * vec.y);
        }

        /// Current view move velocity
        sf::Vector2f m_moveVelocity;
        /// Current view zoom speed
        float m_zoomSpeed;
        /// Current view zoom level
        float m_zoomLevel;
    };
}// namespace prm