#ifndef MESSAGES_H
#define MESSAGES_H

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

struct WorkerQuit {
};

using WorkerMessage = std::variant<WorkerLiveCapture, WorkerQuit>;

// PythonWorker

struct PythonWorkerEnvInit {
};

struct PythonWorkerVideoInit {
    std::string path;
    std::string file;
};

struct PythonWorkerQuit {
};

using PythonWorkerMessage = std::variant<PythonWorkerEnvInit, PythonWorkerVideoInit, PythonWorkerQuit>;

#endif