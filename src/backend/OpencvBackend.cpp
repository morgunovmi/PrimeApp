#include <spdlog/spdlog.h>
#include <OpenImageIO/imageio.h>

#include "OpencvBackend.h"

sf::Image prm::OpencvBackend::MatToImage(const cv::Mat& mat)
{
    sf::Image image;

    image.create(mat.size().width, mat.size().height);
    for (std::size_t x = 0; x < mat.size().width; ++x)
    {
        for (std::size_t y = 0; y < mat.size().height; ++y)
        {
            image.setPixel(x, y,
                           sf::Color{mat.at<cv::Vec3b>(y, x)[0],
                                     mat.at<cv::Vec3b>(y, x)[1],
                                     mat.at<cv::Vec3b>(y, x)[2]});
        }
    }
    return image;
}

void prm::OpencvBackend::Init()
{
    if (m_context.isCamOpen)
    {
        spdlog::info("Camera already initialized");
        return;
    }

    m_context.thread = std::make_unique<std::jthread>(
            &OpencvBackend::Init_, this, std::ref(m_context));
}

void prm::OpencvBackend::LiveCapture(SAVE_FORMAT format, bool save)
{
    if (!m_context.isCamOpen)
    {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (m_isCapturing)
    {
        spdlog::warn(
                "Already capturing, please stop current acquisition first");
        return;
    }

    m_isCapturing = true;
    m_context.thread = std::make_unique<std::jthread>(
            &OpencvBackend::Capture_, this, std::ref(m_context), format, -1, save);
}

void prm::OpencvBackend::SequenceCapture(uint32_t nFrames, SAVE_FORMAT format, bool save)
{
    if (!m_context.isCamOpen)
    {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (m_isCapturing)
    {
        spdlog::warn(
                "Already capturing, please stop current acquisition first");
        return;
    }

    m_isCapturing = true;
    m_context.thread = std::make_unique<std::jthread>(&OpencvBackend::Capture_,
                                                      this, std::ref(m_context),
                                                      format, nFrames, save);
}

void prm::OpencvBackend::TerminateCapture() { m_isCapturing = false; }

void prm::OpencvBackend::Init_(prm::OpencvCameraCtx& ctx)
{
    spdlog::info("Opening camera");

    ctx.camera = std::make_unique<cv::VideoCapture>(0);
    if (!ctx.camera->isOpened())
    {
        PrintError("ERROR: Could not open camera");
        return;
    }

    ctx.isCamOpen = true;
    spdlog::info("Successful camera init");
}

//TODO: Refactor repeating blocks of code
void prm::OpencvBackend::Capture_(OpencvCameraCtx& ctx, SAVE_FORMAT format,
                                  int32_t nFrames, bool save)
{
    std::string videoPath{};
    if (nFrames <= 0)
    {
        videoPath = FileUtils::GenerateVideoPath(m_saveDirPath, LIVE_CAPTURE_PREFIX, format);
    }
    else
    {
        videoPath = FileUtils::GenerateVideoPath(m_saveDirPath, SEQ_CAPTURE_PREFIX, format);
    }

    if (videoPath.empty()) {
        spdlog::error("Couldn't generate videopath");
        return;
    }

    const int xres = ctx.camera->get(cv::CAP_PROP_FRAME_WIDTH);
    const int yres = ctx.camera->get(cv::CAP_PROP_FRAME_HEIGHT);
    const int channels =
            ctx.camera->get(cv::CAP_PROP_XI_IMAGE_IS_COLOR) ? 3 : 1;

    const int singleFrameSize = xres * yres * channels;

    std::vector<std::uint8_t> pixels;

    auto counter = 0;
    while (m_isCapturing)
    {
        // Condition to quit sequence capture
        if (nFrames > 0 && counter >= nFrames)
        {
            m_isCapturing = false;
            break;
        }
        ++counter;

        cv::Mat frame;
        *ctx.camera >> frame;
        if (frame.empty())
        {
            spdlog::error("Got empty frame. Stopping capture...");
            m_isCapturing = false;
            ctx.camera->release();
            ctx.isCamOpen = false;
            break;
        }
        spdlog::debug("Frame no {}", counter);

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        if (save)
        {
            std::copy(frame.data, frame.data + singleFrameSize,
                      std::back_inserter(pixels));
        }

        const auto image = MatToImage(frame);

        std::scoped_lock lock(m_textureMutex);
        m_currentTexture.loadFromImage(image);
        std::this_thread::sleep_for(std::chrono::milliseconds(
                static_cast<uint64_t>(1.f / static_cast<float>(ctx.framerate)) *
                1000));
    }
    ctx.camera->release();
    m_isCapturing = false;
    ctx.isCamOpen = false;

    if (save)
    {
        switch (format)
        {
            case TIF:
            {
                using namespace OIIO;

                std::unique_ptr<ImageOutput> out =
                        ImageOutput::create(videoPath);
                if (!out) return;
                ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);

                if (!out->supports("multiimage") ||
                    !out->supports("appendsubimage"))
                {
                    spdlog::error(
                            "Current plugin doesn't support tif subimages");
                    return;
                }

                ImageOutput::OpenMode appendmode = ImageOutput::Create;

                for (int s = 0; s < counter; ++s)
                {
                    out->open(videoPath, spec, appendmode);
                    out->write_image(TypeDesc::UINT8,
                                     pixels.data() + singleFrameSize * s);
                    appendmode = ImageOutput::AppendSubimage;
                }
                break;
            }

            case MP4:
            {
                cv::VideoWriter writer{
                        videoPath, cv::VideoWriter::fourcc('X', '2', '6', '4'),
                        static_cast<double>(ctx.framerate),
                        cv::Size{xres, yres}};

                for (int s = 0; s < counter; ++s)
                {
                    auto mat = cv::Mat(yres, xres, CV_8UC3,
                                       pixels.data() + singleFrameSize * s);
                    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
                    writer.write(mat);
                }
                writer.release();
                break;
            }
            default:
                spdlog::error("Undefined format");
        }
        spdlog::info("File written to {}", videoPath);
    }
}