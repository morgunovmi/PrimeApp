#ifndef PRIME_APP_FILEUTILS_H
#define PRIME_APP_FILEUTILS_H

#include <string_view>

namespace prm
{
    enum SAVE_FORMAT
    {
        TIF = 0,
        MP4 = 1
    };

    class FileUtils
    {
    public:
        static std::string GenerateVideoPath(std::string_view prefix,
                                             SAVE_FORMAT format);

        static bool WriteVideo(const void* data, std::string_view filePath);
    };
}// namespace slr

#endif//PRIME_APP_FILEUTILS_H
