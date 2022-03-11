#ifndef PRIME_APP_BACKEND_H
#define PRIME_APP_BACKEND_H

#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>
#include <misc/Log.h>

namespace slr {
    class Backend {
    public:
        Backend(int argc, char** argv, sf::RenderWindow& window, sf::Texture& texture,
            sf::Clock& clock, sf::Time& dt, Log& log, std::mutex& mutex) : margc(argc), margv(argv), mWindow(window), mCurrentTexture(texture), mDeltaClock(clock), mDt(dt), mAppLog(log), mTextureMutex(mutex), mIsCapturing() { }

        void Update() {
            mDt = mDeltaClock.restart();
            PollInput();
        }

        virtual void Init() { }

        virtual void LiveCapture() { }

        virtual void TerminateCapture() {}

        virtual ~Backend() = default;

    protected:
        int margc;
        char** margv;
        sf::RenderWindow& mWindow;

        sf::Texture& mCurrentTexture;
        std::mutex&   mTextureMutex;

        std::jthread mWorkerThread;
        std::atomic<bool> mIsCapturing;

        sf::Clock& mDeltaClock;
        sf::Time& mDt;

        Log& mAppLog;

    private:
        void PollInput() {
            sf::Event event{};

            while (mWindow.pollEvent(event)) {
                ImGui::SFML::ProcessEvent(event);

                switch (event.type) {
                case sf::Event::Closed:
                    mWindow.close();
                    break;

                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        mWindow.close();
                        break;
                    default:
                        break;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    };
    }

#endif // PRIME_APP_BACKEND_H
