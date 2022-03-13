#include "OpencvBackend.h"
#include "spdlog/spdlog.h"

sf::Image slr::OpencvBackend::MatToImage(const cv::Mat& mat) {
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
    mWorkerThread = std::jthread([&] {
        spdlog::info("Opening camera");
        mCamera = cv::VideoCapture{0};
        if (!mCamera.isOpened()) {
            PrintError("ERROR: Could not open camera");
            return;
        }
        mIsOpened.store(true);
        spdlog::info("Successful camera init");
    });
}

void slr::OpencvBackend::LiveCapture() {
    if (!mIsOpened.load()) {
        Init();
        return;
    }

    if (mIsOpened.load()) {
        mIsCapturing.store(true);
        mWorkerThread = std::jthread([&]() {
            const auto t = std::time(nullptr);
            const auto tm = *std::localtime(&t);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%H-%M-%S");
            const auto curTime = oss.str();

            const auto videoPath = fmt::format("{}{}.mp4", LIVE_CAPTURE_PREFIX, curTime);

            cv::VideoWriter writer{videoPath, cv::VideoWriter::fourcc('X', '2', '6', '4'), static_cast<double>(mFramerate),
                                   cv::Size{static_cast<int>(mCamera.get(cv::CAP_PROP_FRAME_WIDTH)),
                                            static_cast<int>(mCamera.get(cv::CAP_PROP_FRAME_HEIGHT))}};

            auto counter = 0;
            while (mIsCapturing.load()) {
                ++counter;

                cv::Mat frame;

                mCamera >> frame;
                if (frame.empty()) {
                    spdlog::info("Got empty frame. Stopping capture...");
                    mIsCapturing.store(false);
                    mCamera.release();
                    writer.release();
                    mIsOpened.store(false);
                    break;
                }

                spdlog::info("Frame no {}", counter);

                writer.write(frame);

                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
                const auto image = MatToImage(frame);

                std::scoped_lock lock(mTextureMutex);
                mCurrentTexture.loadFromImage(image);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            mCamera.release();
            writer.release();
        });
    }
}

void slr::OpencvBackend::TerminateCapture() {
    mIsOpened.store(false);
    mIsCapturing.store(false);
}