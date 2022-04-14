#include "OpencvBackend.h"
#include "spdlog/spdlog.h"

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

void slr::OpencvBackend::LiveCapture() {
    if (!mContext.isCamOpen) {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (mContext.isCapturing) {
        spdlog::warn("Already capturing, please stop current acquisition first");
        return;
    }

    mContext.isCapturing = true;
    mContext.thread = std::make_unique<std::jthread>(&OpencvBackend::LiveCapture_, this, std::ref(mContext));
}

void slr::OpencvBackend::SequenceCapture(uint32_t nFrames) {
    if (!mContext.isCamOpen) {
        spdlog::warn("No cam is open\n Please init first");
        return;
    }

    if (mContext.isCapturing) {
        spdlog::warn("Already capturing, please stop current acquisition first");
        return;
    }

    mContext.isCapturing = true;
    mContext.thread = std::make_unique<std::jthread>(&OpencvBackend::SequenceCapture_, this, std::ref(mContext), nFrames);
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

void slr::OpencvBackend::LiveCapture_(slr::OpencvCameraCtx &ctx) {
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H-%M-%S");
    const auto curTime = oss.str();

    const auto videoPath = fmt::format("{}{}.mp4", LIVE_CAPTURE_PREFIX, curTime);

    cv::VideoWriter writer{videoPath, cv::VideoWriter::fourcc('X', '2', '6', '4'),
                           static_cast<double>(ctx.framerate),
                           cv::Size{static_cast<int>(ctx.camera->get(cv::CAP_PROP_FRAME_WIDTH)),
                                    static_cast<int>(ctx.camera->get(cv::CAP_PROP_FRAME_HEIGHT))}};

    auto counter = 0;
    while (ctx.isCapturing) {
        ++counter;

        cv::Mat frame;

        *ctx.camera >> frame;
        if (frame.empty()) {
            spdlog::error("Got empty frame. Stopping capture...");
            ctx.isCapturing = false;
            ctx.camera->release();
            writer.release();
            ctx.isCamOpen = false;
            break;
        }

        spdlog::debug("Frame no {}", counter);

        writer.write(frame);

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        const auto image = MatToImage(frame);

        std::scoped_lock lock(mTextureMutex);
        mCurrentTexture.loadFromImage(image);
        std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<uint64_t>(1.f / static_cast<float>(ctx.framerate)) * 1000));
    }
    ctx.camera->release();
    ctx.isCamOpen = false;
    writer.release();
}

void slr::OpencvBackend::SequenceCapture_(OpencvCameraCtx &ctx, uint16_t nFrames) {
    const auto t = std::time(nullptr);
    const auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H-%M-%S");
    const auto curTime = oss.str();

    const auto videoPath = fmt::format("{}{}.mp4", SEQ_CAPTURE_PREFIX, curTime);

    cv::VideoWriter writer{videoPath, cv::VideoWriter::fourcc('X', '2', '6', '4'),
                           static_cast<double>(ctx.framerate),
                           cv::Size{static_cast<int>(ctx.camera->get(cv::CAP_PROP_FRAME_WIDTH)),
                                    static_cast<int>(ctx.camera->get(cv::CAP_PROP_FRAME_HEIGHT))}};

    for (uint32_t counter = 0; counter < nFrames && ctx.isCapturing; ++counter) {
        spdlog::debug("Counter: {}", counter);
        cv::Mat frame;

        *ctx.camera >> frame;
        if (frame.empty()) {
            spdlog::info("Got empty frame. Stopping capture...");
            ctx.isCapturing = false;
            ctx.camera->release();
            writer.release();
            ctx.isCamOpen = false;
            break;
        }

        spdlog::debug("Frame no {}", counter);

        writer.write(frame);

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        const auto image = MatToImage(frame);

        std::scoped_lock lock(mTextureMutex);
        mCurrentTexture.loadFromImage(image);
        std::this_thread::sleep_for(
                std::chrono::milliseconds(static_cast<uint64_t>(1.f / static_cast<float>(ctx.framerate)) * 1000));
    }

    ctx.camera->release();
    ctx.isCamOpen = false;
    ctx.isCapturing = false;
    writer.release();
}