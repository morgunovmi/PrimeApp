#pragma once

#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>

#include "Backend.h"
#include "PhotometricsBackend.h"

namespace prm
{
    class ImageViewer
    {
    public:
        ImageViewer(std::unique_ptr<Backend>& backend, sf::Texture& texture,
                    std::mutex& mutex)
            : m_pixels(), m_currentFrame(0), m_backend(backend),
              m_currentTexture(texture), m_textureMutex(mutex)
        {
        }

        bool LoadImageStack(std::string_view filePath);

        void SelectImage(std::size_t index);

        void UpdateImage();

        bool SubtractBackground()
        {
            m_workerThread = std::jthread(
                    [&]()
                    {
                        SubtractBackground_(m_pixels.data(), m_imageWidth,
                                            m_imageHeight, 1);
                        UpdateImage();
                    });
            return true;
        }

        static sf::Image MyImageToSfImage(std::vector<uint16_t> imageData,
                                          uint16_t imageWidth,
                                          uint16_t imageHeight, uint32_t minVal,
                                          uint32_t maxVal);

    private:
        bool SubtractBackground_(uint16_t* bytes, uint16_t width,
                                 uint16_t height, uint32_t nFrames);
        std::vector<uint16_t> m_pixels;
        uint16_t m_imageWidth;
        uint16_t m_imageHeight;
        std::size_t m_numFrames;
        std::size_t m_currentFrame;

        std::unique_ptr<Backend>& m_backend;
        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;

        std::jthread m_workerThread;
    };
}// namespace prm