#include <filesystem>
#include <fmt/format.h>
#include <iomanip>
#include <spdlog/spdlog.h>

#include <opencv2/opencv.hpp>

#include "backend/PhotometricsBackend.h"
#include "misc/Meta.h"
#include "utils/FileUtils.h"
#include "utils/Timer.h"

namespace prm
{
    template<typename... Args>
    void PhotometricsBackend::PrintError(fmt::format_string<Args...> fmt,
                                         Args&&... args)
    {
        auto code = pl_error_code();
        char pvcamErrMsg[ERROR_MSG_LEN];
        pl_error_message(code, pvcamErrMsg);

        auto str = fmt::format("\n-----Error code {}-----\n"
                               "{}\n",
                               code, pvcamErrMsg);

        auto str1 = fmt::vformat(fmt, fmt::make_format_args(args...));

        auto str2 = fmt::format("------------------------\n");
        spdlog::error("{}{}{}", str, str1, str2);
    }

    sf::Image PhotometricsBackend::PVCamImageToSfImage(uint16_t* imageData,
                                                       uint16_t imageWidth,
                                                       uint16_t imageHeight,
                                                       uint32_t minVal,
                                                       uint32_t maxVal)
    {
        sf::Image image{};

        image.create(imageWidth, imageHeight);
        for (std::size_t y = 0; y < imageHeight; ++y)
        {
            for (std::size_t x = 0; x < imageWidth; ++x)
            {
                uint16_t val = *(imageData + y * imageWidth + x);
                val = std::clamp(val, (uint16_t) minVal, (uint16_t) maxVal);
                auto val8 =
                        static_cast<uint8_t>(static_cast<float>(val - minVal) /
                                             (maxVal - minVal + 1) * 256.f);
                image.setPixel(x, y, sf::Color{val8, val8, val8});
            }
        }
        return image;
    }

    bool PhotometricsBackend::ShowAppInfo(int argc, char* argv[])
    {
        auto appName = "<unable to get name>";
        if (argc > 0 && argv != nullptr && argv[0] != nullptr)
        {
            appName = argv[0];
        }

        // Read PVCAM library version
        uns16 pvcamVersion{};
        if (PV_OK != pl_pvcam_get_ver(&pvcamVersion))
        {
            PrintError("pl_pvcam_get_ver() error");
            return PV_FAIL;
        }

        spdlog::info("\n*******************************************************"
                     "*****\n"
                     "Application  : {}\n"
                     "PVCAM version: {}.{}.{}\n"
                     "*********************************************************"
                     "***\n",
                     appName, (pvcamVersion >> 8) & 0xFF,
                     (pvcamVersion >> 4) & 0x0F, (pvcamVersion >> 0) & 0x0F);

        return PV_OK;
    }

    bool PhotometricsBackend::InitPVCAM()
    {
        if (m_isPvcamInitialized) { return true; }

        // Initialize PVCAM library
        if (PV_OK != pl_pvcam_init())
        {
            PrintError("pl_pvcam_init() error");
            return false;
        }

        m_isPvcamInitialized = true;
        spdlog::info("PVCAM initialized");

        m_cameraContexts.clear();
        int16 nrOfCameras;

        // Read the number of cameras in the system.
        // This will return total number of PVCAM cameras regardless of interface.
        if (PV_OK != pl_cam_get_total(&nrOfCameras))
        {
            PrintError("pl_cam_get_total() error\n");
            UninitPVCAM();
            return false;
        }

        // Exit if no cameras have been found
        if (nrOfCameras <= 0)
        {
            PrintError("No cameras found in the system");
            UninitPVCAM();
            return false;
        }
        spdlog::info("Number of cameras found: {}", nrOfCameras);

        // Create context structure for all cameras and fill in the PVCAM camera names
        m_cameraContexts.reserve(nrOfCameras);
        for (int16 i = 0; i < nrOfCameras; i++)
        {
            auto ctx = std::make_unique<CameraContext>();
            if (!ctx)
            {
                PrintError("Unable to allocate memory for CameraContext");
                UninitPVCAM();
                return false;
            }

            // Obtain PVCAM-name for this particular camera
            if (PV_OK != pl_cam_get_name(i, ctx->camName))
            {
                PrintError("pl_cam_get_name() error\n");
                UninitPVCAM();
                return false;
            }
            spdlog::info("Camera {} name: '{}'", i, ctx->camName);

            m_cameraContexts.push_back(std::move(ctx));
        }

        return true;
    }

    void PhotometricsBackend::UninitPVCAM()
    {
        if (!m_isPvcamInitialized) { return; }

        m_cameraContexts.clear();

        // Uninitialize the PVCAM library
        if (PV_OK != pl_pvcam_uninit())
        {
            spdlog::error("pl_pvcam_uninit() error\n");
        }
        else
        {
            spdlog::info("PVCAM uninitialized\n");
        }
        m_isPvcamInitialized = false;
    }

