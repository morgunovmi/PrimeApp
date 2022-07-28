#pragma once

#include "misc/Meta.h"

namespace prm
{
    /**
     * Enumeration for all possible save file formats
     */
    enum SAVE_FORMAT
    {
        TIF = 0,///< tiff stack
        MP4 = 1,///< mp4
        DIR = 2///< separate directory with tif stack and metadata
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

        /**
         * Writes a tiff stack of 16 bit images captured by pvcam
         *
         * @param data Byte array containing captured images
         * @param imageWidth Width of each image
         * @param imageHeight Height of each image
         * @param imageSize Size of each image in bytes
         * @param filePath Path where to save the stack file
         * @param numImages Number of images to save in the stack
         * @return
         */
        static bool WritePvcamStack(const void* data, uint16_t imageWidth,
                                    uint16_t imageHeight, uint16_t imageSize,
                                    std::string_view filePath,
                                    uint16_t numImages);

        /**
         * Writes tif stack capture metadata in json file
         *
         * @param filePath Folder in which to save the metadata
         * @param meta Metadata struct
         * @return true on success
         */
        static bool WriteTifMetadata(std::string_view filePath,
                                     const TifStackMeta& meta);

        static std::string ReadFileToString(const std::string_view file_path);
        static std::vector<std::string> Tokenize(const std::string& string);
    };
}// namespace prm