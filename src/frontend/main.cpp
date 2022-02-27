#include "frontend/App.h"

int main() {
    const auto width = 800;
    const auto height = 600;

    sf::ContextSettings settings{};
    settings.antialiasingLevel = 8;

    slr::App system{ width, height, settings };
    system.Run();

    return EXIT_SUCCESS;
}