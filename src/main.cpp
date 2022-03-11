#include "frontend/App.h"
#include "backend/PhotometricsBackend.h"
#include "backend/OpencvBackend.h"

int main(int argc, char** argv) {
    const auto width = 1400;
    const auto height = 600;

    sf::ContextSettings settings{};
    settings.antialiasingLevel = 8;

    slr::App system{ argc, argv, width, height, settings };
    system.Run();

    return EXIT_SUCCESS;
}