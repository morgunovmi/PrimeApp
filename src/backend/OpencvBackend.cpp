#include "OpencvBackend.h"

void slr::OpencvBackend::Init() {
    mWorkerThread = std::jthread([&] {
        mCamera = cv::VideoCapture{0};
        if (!mCamera.isOpened()) {
            std::cerr << "ERROR: Could not open camera" << std::endl;
            return;
        }
        mIsOpened.store(true);
        mAppLog.AddLog("Successfult camera init\n");
    });
}

void slr::OpencvBackend::LiveCapture() {
    if (!mIsOpened.load()) {
        Init();
    }

    if (mIsOpened.load()) {
        mIsCapturing.store(true);
        mWorkerThread = std::jthread([&]() {
            while (mIsCapturing.load()) {
                cv::Mat frame;

                mCamera >> frame;
                mAppLog.AddLog("Meme\n");
                cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

                sf::Image image;
                image.create(frame.size().width, frame.size().height);
                for (std::size_t x = 0; x < frame.size().width; ++x) {
                    for (std::size_t y = 0; y < frame.size().height; ++y) {
                        image.setPixel(x, y, sf::Color{frame.at<cv::Vec3b>(y, x)[0], frame.at<cv::Vec3b>(y, x)[1], frame.at<cv::Vec3b>(y, x)[2]});
                    }
                }

                std::scoped_lock lock(mTextureMutex);
                mCurrentTexture.loadFromImage(image);
            }
            mCamera.release();
        });
    }
}

void slr::OpencvBackend::TerminateCapture() {
    mIsOpened.store(false);
    mIsCapturing.store(false);
}
