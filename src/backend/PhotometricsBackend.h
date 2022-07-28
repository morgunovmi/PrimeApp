#pragma once

#include <SFML/Graphics.hpp>

#include <mutex>
#include <string>
#include <vector>

#include <master.h>
#include <pvcam.h>

#include "Backend.h"
#include "misc/Log.h"
#include "misc/Meta.h"

namespace prm
{
    /// Name-Value Pair Container type - an enumeration type
    struct NVP
    {
        int32 value;
        std::string name;
    };

    /// Container of name-value pairs
    using NVPC = std::vector<NVP>;

    struct READOUT_OPTION
    {
        NVP port;
        int16 speedIndex;
        float readoutFrequency;
        int16 bitDepth;
        std::vector<int16> gains;
    };

    struct SpdtabGain
    {
        // In PVCAM, gain indexes are 1-based.
        int32 index{1};
        // Not all cameras support named gains. If not supported, this
        // string stays empty.
        std::string name{};
        // The bit-depth may be different for each gain, therefore it is stored
        // within this structure. For example, the Prime BSI camera has gains
        // with different bit-depths under the same speed.
        int16 bitDepth{0};
    };

    struct SpdtabSpeed
    {
        // In PVCAM, speed indexes are 0-based.
        int32 index{0};
        // Pixel time can be used to calculate the overall readout rate. This is less
        // relevant with sCMOS sensors, but the pix time is still reported to provide
        // an approximate readout rate of a particular speed.
        uns16 pixTimeNs{1};
        // List of gains under this particular speed.
        std::vector<SpdtabGain> gains;
    };

    struct SpdtabPort
    {
        // Please note that the PARAM_READOUT_PORT is an ENUM_TYPE parameter.
        // For this reason, the current port is not reported as an index, but
        // as a generic number. Applications that are written in a generic way
        // to support all Teledyne Photometrics cameras should not rely on a fact
        // that port numbers are usually zero-based.
        int32 value{0};
        // Name of this port, as retrieved by enumerating the PARAM_READOUT_PORT.
        std::string name{};
        // List of speeds under this particular port.
        std::vector<SpdtabSpeed> speeds;
    };

    /// Struct for camera event synchronisation
    struct Event
    {
        /// Mutex that guards all other members
        std::mutex mutex{};
        /// Condition that any thread could wait on
        std::condition_variable cond{};
        /// A flag that helps with spurious wakeups
        bool flag{false};
    };

    /// Struct grouping camera related info
    struct CameraContext
    {
        /// Camera name, the only member valid before opening camera
        char camName[CAM_NAME_LEN]{'\0'};

        /// Set to true when PVCAM opens camera
        bool isCamOpen{false};

        /// Specifies if the camera is currently capturing
        bool isCapturing{false};

        // All members below are initialized once the camera is successfully open

        /// Camera handle
        int16 hcam{-1};

        /// Event used for communication between acq. loop and EOF callback routine
        Event eofEvent{};

        /// Camera sensor serial size (sensor width)
        uns16 sensorResX{0};
        /// Camera sensor parallel size (sensor height)
        uns16 sensorResY{0};
        /**
         * Sensor region and binning factors to be used for the acquisition,
         * initialized to full sensor size with 1x1 binning upon opening the camera.
         */
        rgn_type region{0, 0, 0, 0, 0, 0};

        uint16_t exposureTime = 10;

        /// Vector of camera readout options, commonly referred to as 'speed table'
        std::vector<SpdtabPort> speedTable{};

        /// Image format reported after acq. setup, value from PL_IMAGE_FORMATS
        int32 imageFormat{PL_IMAGE_FORMAT_MONO16};
        /**
         * Sensor type (if not Frame Transfer CCD then camera is Interline CCD or sCMOS).
         * Not relevant for sCMOS sensors.
         */
        bool isFrameTransfer{false};
        /// Flag marking the camera as Smart Streaming capable
        bool isSmartStreaming{false};

        /// Frame info structure used to store data, for example, in EOF callback handlers
        FRAME_INFO eofFrameInfo{};
        /// The address of latest frame stored, for example, in EOF callback handlers
        void* eofFrame{nullptr};

        /// Used as an acquisition thread or for other independent tasks
        std::unique_ptr<std::jthread> thread{nullptr};
        /// Flag to be set to abort thread (used, for example, in multi-camera code samples)
        bool threadAbortFlag{false};

        /// Lens used on the camera during capture
        Lens lens = X20;
    };

