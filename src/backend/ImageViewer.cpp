#include <OpenImageIO/imageio.h>
#include <opencv2/opencv.hpp>

#include "ImageViewer.h"

namespace prm
{
    bool ImageViewer::LoadImageStack(std::string_view filePath)
    {
        using namespace OIIO;
        auto inp = ImageInput::open(filePath.data());
        if (!inp) { return false; }
        const ImageSpec& spec = inp->spec();
        m_imageWidth = spec.width;
        m_imageHeight = spec.height;
        int channels = spec.nchannels;
        m_pixels =
                std::vector<uint16_t>(m_imageWidth * m_imageHeight * channels);

        inp->read_image(TypeDesc::UINT16, &m_pixels[0]);
        inp->close();

        SelectImage(m_currentFrame);

        return true;
    }

    bool ImageViewer::SubtractBackground_(uint16_t* bytes, uint16_t width,
                                         uint16_t height, uint32_t nFrames)
    {
        const auto frameSizeU16 = width * height;
        for (std::size_t i = 0; i < nFrames; ++i)
        {
            cv::Mat mat{height, width, CV_16U,
                        bytes + i * frameSizeU16};
            cv::imshow("before", mat);
            cv::waitKey(0);
            cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                        cv::Size{15, 15});
            cv::morphologyEx(mat, mat, cv::MORPH_TOPHAT, element,
                             cv::Point{-1, -1});
            cv::imshow("after", mat);
            cv::waitKey(0);
            /*
            cv::imshow("before", mat);
            cv::waitKey(0);
            cv::medianBlur(mat, mat, 5);
            cv::imshow("after", mat);
            cv::waitKey(0);
             */

            std::copy(mat.data, mat.data + frameSizeU16,
                      bytes + i * frameSizeU16);
        }
        return true;
    }

    void ImageViewer::SelectImage(std::size_t index)
    {
        m_currentFrame = index;
        auto backend = dynamic_cast<PhotometricsBackend*>(m_backend.get());

        const auto beginIt = m_pixels.begin() +
                             m_currentFrame * m_imageWidth * m_imageHeight;
        const auto [itMin, itMax] = std::minmax_element(
                beginIt, beginIt + m_imageWidth * m_imageHeight);

        backend->m_minCurrentValue = *itMin;
        backend->m_maxCurrentValue = *itMax;

        UpdateImage();
    }

    void ImageViewer::UpdateImage()
    {
        auto backend = dynamic_cast<PhotometricsBackend*>(m_backend.get());
        const auto image = MyImageToSfImage(
                m_pixels, m_imageWidth, m_imageHeight,
                backend->m_minDisplayValue, backend->m_maxDisplayValue);

        std::scoped_lock lock(m_textureMutex);
        m_currentTexture.loadFromImage(image);
    }

    sf::Image ImageViewer::MyImageToSfImage(std::vector<uint16_t> imageData,
                                            uint16_t imageWidth,
                                            uint16_t imageHeight,
                                            uint32_t minVal, uint32_t maxVal)
    {
        sf::Image image{};

        image.create(imageWidth, imageHeight);
        for (std::size_t y = 0; y < imageHeight; ++y)
        {
            for (std::size_t x = 0; x < imageWidth; ++x)
            {
                auto val = imageData[y * imageWidth + x];
                val = std::clamp(val, (uint16_t) minVal, (uint16_t) maxVal);
                auto val8 =
                        static_cast<uint8_t>(static_cast<float>(val - minVal) /
                                             (maxVal - minVal + 1) * 256.f);
                image.setPixel(x, y, sf::Color{val8, val8, val8});
            }
        }
        return image;
    }
}// namespace prm