    bool PhotometricsBackend::GetSpeedTable(
            const std::unique_ptr<CameraContext>& ctx,
            std::vector<SpdtabPort>& speedTable)
    {
        std::vector<SpdtabPort> table;

        NVPC ports;
        if (!ReadEnumeration(ctx->hcam, &ports, PARAM_READOUT_PORT,
                             "PARAM_READOUT_PORT"))
            return false;

        if (!IsParamAvailable(ctx->hcam, PARAM_SPDTAB_INDEX,
                              "PARAM_SPDTAB_INDEX"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_PIX_TIME, "PARAM_PIX_TIME"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_GAIN_INDEX, "PARAM_GAIN_INDEX"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_BIT_DEPTH, "PARAM_BIT_DEPTH"))
            return false;

        rs_bool isGainNameAvailable;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_NAME, ATTR_AVAIL,
                                  (void*) &isGainNameAvailable))
        {
            PrintError("Error reading ATTR_AVAIL of PARAM_GAIN_NAME");
            return false;
        }
        const bool isGainNameSupported = isGainNameAvailable != FALSE;

        // Iterate through available ports and their speeds
        for (size_t pi = 0; pi < ports.size(); pi++)
        {
            // Set readout port
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_READOUT_PORT,
                                      (void*) &ports[pi].value))
            {
                PrintError("pl_set_param(PARAM_READOUT_PORT) error");
                return false;
            }

            // Get number of available speeds for this port
            uns32 speedCount;
            if (PV_OK != pl_get_param(ctx->hcam, PARAM_SPDTAB_INDEX, ATTR_COUNT,
                                      (void*) &speedCount))
            {
                PrintError("pl_get_param(PARAM_SPDTAB_INDEX) error");
                return false;
            }

            SpdtabPort port;
            port.value = ports[pi].value;
            port.name = ports[pi].name;

            // Iterate through all the speeds
            for (int16 si = 0; si < (int16) speedCount; si++)
            {
                // Set camera to new speed index
                if (PV_OK !=
                    pl_set_param(ctx->hcam, PARAM_SPDTAB_INDEX, (void*) &si))
                {
                    PrintError("pl_set_param(PARAM_SPDTAB_INDEX) error");
                    return false;
                }

                // Get pixel time (readout time of one pixel in nanoseconds) for the
                // current port/speed pair. This can be used to calculate readout
                // frequency of the port/speed pair.
                uns16 pixTime;
                if (PV_OK != pl_get_param(ctx->hcam, PARAM_PIX_TIME,
                                          ATTR_CURRENT, (void*) &pixTime))
                {
                    PrintError("pl_get_param(PARAM_PIX_TIME) error");
                    return false;
                }

                uns32 gainCount;
                if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_INDEX,
                                          ATTR_COUNT, (void*) &gainCount))
                {
                    PrintError("pl_get_param(PARAM_GAIN_INDEX) error");
                    return false;
                }

                SpdtabSpeed speed;
                speed.index = si;
                speed.pixTimeNs = pixTime;

                // Iterate through all the gains, notice it starts at value 1!
                for (int16 gi = 1; gi <= (int16) gainCount; gi++)
                {
                    // Set camera to new gain index
                    if (PV_OK !=
                        pl_set_param(ctx->hcam, PARAM_GAIN_INDEX, (void*) &gi))
                    {
                        PrintError("pl_set_param(PARAM_GAIN_INDEX) error");
                        return false;
                    }

                    // Get bit depth for the current gain
                    int16 bitDepth;
                    if (PV_OK != pl_get_param(ctx->hcam, PARAM_BIT_DEPTH,
                                              ATTR_CURRENT, (void*) &bitDepth))
                    {
                        PrintError("pl_get_param(PARAM_BIT_DEPTH) error");
                        return false;
                    }

                    SpdtabGain gain;
                    gain.index = gi;
                    gain.bitDepth = bitDepth;

                    if (isGainNameSupported)
                    {
                        char gainName[MAX_GAIN_NAME_LEN];
                        if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_NAME,
                                                  ATTR_CURRENT,
                                                  (void*) gainName))
                        {
                            PrintError("pl_get_param(PARAM_GAIN_NAME) error");
                            return false;
                        }

                        gain.name = gainName;
                    }

                    speed.gains.push_back(gain);
                }

                port.speeds.push_back(speed);
            }

            table.push_back(port);
        }

        speedTable.swap(table);
        return true;
    }

    bool PhotometricsBackend::OpenCamera(std::unique_ptr<CameraContext>& ctx)
    {
        if (ctx->isCamOpen) return true;

        // Open a camera with a given name and retrieve its handle. From now on,
        // only the handle will be used when referring to this particular camera.
        // The name is obtained in InitPVCAM() function when listing all cameras
        // in the system.
        if (PV_OK != pl_cam_open(ctx->camName, &ctx->hcam, OPEN_EXCLUSIVE))
        {
            PrintError("pl_cam_open() error");
            return false;
        }
        ctx->isCamOpen = true;
        spdlog::info("Camera {} '{}' opened", ctx->hcam, ctx->camName);

        // Read the version of the Device Driver
        if (!IsParamAvailable(ctx->hcam, PARAM_DD_VERSION, "PARAM_DD_VERSION"))
            return false;
        uns16 ddVersion;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_DD_VERSION, ATTR_CURRENT,
                                  (void*) &ddVersion))
        {
            PrintError("pl_get_param(PARAM_DD_VERSION) error");
            return false;
        }
        spdlog::info("  Device driver version: {}.{}.{}",
                     (ddVersion >> 8) & 0xFF, (ddVersion >> 4) & 0x0F,
                     (ddVersion >> 0) & 0x0F);

        // Historically, the chip name also included camera model name and it was commonly used
        // as camera identifier. On recent cameras, the camera model name should be read from
        // PARAM_PRODUCT_NAME.
        if (!IsParamAvailable(ctx->hcam, PARAM_CHIP_NAME, "PARAM_CHIP_NAME"))
            return false;
        char chipName[CCD_NAME_LEN];
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_CHIP_NAME, ATTR_CURRENT,
                                  (void*) chipName))
        {
            PrintError("pl_get_param(PARAM_CHIP_NAME) error");
            return false;
        }
        spdlog::info("  Sensor chip name: {}", chipName);

        // Read the camera firmware version
        if (!IsParamAvailable(ctx->hcam, PARAM_CAM_FW_VERSION,
                              "PARAM_CAM_FW_VERSION"))
            return false;
        uns16 fwVersion;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_CAM_FW_VERSION, ATTR_CURRENT,
                                  (void*) &fwVersion))
        {
            PrintError("pl_get_param(PARAM_CAM_FW_VERSION) error");
            return false;
        }
        // The camera major and minor firmware version is encoded in a 2-byte number.
        spdlog::info("  Camera firmware version: {}.{}",
                     (fwVersion >> 8) & 0xFF, (fwVersion >> 0) & 0xFF);

        // Read the camera sensor serial size (sensor width/number of columns)
        if (!IsParamAvailable(ctx->hcam, PARAM_SER_SIZE, "PARAM_SER_SIZE"))
            return false;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_SER_SIZE, ATTR_CURRENT,
                                  (void*) &ctx->sensorResX))
        {
            PrintError("Couldn't read CCD X-resolution");
            return false;
        }
        // Read the camera sensor parallel size (sensor height/number of rows)
        if (!IsParamAvailable(ctx->hcam, PARAM_PAR_SIZE, "PARAM_PAR_SIZE"))
            return false;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_PAR_SIZE, ATTR_CURRENT,
                                  (void*) &ctx->sensorResY))
        {
            PrintError("Couldn't read CCD Y-resolution");
            return false;
        }
        spdlog::info("  Sensor size: {}x{} px", ctx->sensorResX,
                     ctx->sensorResY);

        // Initialize the acquisition region to the full sensor size
        ctx->region.s1 = 0;
        ctx->region.s2 = ctx->sensorResX - 1;
        ctx->region.sbin = 1;
        ctx->region.p1 = 0;
        ctx->region.p2 = ctx->sensorResY - 1;
        ctx->region.pbin = 1;

        // Reset PP features to their default values.
        // Ignore errors, this call could fail if the camera does not
        // support post-processing.
        pl_pp_reset(ctx->hcam);

        spdlog::info("");

        // Build and cache the camera speed table
        if (!GetSpeedTable(ctx, ctx->speedTable)) return false;

        // Speed table has been created, print it out
        std::string table{"  Speed table:\n"};
        for (const auto& port: ctx->speedTable)
        {
            table.append(fmt::format("  - port '{}', value {}\n", port.name,
                                     port.value));
            for (const auto& speed: port.speeds)
            {
                table.append(fmt::format(
                        "    - speed index {}, running at {} MHz\n",
                        speed.index, 1000 / (float) speed.pixTimeNs));
                for (const auto& gain: speed.gains)
                {
                    table.append(fmt::format(
                            "      - gain index {}, {}bit-depth {} bpp\n",
                            gain.index,
                            (gain.name.empty()) ? "" : "'" + gain.name + "', ",
                            gain.bitDepth));
                }
            }
        }
        spdlog::info(table);

        // Initialize the camera to the first port, first speed and first gain

        if (PV_OK != pl_set_param(ctx->hcam, PARAM_READOUT_PORT,
                                  (void*) &ctx->speedTable[0].value))
        {
            PrintError("Readout port could not be set");
            return false;
        }
        spdlog::info("  Setting readout port to '{}'", ctx->speedTable[0].name);

        if (PV_OK != pl_set_param(ctx->hcam, PARAM_SPDTAB_INDEX,
                                  (void*) &ctx->speedTable[0].speeds[0].index))
        {
            PrintError("Readout port could not be set");
            return false;
        }
        spdlog::info("Setting readout speed index to {}",
                     ctx->speedTable[0].speeds[0].index);

        if (PV_OK !=
            pl_set_param(ctx->hcam, PARAM_GAIN_INDEX,
                         (void*) &ctx->speedTable[0].speeds[0].gains[0].index))
        {
            PrintError("Gain index could not be set");
            return false;
        }
        spdlog::info("  Setting gain index to {}",
                     ctx->speedTable[0].speeds[0].gains[0].index);

        spdlog::info("");

        // Set the number of sensor clear cycles to 2 (default).
        // This is mostly relevant to CCD cameras only and it has
        // no effect with CLEAR_NEVER or CLEAR_AUTO clearing modes
        // typically used with sCMOS cameras.
        if (!IsParamAvailable(ctx->hcam, PARAM_CLEAR_CYCLES,
                              "PARAM_CLEAR_CYCLES"))
            return false;
        uns16 clearCycles = 2;
        if (PV_OK !=
            pl_set_param(ctx->hcam, PARAM_CLEAR_CYCLES, (void*) &clearCycles))
        {
            PrintError("pl_set_param(PARAM_CLEAR_CYCLES) error");
            return false;
        }

        // Find out if the sensor is a frame transfer or other (typically interline)
        // type. This process is relevant for CCD cameras only.
        ctx->isFrameTransfer = false;
        rs_bool isFrameTransfer;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_FRAME_CAPABLE, ATTR_AVAIL,
                                  (void*) &isFrameTransfer))
        {
            isFrameTransfer = FALSE;
            PrintError("pl_get_param(PARAM_FRAME_CAPABLE) error");
            return false;
        }
        if (isFrameTransfer)
        {
            if (PV_OK != pl_get_param(ctx->hcam, PARAM_FRAME_CAPABLE,
                                      ATTR_CURRENT, (void*) &isFrameTransfer))
            {
                isFrameTransfer = FALSE;
                PrintError("pl_get_param(PARAM_FRAME_CAPABLE) error");
                return false;
            }
            ctx->isFrameTransfer = isFrameTransfer == TRUE;
        }
        if (ctx->isFrameTransfer)
        {
            spdlog::info("  Camera with Frame Transfer capability sensor");
        }
        else
        {
            spdlog::info("  Camera without Frame Transfer capability sensor");
        }

        // If this is a Frame Transfer sensor set PARAM_PMODE to PMODE_FT.
        // The other common mode for these sensors is PMODE_ALT_FT.
        if (!IsParamAvailable(ctx->hcam, PARAM_PMODE, "PARAM_PMODE"))
            return false;
        if (ctx->isFrameTransfer)
        {
            int32 PMode = PMODE_FT;
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_PMODE, (void*) &PMode))
            {
                PrintError("pl_set_param(PARAM_PMODE) error");
                return false;
            }
        }
        // If not a Frame Transfer sensor (i.e. Interline), set PARAM_PMODE to
        // PMODE_NORMAL, or PMODE_ALT_NORMAL.
        else
        {
            int32 PMode = PMODE_NORMAL;
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_PMODE, (void*) &PMode))
            {
                PrintError("pl_set_param(PARAM_PMODE) error");
                return false;
            }
        }

        // Check if the camera supports Smart Streaming feature.
        if (!IsParamAvailable(ctx->hcam, PARAM_SMART_STREAM_MODE,
                              "PARAM_SMART_STREAM_MODE"))
        {
            spdlog::info("  Smart Streaming is not available");
            ctx->isSmartStreaming = false;
        }
        else
        {
            spdlog::info("  Smart Streaming is available");
            ctx->isSmartStreaming = true;
        }

        if (!IsParamAvailable(ctx->hcam, PARAM_ROI_COUNT, "PARAM_ROI_COUNT"))
        {
            CloseAllCamerasAndUninit();
            return false;
        }
        uns16 roiCount;
        // ATTR_COUNT or ATTR_MAX can be checked here, both give the same value
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_ROI_COUNT, ATTR_MAX,
                                  (void*) &roiCount))
        {
            PrintError("pl_get_param(PARAM_ROI_COUNT) error");
            CloseAllCamerasAndUninit();
            return false;
        }
        spdlog::info("ROI count: {}", roiCount);

        return true;
    }

    void PhotometricsBackend::CloseCamera(std::unique_ptr<CameraContext>& ctx)
    {
        if (!ctx->isCamOpen) { return; }

        if (PV_OK != pl_cam_close(ctx->hcam))
        {
            PrintError("pl_cam_close() error");
        }
        else
        {
            spdlog::info("Camera {} '{}' closed\n", ctx->hcam, ctx->camName);
        }
        ctx->isCamOpen = false;
    }

    void PhotometricsBackend::CloseAllCamerasAndUninit()
    {
        printf("\n");

        for (auto& ctx: m_cameraContexts) { CloseCamera(ctx); }

        UninitPVCAM();
    }

    void PhotometricsBackend::Init()
    {
        if (m_isPvcamInitialized)
        {
            spdlog::info("Already initialized");
            return;
        }

        if (!InitPVCAM())
        {
            spdlog::error("Couldn't init PVCAM");
            return;
        }

        auto& ctx = m_cameraContexts[m_cameraIndex];

        ctx->thread = std::make_unique<std::jthread>(
                &PhotometricsBackend::Init_, this);
    }

    bool PhotometricsBackend::InitAndOpenOneCamera()
    {
        if (m_cameraIndex >= m_cameraContexts.size())
        {
            PrintError("Camera index #{} is invalid", m_cameraIndex);
            UninitPVCAM();
            return false;
        }

        if (!OpenCamera(m_cameraContexts.at(m_cameraIndex)))
        {
            CloseCamera(m_cameraContexts.at(m_cameraIndex));
            UninitPVCAM();
            return false;
        }

        return true;
    }

    bool PhotometricsBackend::ReadEnumeration(int16 hcam, NVPC* pNvpc,
                                              uns32 paramID,
                                              const char* paramName)
    {
        if (!pNvpc || !paramName) { return false; }

        if (!IsParamAvailable(hcam, paramID, paramName)) { return false; }

        uns32 count;
        if (PV_OK != pl_get_param(hcam, paramID, ATTR_COUNT, (void*) &count))
        {
            PrintError("pl_get_param({}) error", paramName);
            return false;
        }

        NVPC nvpc;
        for (uns32 i = 0; i < count; ++i)
        {
            // Retrieve the enum string length
            uns32 strLength;
            if (PV_OK != pl_enum_str_length(hcam, paramID, i, &strLength))
            {
                PrintError("pl_enum_str_length({}) error", paramName);
                return false;
            }

            // Allocate the destination string
            char* name = new (std::nothrow) char[strLength];
            if (!name)
            {
                PrintError("Unable to allocate memory for {} enum item name",
                           paramName);
                return false;
            }

            // Get the string and value
            int32 value;
            if (PV_OK !=
                pl_get_enum_param(hcam, paramID, i, &value, name, strLength))
            {
                PrintError("pl_get_enum_param({}) error", paramName);
                delete[] name;
                return false;
            }

            NVP nvp;
            nvp.value = value;
            nvp.name = name;
            nvpc.push_back(nvp);

            delete[] name;
        }
        pNvpc->swap(nvpc);

        return !pNvpc->empty();
    }

    bool PhotometricsBackend::IsParamAvailable(int16 hcam, uns32 paramID,
                                               const char* paramName)
    {

        if (paramName == nullptr) { return PV_FAIL; }

        rs_bool isAvailable;
        if (PV_OK !=
            pl_get_param(hcam, paramID, ATTR_AVAIL, (void*) &isAvailable))
        {
            PrintError("Error reading ATTR_AVAIL of {}", paramName);
            return PV_FAIL;
        }
        if (isAvailable == FALSE)
        {
            spdlog::info("Parameter {} is not available", paramName);
            return PV_FAIL;
        }

        return PV_OK;
    }

    void PhotometricsBackend::UpdateCtxImageFormat(
            std::unique_ptr<CameraContext>& ctx)
    {
        ctx->imageFormat = PL_IMAGE_FORMAT_MONO8;

        rs_bool isAvailable;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_IMAGE_FORMAT, ATTR_AVAIL,
                                  (void*) &isAvailable))
            return;
        if (isAvailable == FALSE) return;

        int32 imageFormat;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_IMAGE_FORMAT, ATTR_CURRENT,
                                  (void*) &imageFormat))
            return;

        ctx->imageFormat = imageFormat;
    }

    bool PhotometricsBackend::SelectCameraExpMode(
            const std::unique_ptr<CameraContext>& ctx, int16& expMode,
            int16 legacyTrigMode, int16 extendedTrigMode)
    {
        NVPC triggerModes;
        if (!ReadEnumeration(ctx->hcam, &triggerModes, PARAM_EXPOSURE_MODE,
                             "PARAM_EXPOSURE_MODE"))
        {
            return false;
        }
        // Try to find the legacy mode first
        for (const NVP& nvp: triggerModes)
        {
            if (nvp.value == legacyTrigMode)
            {
                // If this is a legacy (mostly CCD) camera, return the legacy mode
                expMode = legacyTrigMode;
                return true;
            }
        }

        // If not, select the extended mode and choose the first expose-out mode.
        for (const NVP& nvp: triggerModes)
        {
            if (nvp.value == extendedTrigMode)
            {
                // Modern cameras should all support the expose-out mode, but let's make sure.
                if (!IsParamAvailable(ctx->hcam, PARAM_EXPOSE_OUT_MODE,
                                      "PARAM_EXPOSE_OUT_MODE"))
                {
                    expMode = extendedTrigMode;
                    return true;
                }
                // Select the first available expose-out mode. For the SDK example purposes, the
                // expose-out mode is not crucial as it controls the expose-out hardware signal.
                NVPC expOutModes;
                if (!ReadEnumeration(ctx->hcam, &expOutModes,
                                     PARAM_EXPOSE_OUT_MODE,
                                     "PARAM_EXPOSE_OUT_MODE"))
                {
                    return false;
                }
                // Select the first one
                const auto expOutMode =
                        static_cast<int16>(expOutModes[0].value);
                // And return our final 'exp' mode that can be used in pl_exp_setup functions.
                // The final mode is an 'or-ed' value of exposure (trigger) mode and expose-out mode.
                expMode = extendedTrigMode | expOutMode;
                return true;
            }
        }

        // If nothing was selected in the previous loop, then something had to fail.
        // This is a serious and unlikely error. The camera must support either
        // the legacy mode or the new extended trigger mode.
        PrintError("ERROR: Failed to select camera exposure mode!");
        return false;
    }

    void PhotometricsBackend::SequenceCapture(uint32_t nFrames,
                                              SAVE_FORMAT format, bool save)
    {
        if (!m_isPvcamInitialized)
        {
            spdlog::warn("Initialize PVCAM first");
            return;
        }

        auto& ctx = m_cameraContexts[m_cameraIndex];
        if (!ctx->isCamOpen)
        {
            spdlog::warn("Camera not opened");
            return;
        }

        if (ctx->isCapturing)
        {
            spdlog::warn("Already capturing");
            return;
        }

        ctx->threadAbortFlag = false;
        ctx->thread = std::make_unique<std::jthread>(
                &PhotometricsBackend::SequenceCapture_, this, nFrames, format,
                save);
    }

    void PhotometricsBackend::LiveCapture(SAVE_FORMAT format, bool save)
    {
        if (!m_isPvcamInitialized)
        {
            spdlog::warn("Initialize PVCAM first");
            return;
        }

        auto& ctx = m_cameraContexts[m_cameraIndex];
        if (!ctx->isCamOpen)
        {
            spdlog::warn("Camera not opened");
            return;
        }

        if (ctx->isCapturing)
        {
            spdlog::warn("Already capturing");
            return;
        }

        ctx->threadAbortFlag = false;
        ctx->thread = std::make_unique<std::jthread>(
                &PhotometricsBackend::LiveCapture_, this, format, save);
    }

    void PhotometricsBackend::TerminateCapture()
    {
        spdlog::info("\n>>>\n"
                     ">>> CLI TERMINATION HANDLER");

        for (auto& ctx: m_cameraContexts)
        {
            if (!ctx || !ctx->isCamOpen) { continue; }

            {
                std::scoped_lock lock(ctx->eofEvent.mutex);
                if (ctx->threadAbortFlag) { continue; }
                ctx->threadAbortFlag = true;
                spdlog::info(">>> Requesting ABORT on camera {}\n", ctx->hcam);
            }
            ctx->eofEvent.cond.notify_all();
        }
        spdlog::info(">>>\n\n");
    }

    void PhotometricsBackend::CustomEofHandler(FRAME_INFO* pFrameInfo,
                                               void* pContext)
    {
        if (!pFrameInfo || !pContext) return;
        auto ctx = static_cast<CameraContext*>(pContext);

        // Store the frame information for later use on the main thread
        ctx->eofFrameInfo = *pFrameInfo;

        // Obtain a pointer to the last acquired frame
        if (PV_OK != pl_exp_get_latest_frame(ctx->hcam, &ctx->eofFrame))
        {
            PrintError("pl_exp_get_latest_frame() error");
            ctx->eofFrame = nullptr;
        }

        // Unblock the acquisition thread
        {
            std::lock_guard<std::mutex> lock(ctx->eofEvent.mutex);
            ctx->eofEvent.flag = true;
        }
        ctx->eofEvent.cond.notify_all();
    }

    bool PhotometricsBackend::WaitForEofEvent(CameraContext* ctx,
                                              uns32 timeoutMs,
                                              bool& errorOccurred)
    {
        std::unique_lock<std::mutex> lock(ctx->eofEvent.mutex);

        errorOccurred = false;
        ctx->eofEvent.cond.wait_for(
                lock, std::chrono::milliseconds(timeoutMs),
                [ctx]() { return ctx->eofEvent.flag || ctx->threadAbortFlag; });
        if (ctx->threadAbortFlag)
        {
            spdlog::info("Processing aborted on camera {}\n", ctx->hcam);
            return false;
        }
        if (!ctx->eofEvent.flag)
        {
            spdlog::error("Camera {} timed out waiting for a frame\n",
                          ctx->hcam);
            errorOccurred = true;
            return false;
        }
        ctx->eofEvent.flag = false;// Reset flag

        if (!ctx->eofFrame)
        {
            errorOccurred = true;
            return false;
        }

        return true;
    }

    void PhotometricsBackend::Init_()
    {
        spdlog::info("Starting init");
        InitAndOpenOneCamera();
    }

    void PhotometricsBackend::SequenceCapture_(uint32_t nFrames,
                                               SAVE_FORMAT format, bool save)
    {
        const auto videoPath = FileUtils::GenerateVideoPath(
                m_saveDirPath, SEQ_CAPTURE_PREFIX, format);
        if (videoPath.empty())
        {
            spdlog::error("Couldn't generate videopath");
            return;
        }

        auto& ctx = m_cameraContexts[0];

        std::vector<std::uint8_t> bytes;

        if (PV_OK != pl_cam_register_callback_ex3(ctx->hcam, PL_CALLBACK_EOF,
                                                  (void*) CustomEofHandler,
                                                  (void*) ctx.get()))
        {
            PrintError("pl_cam_register_callback() error");
            CloseAllCamerasAndUninit();
            return;
        }

        uns32 exposureBytes;
        // Select the appropriate internal trigger mode for this camera.
        int16 expMode;
        if (!SelectCameraExpMode(ctx, expMode, TIMED_MODE, EXT_TRIG_INTERNAL))
        {
            CloseAllCamerasAndUninit();
            return;
        }

        /**
    Prepare the acquisition. The pl_exp_setup_seq() function returns the size of the
    frame buffer required for the entire frame sequence. Since we are requesting a
    single frame only, size of one frame will be reported.
    */
        if (PV_OK != pl_exp_setup_seq(ctx->hcam, 1, 1, &ctx->region, expMode,
                                      ctx->exposureTime, &exposureBytes))
        {
            PrintError("pl_exp_setup_seq() error");
            CloseAllCamerasAndUninit();
            return;
        }
        UpdateCtxImageFormat(ctx);

        uint16_t actualImageWidth =
                (ctx->region.s2 - ctx->region.s1 + 1) / ctx->region.sbin;
        uint16_t actualImageHeight =
                (ctx->region.p2 - ctx->region.p1 + 1) / ctx->region.pbin;

        const auto bitDepth = ctx->speedTable[0].speeds[0].gains[0].bitDepth;
        spdlog::info("Bit depth for camera: {}", bitDepth);
        // Allocate a buffer of the size reported by the pl_exp_setup_seq() function.
        uns8* frameInMemory = new (std::nothrow) uns8[exposureBytes];
        if (!frameInMemory)
        {
            spdlog::error("Unable to allocate buffer for camera {}\n",
                          ctx->hcam);
            CloseAllCamerasAndUninit();
            return;
        }

        bool errorOccurred = false;
        uns32 imageCounter = 0;
        ctx->isCapturing = true;

        spdlog::info("Starting sequence capture loop on cam {}\n", ctx->hcam);
        std::vector<double> captureTimes{};
        while (imageCounter < nFrames)
        {
            Timer timer{};
            /**
        Start the acquisition. Since the pl_exp_setup_seq() was configured to use
        the internal camera trigger, the acquisition is started immediately.
        In hardware triggering modes the camera would wait for an external trigger signal.
        */
            if (PV_OK != pl_exp_start_seq(ctx->hcam, frameInMemory))
            {
                PrintError("pl_exp_start_seq() error");
                errorOccurred = true;
                break;
            }

            /**
        Here we need to wait for a frame readout notification signaled by the eofEvent
        in the CameraContext which is raised in the callback handler we registered.
        If the frame does not arrive within 5 seconds, or if the user aborts the acquisition
        with ctrl+c shortcut, the main 'while' loop is interrupted and the acquisition is
        aborted.
        */
            if (!WaitForEofEvent(ctx.get(), 5000, errorOccurred)) break;

            void* frame;
            if (pl_exp_get_latest_frame(ctx->hcam, &frame) != PV_OK)
            {
                spdlog::error("Jeff");
                PrintError("Couldn't get latest frame");
                continue;
            }

            spdlog::info("Frame #{} acquired, timestamp = {}\n", imageCounter,
                         100 * ctx->eofFrameInfo.TimeStamp);

            const auto [itMin, itMax] = std::minmax_element(
                    (uint16_t*) ctx->eofFrame,
                    (uint16_t*) ctx->eofFrame +
                            actualImageHeight * actualImageWidth);

            m_minCurrentValue = *itMin;
            m_maxCurrentValue = *itMax;

            if (save)
            {
                std::copy((uint8_t*) ctx->eofFrame,
                          (uint8_t*) ctx->eofFrame + exposureBytes,
                          std::back_inserter(bytes));
            }

            sf::Image image = PVCamImageToSfImage(
                    (uint16_t*) ctx->eofFrame, actualImageWidth,
                    actualImageHeight, m_minDisplayValue, m_maxDisplayValue);

            std::scoped_lock lock(m_textureMutex);
            m_currentTexture.loadFromImage(image);
            //TODO sleep from framerate
            /**
        When acquiring sequences, call the pl_exp_finish_seq() after the entire sequence
        finishes. This call is not strictly necessary, especially with the latest camera models,
        therefore, if fast, repeated calls to pl_exp_start_seq() are needed, this call can
        be omitted. Omitting the call may increase frame rate with the 'emulated' software triggering
        where pl_exp_start_seq() is usually called in a loop. However, testing is recommended
        to ensure desired acquisition stability.
        */
            if (PV_OK != pl_exp_finish_seq(ctx->hcam, frameInMemory, 0))
            {
                PrintError("pl_exp_finish_seq() error");
            }
            else
            {
                spdlog::info("Acquisition finished on camera {}\n", ctx->hcam);
            }

            imageCounter++;
            captureTimes.push_back(timer.stop());
        }
        ctx->isCapturing = false;

        const auto totalCaptureTime =
                std::accumulate(captureTimes.begin(), captureTimes.end(), 0.0);
        const auto fps = captureTimes.size() / totalCaptureTime;

        const auto frametimeAvg = 1 / fps;
        spdlog::info("Captured {} frames in {} seconds\nAvg fps: {}",
                     imageCounter, totalCaptureTime, fps);
        /**
    Here the pl_exp_abort() is not strictly required as correctly acquired sequence does not
    need to be aborted. However, it is kept here for situations where the acquisition
    is interrupted by the user or when it unexpectedly times out.
    */
        if (PV_OK != pl_exp_abort(ctx->hcam, CCS_HALT))
        {
            PrintError("pl_exp_abort() error");
        }

        // Cleanup before exiting the application.
        delete[] frameInMemory;

        if (save)
        {
            if (!std::filesystem::create_directory(
                        std::filesystem::path{videoPath}))
            {
                spdlog::error("Couldn't create a directory");
            }

            auto meta = TifStackMeta{
                    .numFrames = imageCounter,
                    .exposure = ctx->exposureTime,
                    .fps = fps,
                    .frametimeAvg = frametimeAvg,
                    .frametimeMin = *std::min_element(captureTimes.begin(),
                                                      captureTimes.end()),
                    .frametimeMax = *std::max_element(captureTimes.begin(),
                                                      captureTimes.end()),
                    .binning = ctx->region.pbin == 1 ? ONE : TWO,
                    .lens = ctx->lens};

            meta.frametimeStd = std::sqrt(
                    std::accumulate(
                            captureTimes.begin(), captureTimes.end(), 0.0,
                            [&meta](double a, double b) {
                                return a + (b - meta.frametimeAvg) *
                                                   (b - meta.frametimeAvg);
                            }) /
                    captureTimes.size());

            if (!FileUtils::WriteTifMetadata(videoPath, meta) ||
                !FileUtils::WritePvcamStack(bytes.data(), actualImageWidth,
                                            actualImageHeight, exposureBytes,
                                            videoPath, imageCounter))
            {
                spdlog::error("Failed writing stack to {}", videoPath);
            }
            else
            {
                spdlog::info("Stack written to {}", videoPath);
            }
        }
    }

    void PhotometricsBackend::LiveCapture_(SAVE_FORMAT format, bool save)
    {
        const auto videoPath = FileUtils::GenerateVideoPath(
                m_saveDirPath, LIVE_CAPTURE_PREFIX, format);
        if (videoPath.empty())
        {
            spdlog::error("Couldn't generate videopath");
            return;
        }

        auto& ctx = m_cameraContexts[0];

        std::vector<uint8_t> bytes;

        if (PV_OK != pl_cam_register_callback_ex3(ctx->hcam, PL_CALLBACK_EOF,
                                                  (void*) CustomEofHandler,
                                                  (void*) ctx.get()))
        {
            PrintError("pl_cam_register_callback() error");
            CloseAllCamerasAndUninit();
            return;
        }
        spdlog::info("EOF callback handler registered on camera {}\n",
                     ctx->hcam);

        uns32 exposureBytes;

        const uns16 circBufferFrames = 20;
        int16 bufferMode = CIRC_OVERWRITE;

        // Select the appropriate internal trigger mode for this camera.
        int16 expMode;
        if (!SelectCameraExpMode(ctx, expMode, TIMED_MODE, EXT_TRIG_INTERNAL))
        {
            CloseAllCamerasAndUninit();
            return;
        }
        /**
        Prepare the continuous acquisition with circular buffer mode. The
        pl_exp_setup_cont() function returns the size of one frame (unlike
        the pl_exp_setup_seq() that returns a buffer size for the entire sequence).
        */
        if (PV_OK != pl_exp_setup_cont(ctx->hcam, 1, &ctx->region, expMode,
                                       ctx->exposureTime, &exposureBytes,
                                       bufferMode))
        {
            PrintError("pl_exp_setup_cont() error\n");
            CloseAllCamerasAndUninit();
            return;
        }
        spdlog::info("Acquisition setup successful on camera {}\n", ctx->hcam);
        UpdateCtxImageFormat(ctx);

        uint16_t actualImageWidth =
                (ctx->region.s2 - ctx->region.s1 + 1) / ctx->region.sbin;
        uint16_t actualImageHeight =
                (ctx->region.p2 - ctx->region.p1 + 1) / ctx->region.pbin;

        const uns32 circBufferBytes = circBufferFrames * exposureBytes;
        /**
        Now allocate the buffer memory. The application is in control of the
        circular buffer and should allocate memory of appropriate size.
        */
        uns8* circBufferInMemory = new (std::nothrow) uns8[circBufferBytes];
        if (!circBufferInMemory)
        {
            PrintError("Unable to allocate buffer for camera {}\n", ctx->hcam);
            CloseAllCamerasAndUninit();
            return;
        }
        /**
        Start the continuous acquisition. By passing the entire size of the buffer
        to pl_exp_start_cont() function, PVCAM can calculate the capacity of the circular buffer.
        */
        if (PV_OK !=
            pl_exp_start_cont(ctx->hcam, circBufferInMemory, circBufferBytes))
        {
            PrintError("pl_exp_start_cont() error\n");
            CloseAllCamerasAndUninit();
            delete[] circBufferInMemory;
            return;
        }
        spdlog::info("Acquisition started on camera {}\n", ctx->hcam);

        uns32 imageCounter = 0;
        bool errorOccurred = false;
        ctx->isCapturing = true;

        std::vector<double> captureTimes{};
        while (true)
        {
            Timer timer{};
            /**
        Here we need to wait for a frame readout notification signaled by the eofEvent
        in the CameraContext which is raised in the callback handler we registered.
        If the frame does not arrive within 5 seconds or if user aborts the acquisition
        with ctrl+c keyboard shortcut, the main 'while' loop is interrupted and the
        acquisition is aborted.
        */
            spdlog::info("Waiting for EOF event to occur on camera {}\n",
                         ctx->hcam);
            if (!WaitForEofEvent(ctx.get(), 5000, errorOccurred)) break;

            void* frame;
            if (pl_exp_get_latest_frame(ctx->hcam, &frame) != PV_OK)
            {
                PrintError("Couldn't get latest frame");
                continue;
            }

            // Timestamp is in hundreds of microseconds
            spdlog::info("Frame #{} acquired, timestamp = {}\n",
                         ctx->eofFrameInfo.FrameNr,
                         100 * ctx->eofFrameInfo.TimeStamp);

            const auto [itMin, itMax] = std::minmax_element(
                    (uint16_t*) frame,
                    (uint16_t*) frame + actualImageHeight * actualImageWidth);

            m_minCurrentValue = *itMin;
            m_maxCurrentValue = *itMax;

            if (save)
            {
                std::copy((uint8_t*) frame, (uint8_t*) frame + exposureBytes,
                          std::back_inserter(bytes));
            }

            sf::Image image = PVCamImageToSfImage(
                    (uint16_t*) frame, actualImageWidth, actualImageHeight,
                    m_minDisplayValue, m_maxDisplayValue);

            std::scoped_lock lock(m_textureMutex);
            m_currentTexture.loadFromImage(image);
            //TODO sleep from framerate

            imageCounter++;
            captureTimes.push_back(timer.stop());
        }
        ctx->isCapturing = false;

        const auto totalCaptureTime =
                std::accumulate(captureTimes.begin(), captureTimes.end(), 0.0);
        const auto fps = captureTimes.size() / totalCaptureTime;

        const auto frametimeAvg = 1 / fps;
        spdlog::info("Captured {} frames in {} seconds\nAvg fps: {}",
                     imageCounter, totalCaptureTime, fps);

        if (PV_OK != pl_exp_abort(ctx->hcam, CCS_HALT))
        {
            PrintError("pl_exp_abort() error");
        }
        else
        {
            spdlog::info("Acquisition stopped on camera {}\n", ctx->hcam);
        }

        delete[] circBufferInMemory;


        if (save)
        {
            if (!std::filesystem::create_directory(
                        std::filesystem::path{videoPath}))
            {
                spdlog::error("Couldn't create a directory");
            }

            auto meta = TifStackMeta{
                    .numFrames = imageCounter,
                    .exposure = ctx->exposureTime,
                    .fps = fps,
                    .frametimeAvg = frametimeAvg,
                    .frametimeMin = *std::min_element(captureTimes.begin(),
                                                      captureTimes.end()),
                    .frametimeMax = *std::max_element(captureTimes.begin(),
                                                      captureTimes.end()),
                    .binning = ctx->region.pbin == 1 ? ONE : TWO,
                    .lens = ctx->lens};

            meta.frametimeStd = std::sqrt(
                    std::accumulate(
                            captureTimes.begin(), captureTimes.end(), 0.0,
                            [&meta](double a, double b) {
                                return a + (b - meta.frametimeAvg) *
                                                   (b - meta.frametimeAvg);
                            }) /
                    captureTimes.size());

            if (!FileUtils::WriteTifMetadata(videoPath, meta) ||
                !FileUtils::WritePvcamStack(bytes.data(), actualImageWidth,
                                            actualImageHeight, exposureBytes,
                                            videoPath, imageCounter))
            {
                spdlog::error("Failed writing stack to {}", videoPath);
            }
            else
            {
                spdlog::info("Stack written to {}", videoPath);
            }
        }
    }

    bool PhotometricsBackend::SubtractBackground(uint8_t* bytes, uint16_t width,
                                                 uint16_t height,
                                                 uint32_t nFrames)
    {
        const auto frameSizeU16 = width * height;
        for (std::size_t i = 0; i < nFrames; ++i)
        {
            cv::Mat mat{height, width, CV_16U,
                        (uint16_t*) bytes + i * frameSizeU16};
            cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                        cv::Size{15, 15});
            cv::morphologyEx(mat, mat, cv::MORPH_TOPHAT, element,
                             cv::Point{-1, -1});

            std::copy(mat.data, mat.data + 2 * frameSizeU16,
                      bytes + i * 2 * frameSizeU16);
        }

        return true;
    }
}// namespace prm
