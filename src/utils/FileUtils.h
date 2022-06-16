#pragma once

#include <string_view>

namespace prm
{
    /**
     * Enumeration for all possible save file formats
     */
    enum SAVE_FORMAT
    {
        TIF = 0,///< tiff stack
        MP4 = 1 ///< mp4
    };

    /**
     * Class for common file utility functions
     */
    class FileUtils
    {
    public:
        /**
         * Generates a video path with the current timestamp
         *
         * @param prefix Prefix string to use in the path
         * @param format File save format from the SAVE_FORMAT enum
         * @param dirPath Directory in which to save the video
         * @return std::string with the generated path
         */
        static std::string GenerateVideoPath(std::string_view dirPath,
                                     std::string_view prefix,
                                     SAVE_FORMAT format);

        static bool WriteVideo(const void* data,
                               std::string_view filePath);
    };
}// namespace prm