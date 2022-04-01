#ifndef PRIME_APP_VIDEOPROCESSOR_H
#define PRIME_APP_VIDEOPROCESSOR_H

#include <pybind11/embed.h>
#include <SFML/Graphics.hpp>

#include "workers/PythonWorker.h"
#include "spdlog/spdlog.h"

namespace py = pybind11;
using namespace py::literals;

class VideoProcessor {
public:
    VideoProcessor(sf::Texture &texture, std::mutex& mutex) : mCurrentTexture(texture), mTextureMutex(mutex), mMessageQueue(), mPythonWorker(1, mMessageQueue) {
        mPythonWorker.Run();
        Init();
    }

    void LoadVideo(const std::string& path, const std::string& file);

    void LocateOneFrame(int frameNum, int minm, double ecc, int size, int diameter);

    void LocateAllFrames();

    void LinkAndFilter(int searchRange, int memory, int minTrajectoryLen, int driftSmoothing);

    void GroupAndPlotTrajectory(int minDiagSize, int maxDiagSize);

    ~VideoProcessor() { spdlog::info("Killing video processor"); mMessageQueue.Send(PythonWorkerQuit{}); }

private:
    void Init();

    sf::Texture& mCurrentTexture;
    std::mutex& mTextureMutex;

    MessageQueue<PythonWorkerMessage> mMessageQueue;

    PythonWorker mPythonWorker;
};

#endif //PRIME_APP_VIDEOPROCESSOR_H
