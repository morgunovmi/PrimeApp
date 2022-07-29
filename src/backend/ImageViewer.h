#pragma once

#include <SFML/Graphics.hpp>
#include <mutex>
#include <thread>

#include "Backend.h"
#include "PhotometricsBackend.h"

namespace prm
{
    class ImageViewer
    {
    public:
        ImageViewer(std::unique_ptr<Backend>& backend, sf::Texture& texture,
                    std::mutex& mutex)
            : m_pixels(), m_modifiedPixels(), m_isImageLoaded(false),
              m_currentFrame(0), m_backend(backend), m_currentTexture(texture),
              m_textureMutex(mutex)
        {
        }

        bool LoadImageStack(const std::string& filePath);

        void SelectImage(std::size_t index);

        void UpdateImage();

        bool TopHatFilter(uint16_t filterSize, bool allFrames)
        {
            m_workerThread = std::jthread(
                    [&, filterSize, allFrames]()
                    {
                        TopHatFilter_(m_modifiedPixels, m_imageWidth,
                                      m_imageHeight,
                                      allFrames ? m_numFrames : 1, filterSize);
                        UpdateImage();
                    });
            return true;
        }

        bool MedianFilter(uint16_t filterSize, bool allFrames)
        {
            m_workerThread = std::jthread(
                    [&, filterSize, allFrames]()
                    {
                        MedianFilter_(m_modifiedPixels, m_imageWidth,
                                      m_imageHeight,
                                      allFrames ? m_numFrames : 1, filterSize);
                        UpdateImage();
                    });
            return true;
        }

        void ResetImage()
        {
            m_workerThread = std::jthread(
                    [&]()
                    {
                        m_modifiedPixels.clear();
                        std::copy(m_pixels.begin(), m_pixels.end(),
                                  std::back_inserter(m_modifiedPixels));
                        UpdateImage();
                    });
        }

        std::size_t GetNumImages() const { return m_numFrames; }

        bool m_isImageLoaded;

    private:
        static sf::Image
        MyImageToSfImage(const std::vector<uint16_t>& imageData,
                         std::size_t idx, uint16_t imageWidth,
                         uint16_t imageHeight, uint32_t minVal,
                         uint32_t maxVal);

        bool TopHatFilter_(std::vector<uint16_t>& bytes, uint16_t width,
                           uint16_t height, uint32_t nFrames,
                           uint16_t filterSize);

        bool MedianFilter_(std::vector<uint16_t>& bytes, uint16_t width,
                           uint16_t height, uint32_t nFrames,
                           uint16_t filterSize);

        std::vector<uint16_t> m_pixels;
        std::vector<uint16_t> m_modifiedPixels;

        uint16_t m_imageWidth;
        uint16_t m_imageHeight;
        uint16_t m_numFrames;
        std::size_t m_currentFrame;

        std::unique_ptr<Backend>& m_backend;
        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;

        std::jthread m_workerThread;
    };
}// namespace prm