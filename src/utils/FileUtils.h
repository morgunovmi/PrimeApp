#pragma once

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
                                    const std::string& filePath,
                                    uint16_t numImages);

        static std::string ReadFileToString(const std::string_view file_path);
        static std::vector<std::string> Tokenize(const std::string& string);
    };
}// namespace prm