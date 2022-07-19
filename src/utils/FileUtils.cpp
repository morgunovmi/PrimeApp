#include <OpenImageIO/imageio.h>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

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
            case DIR:
                break;
            default:
                return std::string{};
        }
        return videoPath;
    }

    bool FileUtils::WritePvcamStack(const void* data, uint16_t imageWidth,
                                    uint16_t imageHeight, uint16_t imageSize,
                                    const std::string& filePath,
                                    uint16_t numImages)
    {
        using namespace OIIO;
        const auto tifPath = std::string{filePath + "\\stack.tif"};

        std::unique_ptr<ImageOutput> out = ImageOutput::create(tifPath);
        if (!out) return false;
        ImageSpec spec(imageWidth, imageHeight, 1, TypeDesc::UINT16);
        spec.attribute("compression", "none");

        if (!out->supports("multiimage") || !out->supports("appendsubimage"))
        {
            return false;
        }

        ImageOutput::OpenMode appendmode = ImageOutput::Create;

        for (std::size_t s = 0; s < numImages; ++s)
        {
            out->open(tifPath, spec, appendmode);
            out->write_image(TypeDesc::UINT16, (uint8_t*) data + imageSize * s);
            appendmode = ImageOutput::AppendSubimage;
        }
        return true;
    }

    bool FileUtils::WriteTifMetadata(const std::string& filePath,
                                     const TifStackMeta& meta)
    {
        if (auto ofs = std::ofstream{filePath + "\\meta.json"})
        {
            ofs << nlohmann::json{meta}.dump(4) << '\n';
        }

        return true;
    }

    std::string FileUtils::ReadFileToString(const std::string_view file_path)
    {
        if (auto ifs = std::ifstream{file_path.data()})
        {
            return {std::istreambuf_iterator<char>{ifs}, {}};
        }
        throw std::runtime_error{"Failed to read file to string"};
    }

    std::vector<std::string> FileUtils::Tokenize(const std::string& string)
    {
        const std::string delims{"\r\n\t"};
        std::vector<std::string> parts{};
        for (auto beg = string.find_first_not_of(delims);
             beg != std::string::npos;)
        {
            auto end = string.find_first_of(delims, beg);
            parts.emplace_back(string.substr(beg, end - beg));
            beg = string.find_first_not_of(delims, end);
        }
        return parts;
    }
}// namespace prm