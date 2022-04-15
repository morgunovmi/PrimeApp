#ifndef PRIME_APP_BACKEND_H
#define PRIME_APP_BACKEND_H

#include "imgui-SFML.h"
#include "spdlog/spdlog.h"
#include <SFML/Graphics.hpp>
#include <misc/Log.h>

#include "messages/MessageQueue.h"
#include "messages/messages.h"

namespace slr {
    enum CAP_FORMAT {
        TIF = 0,
        MP4 = 1
    };

    class Backend {
    public:
        Backend(int argc, char **argv, sf::RenderWindow &window, sf::Texture &texture,
                sf::Time &dt, std::mutex &mutex) : margc(argc), margv(argv), mWindow(window),
                                                   mCurrentTexture(texture),
                                                   mDt(dt), mTextureMutex(mutex) {}

        explicit Backend(const std::unique_ptr<Backend> &other) : margc(other->margc), margv(other->margv),
                                                                  mWindow(other->mWindow),
                                                                  mCurrentTexture(other->mCurrentTexture),
                                                                  mDt(other->mDt),
                                                                  mTextureMutex(other->mTextureMutex) {}

        virtual void Init() {}

        virtual void LiveCapture(CAP_FORMAT format) {}

        virtual void SequenceCapture(uint32_t nFrames, CAP_FORMAT format) {}

        virtual void TerminateCapture() {}

        virtual ~Backend() = default;

    protected:
        int margc;
        char **margv;
        sf::RenderWindow &mWindow;

        sf::Texture &mCurrentTexture;
        std::mutex &mTextureMutex;

        sf::Time &mDt;

        template<typename... Args>
        static void PrintError(fmt::format_string<Args...> fmt, Args &&...args) {
            spdlog::error(fmt, args...);
        }
    };
}

#endif // PRIME_APP_BACKEND_H
