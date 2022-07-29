#include <OpenImageIO/imageio.h>
#include <opencv2/opencv.hpp>

#include "ImageViewer.h"

namespace prm
{
    bool ImageViewer::LoadImageStack(const std::string& filePath)
    {
        m_workerThread = std::jthread(
                [&, filePath]()
                {
                    using namespace OIIO;
                    spdlog::info("{}", filePath);
                    auto inp = ImageInput::open(filePath);
                    if (!inp) { return; }
                    const ImageSpec& spec = inp->spec();
                    m_imageWidth = spec.width;
                    m_imageHeight = spec.height;

                    m_numFrames = 1;
                    while (inp->seek_subimage(m_numFrames, 0))
                    {
                        ++m_numFrames;
                    }

                    spdlog::info("Num images: {}", m_numFrames);

                    m_pixels = std::vector<uint16_t>(
                            m_imageWidth * m_imageHeight * m_numFrames);

                    for (std::size_t i = 0;
                         i < m_numFrames && inp->seek_subimage(i, 0); ++i)
                    {
                        spdlog::info("Loading subimage {}", i);
                        inp->read_image(
                                TypeDesc::UINT16,
                                &m_pixels[i * m_imageWidth * m_imageHeight]);
                    }

                    //inp->read_image(TypeDesc::UINT16, &m_pixels[0]);
                    inp->close();

                    std::copy(m_pixels.begin(), m_pixels.end(),
                              std::back_inserter(m_modifiedPixels));

                    spdlog::info("Loading complete");

                    m_isImageLoaded = true;

                    SelectImage(m_currentFrame);
                });
        return true;
    }

    bool ImageViewer::TopHatFilter_(std::vector<uint16_t>& bytes,
                                    uint16_t width, uint16_t height,
                                    uint32_t nFrames, uint16_t filterSize)
    {
        if (!m_isImageLoaded) { return true; }
        const auto frameSizeU16 = width * height;
        for (std::size_t i = 0; i < nFrames; ++i)
        {
            cv::Mat mat{height, width, CV_16U, &(bytes[i * width * height])};
            cv::Mat element = cv::getStructuringElement(
                    cv::MORPH_ELLIPSE, cv::Size{filterSize, filterSize});
            cv::morphologyEx(mat, mat, cv::MORPH_TOPHAT, element,
                             cv::Point{-1, -1});

            std::copy((uint16_t*) mat.data, (uint16_t*) mat.data + frameSizeU16,
                      &(bytes[i * width * height]));
            spdlog::info("Processed frame {}", i);
        }
        return true;
    }

    void ImageViewer::SelectImage(std::size_t index)
    {
        if (!m_isImageLoaded) { return; }
        m_currentFrame = index;
        auto backend = dynamic_cast<PhotometricsBackend*>(m_backend.get());

        const auto beginIt = m_modifiedPixels.begin() +
                             m_currentFrame * m_imageWidth * m_imageHeight;
        const auto [itMin, itMax] = std::minmax_element(
                beginIt, beginIt + m_imageWidth * m_imageHeight);

        backend->m_minCurrentValue = *itMin;
        backend->m_maxCurrentValue = *itMax;

        UpdateImage();
    }

    void ImageViewer::UpdateImage()
    {
        if (!m_isImageLoaded) { return; }
        auto backend = dynamic_cast<PhotometricsBackend*>(m_backend.get());
        const auto image = MyImageToSfImage(
                m_modifiedPixels, m_currentFrame, m_imageWidth, m_imageHeight,
                backend->m_minDisplayValue, backend->m_maxDisplayValue);

        std::scoped_lock lock(m_textureMutex);
        m_currentTexture.loadFromImage(image);
    }

    sf::Image
    ImageViewer::MyImageToSfImage(const std::vector<uint16_t>& imageData,
                                  std::size_t idx, uint16_t imageWidth,
                                  uint16_t imageHeight, uint32_t minVal,
                                  uint32_t maxVal)
    {
        sf::Image image{};

        image.create(imageWidth, imageHeight);
        for (std::size_t y = 0; y < imageHeight; ++y)
        {
            for (std::size_t x = 0; x < imageWidth; ++x)
            {
                auto val = imageData[idx * imageWidth * imageHeight +
                                     y * imageWidth + x];
                val = std::clamp(val, (uint16_t) minVal, (uint16_t) maxVal);
                auto val8 =
                        static_cast<uint8_t>(static_cast<float>(val - minVal) /
                                             (maxVal - minVal + 1) * 256.f);
                image.setPixel(x, y, sf::Color{val8, val8, val8});
            }
        }
        return image;
    }

    bool ImageViewer::MedianFilter_(std::vector<uint16_t>& bytes,
                                    uint16_t width, uint16_t height,
                                    uint32_t nFrames, uint16_t filterSize)
    {
        if (!m_isImageLoaded) { return false; }
        const auto frameSizeU16 = width * height;
        for (std::size_t i = 0; i < nFrames; ++i)
        {
            cv::Mat mat{height, width, CV_16U, &(bytes[i * width * height])};

            cv::medianBlur(mat, mat, filterSize);

            std::copy((uint16_t*) mat.data, (uint16_t*) mat.data + frameSizeU16,
                      &(bytes[i * width * height]));
            spdlog::info("Processed frame {}", i);
        }
        return true;
    }
}// namespace prm