#pragma once

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include "messages/MessageQueue.h"
#include "messages/messages.h"
#include "misc/Log.h"
#include "utils/FileUtils.h"

namespace prm
{
    /// String prefix for live capture files
    const std::string LIVE_CAPTURE_PREFIX{"LiveCapture_"};
    /// String prefix for sequence capture files
    const std::string SEQ_CAPTURE_PREFIX{"SequenceCapture_"};

    /**
     * Base camera backend class to group common functionality
     */
    class Backend
    {
    public:
        Backend(int argc, char** argv, sf::RenderWindow& window,
                sf::Texture& texture, sf::Time& dt, std::mutex& mutex)
            : m_argc(argc), m_argv(argv), m_window(window),
              m_currentTexture(texture), m_dt(dt), m_textureMutex(mutex)
        {
        }

        /**
         * Explicit "copy"  constructor from a unique_ptr
         *
         * @param other unique_ptr to a Backend from which to construct a new one
         */
        explicit Backend(const std::unique_ptr<Backend>& other)
            : m_argc(other->m_argc), m_argv(other->m_argv),
              m_window(other->m_window),
              m_currentTexture(other->m_currentTexture), m_dt(other->m_dt),
              m_textureMutex(other->m_textureMutex)
        {
        }

        /**
         * Handles generic camera initialization
         */
        virtual void Init() {}

        /**
         * Captures live image sequence
         *
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        virtual void LiveCapture(SAVE_FORMAT format, bool save) {}

        /**
         * Captures image sequence of specified length
         *
         * @param nFrames Image sequence length
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        virtual void SequenceCapture(uint32_t nFrames, SAVE_FORMAT format,
                                     bool save)
        {
        }

        [[nodiscard]] bool IsCapturing() const { return m_isCapturing; }

        [[nodiscard]] const std::string& GetDirPath() const { return m_saveDirPath; }
        void SetDirPath(std::string_view dirPath)
        {
            m_saveDirPath = dirPath.data();
        }

        /**
         * Stops the ongoing capture process
         */
        virtual void TerminateCapture() {}

        virtual ~Backend() = default;

    protected:
        /// Command line argument count
        int m_argc;

        /// Command line argument array
        char** m_argv;

        /// Directory in which to save the video
        std::string m_saveDirPath = ".";

        /// Is camera capturing
        bool m_isCapturing = false;

        /// Reference to the SFML render window
        sf::RenderWindow& m_window;

        /// Reference to the SFML texture that is to be drawn this frame
        sf::Texture& m_currentTexture;
        /// Mutex for texture synchronisation
        std::mutex& m_textureMutex;

        /// Delta time for last frame
        sf::Time& m_dt;

        /**
         * Generic error printing function
         *
         * @tparam Args Variadic format string arguments pack
         * @param fmt Format string
         * @param args Arguments to insert in the format string
         */
        template<typename... Args>
        static void PrintError(fmt::format_string<Args...> fmt, Args&&... args)
        {
            spdlog::error(fmt, args...);
        }
    };
}// namespace prm