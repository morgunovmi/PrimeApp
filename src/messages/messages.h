#include <SFML/Graphics.hpp>

#include <vector>
#include <variant>

#include "imgui.h"

// Supervisor
struct OneImageResult {
    sf::Image image;
};

struct ImageVectorResult {
    std::vector<sf::Image> resultFrames;
};

using SupervisorMessage = std::variant<OneImageResult, ImageVectorResult>;

// Worker
struct WorkerLiveCapture {
    sf::Vector2u imageSize;
};

struct WorkerQuit{};

using WorkerMessage = std::variant<WorkerLiveCapture, WorkerQuit>;