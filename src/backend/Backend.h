#ifndef PRIME_APP_BACKEND_H
#define PRIME_APP_BACKEND_H

#include <imgui-SFML.h>
#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

#include "misc/Log.h"
#include "messages/MessageQueue.h"
#include "messages/messages.h"
#include "utils/FileUtils.h"

namespace prm
{
    constexpr std::string_view LIVE_CAPTURE_PREFIX{"LiveCapture_"};
    constexpr std::string_view SEQ_CAPTURE_PREFIX{"SequenceCapture_"};

    class Backend
    {
    public:
        Backend(int argc, char** argv, sf::RenderWindow& window,
                sf::Texture& texture, sf::Time& dt, std::mutex& mutex)
            : m_argc(argc), m_argv(argv), m_window(window),
              m_currentTexture(texture), m_dt(dt), m_textureMutex(mutex)
        {
        }

        explicit Backend(const std::unique_ptr<Backend>& other)
            : m_argc(other->m_argc), m_argv(other->m_argv),
              m_window(other->m_window),
              m_currentTexture(other->m_currentTexture), m_dt(other->m_dt),
              m_textureMutex(other->m_textureMutex)
        {
        }

        virtual void Init() {}
        virtual void LiveCapture(SAVE_FORMAT format) {}
        virtual void SequenceCapture(uint32_t nFrames, SAVE_FORMAT format) {}
        virtual void TerminateCapture() {}

        virtual ~Backend() = default;

    protected:
        int m_argc;
        char** m_argv;
        sf::RenderWindow& m_window;

        sf::Texture& m_currentTexture;
        std::mutex& m_textureMutex;

        sf::Time& m_dt;

        template<typename... Args>
        static void PrintError(fmt::format_string<Args...> fmt, Args&&... args)
        {
            spdlog::error(fmt, args...);
        }
    };
}// namespace prm

#endif// PRIME_APP_BACKEND_H
