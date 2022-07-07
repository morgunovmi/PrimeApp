#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <pybind11/embed.h>
#include <spdlog/spdlog.h>

#include "workers/PythonWorker.h"

namespace py = pybind11;
using namespace py::literals;

namespace prm
{
    /**
     * Class that implements the NTA related image analysis functionality
     * Uses an embedded python interpreter from pybind
     * ALl methods just send python strings to an interpreter running in a different thread
     */
    class VideoProcessor
    {
    public:
        VideoProcessor(sf::Texture& texture, std::mutex& mutex)
            : m_messageQueue(),
              m_pythonWorker(1, m_messageQueue, texture, mutex)
        {
            m_pythonWorker.Run();
            Init();
        }

        /**
         * Loads the video from a given path
         *
         * @param path String with a video file path (Actually not video but a tif stack)
         */
        void LoadVideo(std::string_view path);

        // Here and below some of the parameters are for trackpy functions.
        // Browse their docs to find out more about them

        /**
         * Locates blobs on one tif frame
         * Used for parameter fine tuning
         *
         * @param frameNum Number of the frame to analyze
         * @param minm The minimum integrated brightness
         * @param ecc Max eccentricity
         * @param size Max radius of gyration of blob's Gaussian-like profile
         * @param diameter The feature’s extent in each dimension
         */
        void LocateOneFrame(int frameNum, int minm, double ecc, int size,
                            int diameter);

        /**
         * Locates blobs on all frames of the image sequence
         */
        void LocateAllFrames();

        /**
         * Links the blobs between frames, filters out small trajectories, subtracts ensemble drift
         *
         * @param searchRange The maximum distance features can move between frames
         * @param memory the maximum number of frames during which a feature can vanish, then reappear nearby, and be considered the same particle
         * @param minTrajectoryLen minimum number of points (video frames) to survive
         * @param driftSmoothing Smooth the drift using a forward-looking rolling mean over this many frames.
         */
        void LinkAndFilter(int searchRange, int memory, int minTrajectoryLen,
                           int driftSmoothing);

        /**
         * Groups the dataframe by particles and filters out outlying ones
         *
         * @param minDiagSize Min particle diagonal size to survive
         * @param maxDiagSize Max particle diagonal size to survive
         */
        void GroupAndPlotTrajectory(int minDiagSize, int maxDiagSize);

        /**
         * Plots the particle size distribution histogram and saves it to a file
         *
         * @param fps Framerate of the image sequence capture
         * @param scale Microns per pixel
         */
        void PlotSizeHist(double fps, double scale);

        /**
         * Computes the mean particle size in nanometers, prints the result to log
         *
         * @param fps Framerate of the image sequence capture
         * @param scale Microns per pixel
         */
        void GetSize(double fps, double scale);

        /**
         * Runs an arbitrary python query in the embedded interpreter
         *
         * @param query query string to run
         */
        void RunPythonQuery(std::string_view query);

        ~VideoProcessor()
        {
            spdlog::info("Killing video processor");
            m_messageQueue.Send(PythonWorkerQuit{});
        }

    private:
        /**
         * Initializes the python environment
         */
        void Init();

        /// Queue for messages for the worker thread
        MessageQueue<PythonWorkerMessage> m_messageQueue;

        /// Python worker that runs an embedded interpreter in a separate thread
        PythonWorker m_pythonWorker;

        /// Current vide file path
        std::filesystem::path vidPath;
    };
}// namespace prm