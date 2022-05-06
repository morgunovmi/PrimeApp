#ifndef PRIME_APP_VIDEOPROCESSOR_H
#define PRIME_APP_VIDEOPROCESSOR_H

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <pybind11/embed.h>
#include <spdlog/spdlog.h>

#include "workers/PythonWorker.h"

namespace py = pybind11;
using namespace py::literals;

namespace prm
{
    class VideoProcessor
    {
    public:
        VideoProcessor(sf::Texture& texture, std::mutex& mutex)
            : m_currentTexture(texture), m_textureMutex(mutex),
              m_messageQueue(), m_pythonWorker(1, m_messageQueue)
        {
            m_pythonWorker.Run();
            Init();
        }

        void LoadVideo(std::string_view path);

        void LocateOneFrame(int frameNum, int minm, double ecc, int size,
                            int diameter);

        void LocateAllFrames();

        void LinkAndFilter(int searchRange, int memory, int minTrajectoryLen,
                           int driftSmoothing);

        void GroupAndPlotTrajectory(int minDiagSize, int maxDiagSize);

        void PlotSizeHist(double fps, double scale);

        void GetSize(double fps, double scale);

        void RunPythonQuery(std::string_view query);

        ~VideoProcessor()
        {
            spdlog::info("Killing video processor");
            m_messageQueue.Send(PythonWorkerQuit{});
        }

    private:
        void Init();

        sf::Texture& m_currentTexture;
        std::mutex& m_textureMutex;

        MessageQueue<PythonWorkerMessage> m_messageQueue;

        PythonWorker m_pythonWorker;

        std::filesystem::path vidPath;
    };
}// namespace prm

#endif//PRIME_APP_VIDEOPROCESSOR_H
