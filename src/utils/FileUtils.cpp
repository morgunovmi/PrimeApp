#include <ctime>
#include <iomanip>
#include <sstream>

#include "FileUtils.h"

namespace prm
{
    std::string FileUtils::GenerateVideoPath(std::string_view dirPath,
                                             std::string_view prefix,
                                             SAVE_FORMAT format)
    {
        const auto t = std::time(nullptr);
        const auto tm = *std::localtime(&t);

        std::ostringstream oss{};
        oss << std::put_time(&tm, "%H-%M-%S");
        const auto curTime = oss.str();

        auto videoPath = prefix.data() + curTime;
        videoPath = "\\" + videoPath;
        videoPath = dirPath.data() + videoPath;

        switch (format)
        {
            case TIF:
                videoPath.append(".tif");
                break;
            case MP4:
                videoPath.append(".mp4");
                break;
            default:
                return std::string{};
        }
        return videoPath;
    }

    bool FileUtils::WriteVideo(const void* data, std::string_view filePath)
    {

        return true;
    }
}// namespace prm