    /**
     * Backend implementation for Teledyne Photometrics cameras
     * Connects to the usb camera and captures image sequence from it
     */
    class PhotometricsBackend : public Backend
    {
    public:
        PhotometricsBackend(int argc, char** argv, sf::RenderWindow& window,
                            sf::Texture& currentTexture, sf::Time& dt,
                            std::mutex& mutex)
            : Backend(argc, argv, window, currentTexture, dt, mutex)
        {
            if (!ShowAppInfo(m_argc, m_argv))
            {
                PrintError("Couldn't show app info");
            }
            Init();
        }

        /**
         * Explicit "copy"  constructor from a unique_ptr
         *
         * @param other unique_ptr to a Backend from which to construct a new one
         */
        explicit PhotometricsBackend(const std::unique_ptr<Backend>& other)
            : Backend(other)
        {
            if (!ShowAppInfo(m_argc, m_argv))
            {
                PrintError("Couldn't show app info");
            }
            Init();
        }

        /**
         * Handles generic camera initialization
         */
        void Init() override;

        /**
         * Captures live image sequence
         *
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void LiveCapture(SAVE_FORMAT format, bool save) override;

        /**
         * Captures image sequence of specified length
         *
         * @param nFrames Image sequence length
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void SequenceCapture(uint32_t nFrames, SAVE_FORMAT format,
                             bool save) override;

        /**
         * Stops the ongoing capture process
         */
        void TerminateCapture() override;

        /**
         * Returns a pointer to the current camera context
         *
         * @return Raw pointer to the camera context
         */
        CameraContext* GetCurrentCameraContext()
        {
            return m_cameraContexts[m_cameraIndex].get();
        }

        /**
         * Converts byte array captured by PVCAM to SFML Image
         *
         * @param imageData Byte array
         * @param imageWidth Image width
         * @param imageHeight Image height
         * @param minVal Minimum brightness value to display
         * @param maxVal Maximum brightness value to display
         * @return Resulting SFML Image
         */
        static sf::Image PVCamImageToSfImage(uint16_t* imageData,
                                             uint16_t imageWidth,
                                             uint16_t imageHeight,
                                             uint32_t minVal, uint32_t maxVal);

        ~PhotometricsBackend() override { CloseAllCamerasAndUninit(); }

    private:
        /**
         * Internal init function to send to the worker thread
         */
        void Init_();
        /**
         * Internal live capture implementation to send to the worker thread
         *
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void LiveCapture_(SAVE_FORMAT format, bool save);
        /**
         * Internal sequence capture implementation to send to the worker thread
         *
         * @param nFrames Image sequence length
         * @param format Save file format from the SAVE_FORMAT enum
         * @param save Flag indicating the need to save the captured sequence
         */
        void SequenceCapture_(uint32_t nFrames, SAVE_FORMAT format, bool save);

        /**
         * Initializes PVCAM library, obtains basic camera availability information,
         * opens one camera and retrieves basic camera parameters and characteristics.
         * @return true if camera opened successfully
         */
        bool InitAndOpenOneCamera();

        /**
         * PVCam error printing function
         *
         * @tparam Args Variadic format string arguments pack
         * @param fmt Format string
         * @param args Arguments to insert in the format string
         */
        template<typename... Args>
        static void PrintError(fmt::format_string<Args...> fmt, Args&&... args);

        /**
         * Displays application name and version
         *
         * @param argc Command line argument count
         * @param argv Command line argument string array
         * @return true on success
         */
        bool ShowAppInfo(int argc, char* argv[]);

        /**
         * Reads name-value pairs for given PVCAM enum-type parameter.
         *
         * @param[in] hcam Camera handle
         * @param[out] pNvpc Pointer to the container to read the enumeration to
         * @param[in] paramID ID of the parameter
         * @param[in] paramName String name of the parameter
         * @return true on success
         */
        bool ReadEnumeration(int16 hcam, NVPC* pNvpc, uns32 paramID,
                             const char* paramName);

        /**
         * Checks parameter availability
         *
         * @param hcam Camera handle
         * @param paramID ID of the parameter
         * @param paramName String name of the parameter
         * @return true if parameter is available
         */
        bool IsParamAvailable(int16 hcam, uns32 paramID, const char* paramName);

        /**
         * Builds and returns the camera speed table.
         *
         * @param[in] ctx unique_ptr to the camera context
         * @param[out] speedTable Vector to read the table to
         * @return true on success
         */
        bool GetSpeedTable(const std::unique_ptr<CameraContext>& ctx,
                           std::vector<SpdtabPort>& speedTable);

