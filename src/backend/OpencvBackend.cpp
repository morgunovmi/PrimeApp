#include "OpencvBackend.h"
#include "spdlog/spdlog.h"
#include <OpenImageIO/imageio.h>

sf::Image slr::OpencvBackend::MatToImage(const cv::Mat &mat) {
    sf::Image image;

    image.create(mat.size().width, mat.size().height);
    for (std::size_t x = 0; x < mat.size().width; ++x) {
        for (std::size_t y = 0; y < mat.size().height; ++y) {
            image.setPixel(x, y, sf::Color{mat.at<cv::Vec3b>(y, x)[0],
                                           mat.at<cv::Vec3b>(y, x)[1],
                                           mat.at<cv::Vec3b>(y, x)[2]});
        }
    }

    return image;
}

void slr::OpencvBackend::Init() {
    if (mContext.isCamOpen) {
        spdlog::info("Camera already initialized");
        return;
    }

    mContext.thread = std::make_unique<std::jthread>(&OpencvBackend::Init_, this, std::ref(mContext));
}

void slr::OpencvBackend::LiveCapture(CAP_FORMAT format) {
    if (!mContext.isCamOpen) {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (mContext.isCapturing) {
        spdlog::warn("Already capturing, please stop current acquisition first");
        return;
    }

    mContext.isCapturing = true;
    mContext.thread = std::make_unique<std::jthread>(&OpencvBackend::Capture_, this, std::ref(mContext), format, -1);
}

void slr::OpencvBackend::SequenceCapture(uint32_t nFrames, CAP_FORMAT format) {
    if (!mContext.isCamOpen) {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (mContext.isCapturing) {
        spdlog::warn("Already capturing, please stop current acquisition first");
        return;
    }

    mContext.isCapturing = true;
    mContext.thread = std::make_unique<std::jthread>(&OpencvBackend::Capture_, this, std::ref(mContext),
                                                     format, nFrames);
}

void slr::OpencvBackend::TerminateCapture() {
    mContext.isCapturing = false;
}

void slr::OpencvBackend::Init_(slr::OpencvCameraCtx &ctx) {
    spdlog::info("Opening camera");
    ctx.camera = std::make_unique<cv::VideoCapture>(0);

    if (!ctx.camera->isOpened()) {
        PrintError("ERROR: Could not open camera");
        return;
    }

    ctx.isCamOpen = true;
    spdlog::info("Successful camera init");
}

//TODO: Refactor repeating blocks of code
void slr::OpencvBackend::Capture_(OpencvCameraCtx &ctx, CAP_FORMAT format, int16_t nFrames) {
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H-%M-%S");
    const auto curTime = oss.str();

    auto videoPath = curTime;
    if (nFrames <= 0) {
        videoPath = fmt::format("{}{}", LIVE_CAPTURE_PREFIX, videoPath);
    } else {
        videoPath = fmt::format("{}{}", SEQ_CAPTURE_PREFIX, videoPath);
    }

    switch (format) {
        case TIF:
            videoPath.append(".tif");
            break;
        case MP4:
            videoPath.append(".mp4");
            break;
        default:
            spdlog::error("Undefined format");
    }

    const int xres = ctx.camera->get(cv::CAP_PROP_FRAME_WIDTH);
    const int yres = ctx.camera->get(cv::CAP_PROP_FRAME_HEIGHT);
    const int channels = ctx.camera->get(cv::CAP_PROP_XI_IMAGE_IS_COLOR) ? 3 : 1;

    const int singleFrameSize = xres * yres * channels;

    std::vector <std::uint8_t> pixels;

    auto counter = 0;
    while (ctx.isCapturing) {
        // Condition to quit sequence capture
        if (nFrames > 0 && counter >= nFrames) {
            ctx.isCapturing = false;
            break;
        }

        ++counter;

        cv::Mat frame;

        *ctx.camera >> frame;
        if (frame.empty()) {
            spdlog::error("Got empty frame. Stopping capture...");
            ctx.isCapturing = false;
            ctx.camera->release();
            ctx.isCamOpen = false;
            break;
        }

        spdlog::info("Frame no {}", counter);

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        std::copy(frame.data, frame.data + singleFrameSize, std::back_inserter(pixels));

        const auto image = MatToImage(frame);

        std::scoped_lock lock(mTextureMutex);
        mCurrentTexture.loadFromImage(image);
        std::this_thread::sleep_for(
                std::chrono::milliseconds(
                        static_cast<uint64_t>(1.f / static_cast<float>(ctx.framerate)) * 1000));
    }
    ctx.camera->release();
    ctx.isCapturing = false;
    ctx.isCamOpen = false;

    switch (format) {
        case TIF: {
            using namespace OIIO;

            std::unique_ptr <ImageOutput> out = ImageOutput::create(videoPath);
            if (!out)
                return;
            ImageSpec spec(xres, yres, channels, TypeDesc::UINT8);

            if (!out->supports("multiimage") ||
                !out->supports("appendsubimage")) {
                spdlog::error("Current plugin doesn't support tif subimages");
                return;
            }

            ImageOutput::OpenMode appendmode = ImageOutput::Create;

            for (int s = 0; s < counter; ++s) {
                out->open(videoPath, spec, appendmode);
                out->write_image(TypeDesc::UINT8, pixels.data() + singleFrameSize * s);
                appendmode = ImageOutput::AppendSubimage;
            }
            break;
        }

        case MP4: {
            cv::VideoWriter writer{videoPath, cv::VideoWriter::fourcc('X', '2', '6', '4'),
                                   static_cast<double>(ctx.framerate),
                                   cv::Size{xres, yres}};

            for (int s = 0; s < counter; ++s) {
                auto mat = cv::Mat(yres, xres, CV_8UC3, pixels.data() + singleFrameSize * s);
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