        /**
         * Opens camera if not open yet
         *
         * @param ctx unique_ptr to camera context
         * @return true on success
         */
        bool OpenCamera(std::unique_ptr<CameraContext>& ctx);

        /**
         * Closes given camera if not closed yet
         *
         * @param ctx unique_ptr to camera context
         */
        void CloseCamera(std::unique_ptr<CameraContext>& ctx);

        /**
         * Initializes PVCAM library and allocates camera context for all detected cameras
         *
         * @return true on success
         */
        bool InitPVCAM();

        /**
         * Releases allocated camera contexts and uninitializes the PVCAM library
         */
        void UninitPVCAM();

        /**
         * Closes the cameras and uninitializes PVCAM
         */
        void CloseAllCamerasAndUninit();

        /**
         * This function is called after pl_exp_setup_seq and pl_exp_setup_cont functions,
         * or after changing value of the selected post-processing parameters. The function reads
         * the current image format reported by the camera.
         * With most cameras, each pixel is transferred in 2 bytes, up to 16 bits per pixel.
         * However, selected cameras support 8-bit sensor readouts and some post processing features
         * also enable 32-bit image format.
         * The actual bit depth, i.e. the number of bits holding pixel values, is still
         * independent and reported by PARAM_BIT_DEPTH parameter.
         *
         * @param ctx unique_ptr to the camera context
         */
        void UpdateCtxImageFormat(std::unique_ptr<CameraContext>& ctx);

        /**
         *
         * Selects an appropriate exposure mode for use in pl_exp_setup_seq() and pl_exp_setup_cont()
         * functions. The function checks whether the camera supports the legacy (TIMED_MODE, STROBED_MODE, ...)
         * or extended trigger modes (EXT_TRIG_INTERNAL, ...) and returns a correct value together
         * with the first expose-out mode option, if applicable. Usually, if an application works with
         * one camera model, such dynamic discovery is not required. However, the SDK examples
         * are written so that they will function with the older, legacy cameras, too.
         *
         * @param ctx unique_ptr to the camera context
         * @param expMode
         * @param legacyTrigMode
         * @param extendedTrigMode
         * @return true on success
         */
        bool SelectCameraExpMode(const std::unique_ptr<CameraContext>& ctx,
                                 int16& expMode, int16 legacyTrigMode,
                                 int16 extendedTrigMode);

        /**
         *
         * Generic EOF callback handler used in most code samples.
         * This is the function registered as a callback function and PVCAM will call it
         * every time a new frame arrives.
         *
         * @param pFrameInfo Pointer to the captured frame info struct
         * @param pContext Pointer to the camera context
         */
        static void CustomEofHandler(FRAME_INFO* pFrameInfo, void* pContext);

        /**
         * Waits for a notification that is usually sent by EOF callback handler.
         *
         * @param[in] ctx Pointer to the camera context
         * @param[in] timeoutMs Time to wait for in milliseconds
         * @param[out] errorOccurred Flag that shows if an error has ocurred
         * @return false if the event didn't occur before the specified timeout, or when user aborted the waiting.
         */
        static bool WaitForEofEvent(CameraContext* ctx, uns32 timeoutMs,
                                    bool& errorOccurred);

        /**
         * Subtracts image background by calculating morphological opening
         *
         * @param bytes Image byte array
         * @param width Image width
         * @param height Image height
         * @param nFrames Number of images in the array
         * @return true on success
         */
        static bool SubtractBackground(uint8_t* bytes,
                                       uint16_t width, uint16_t height,
                                       uint32_t nFrames);

    private:
        /// Index of the current camera
        const uns16 m_cameraIndex = 0;

        /// Vector of all camera contexts
        std::vector<std::unique_ptr<CameraContext>> m_cameraContexts;

    public:
        /// Shows if PVCam environment is initialized
        bool m_isPvcamInitialized = false;

        /// Minimum brightness value to display in the GUI
        int m_minDisplayValue = 0;
        /// Maximum brightness value to display in the GUI
        int m_maxDisplayValue = 4096;// 12 bits

        /// Minimum brightness value in the current frame
        uint16_t m_minCurrentValue = 0;
        /// Maximum brightness value in the current frame
        uint16_t m_maxCurrentValue = 0;

        bool m_bSubtractBackground = false;
    };
}// namespace prm