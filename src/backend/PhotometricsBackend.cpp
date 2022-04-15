#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "backend/PhotometricsBackend.h"


namespace slr {
    template<typename... Args>
    void PhotometricsBackend::PrintError(fmt::format_string<Args...> fmt, Args &&...args) {
        auto code = pl_error_code();
        char pvcamErrMsg[ERROR_MSG_LEN];
        pl_error_message(code, pvcamErrMsg);

        auto str = fmt::format("\n-----Error code {}-----\n"
                               "{}\n", code, pvcamErrMsg);

        auto str1 = fmt::vformat(fmt, fmt::make_format_args(args...));

        auto str2 = fmt::format("------------------------\n");
        spdlog::error("{}{}{}", str, str1, str2);
    }

    bool PhotometricsBackend::ShowAppInfo(int argc, char *argv[]) {
        std::scoped_lock lock(mPvcamMutex);

        auto appName = "<unable to get name>";
        if (argc > 0 && argv != nullptr && argv[0] != nullptr) {
            appName = argv[0];
        }

        // Read PVCAM library version
        uns16 pvcamVersion{};
        if (PV_OK != pl_pvcam_get_ver(&pvcamVersion)) {
            PrintError("pl_pvcam_get_ver() error");
            return PV_FAIL;
        }

        spdlog::info("\n************************************************************\n"
                     "Application  : {}\n"
                     "PVCAM version: {}.{}.{}\n"
                     "************************************************************\n",
                     appName, (pvcamVersion >> 8) & 0xFF,
                     (pvcamVersion >> 4) & 0x0F,
                     (pvcamVersion >> 0) & 0x0F);

        return PV_OK;
    }

    bool PhotometricsBackend::InitPVCAM() {
        std::scoped_lock lock(mPvcamMutex);

        if (mIsPvcamInitialized) {
            return true;
        }

        // Initialize PVCAM library
        if (PV_OK != pl_pvcam_init()) {
            PrintError("pl_pvcam_init() error");
            return false;
        }

        mIsPvcamInitialized = true;
        spdlog::info("PVCAM initialized");

        mCameraContexts.clear();
        int16 nrOfCameras;

        // Read the number of cameras in the system.
        // This will return total number of PVCAM cameras regardless of interface.
        if (PV_OK != pl_cam_get_total(&nrOfCameras)) {
            PrintError("pl_cam_get_total() error\n");
            UninitPVCAM();
            return false;
        }

        // Exit if no cameras have been found
        if (nrOfCameras <= 0) {
            PrintError("No cameras found in the system");
            UninitPVCAM();
            return false;
        }
        spdlog::info("Number of cameras found: {}", nrOfCameras);

        // Create context structure for all cameras and fill in the PVCAM camera names
        mCameraContexts.reserve(nrOfCameras);
        for (int16 i = 0; i < nrOfCameras; i++) {
            auto ctx = std::make_unique<CameraContext>();
            if (!ctx) {
                PrintError("Unable to allocate memory for CameraContext");
                UninitPVCAM();
                return false;
            }

            // Obtain PVCAM-name for this particular camera
            if (PV_OK != pl_cam_get_name(i, ctx->camName)) {
                PrintError("pl_cam_get_name() error\n");
                UninitPVCAM();
                return false;
            }
            spdlog::info("Camera {} name: '{}'", i, ctx->camName);

            mCameraContexts.push_back(std::move(ctx));
        }

        return true;
    }

    void PhotometricsBackend::UninitPVCAM() {
        if (!mIsPvcamInitialized) {
            return;
        }

        mCameraContexts.clear();

        // Uninitialize the PVCAM library
        if (PV_OK != pl_pvcam_uninit()) {
            printf("pl_pvcam_uninit() error\n");
        } else {
            printf("PVCAM uninitialized\n");
        }
        mIsPvcamInitialized = false;
    }

    bool
    PhotometricsBackend::GetSpeedTable(const std::unique_ptr<CameraContext> &ctx, std::vector<SpdtabPort> &speedTable) {
        std::vector<SpdtabPort> table;

        NVPC ports;
        if (!ReadEnumeration(ctx->hcam, &ports, PARAM_READOUT_PORT, "PARAM_READOUT_PORT"))
            return false;

        if (!IsParamAvailable(ctx->hcam, PARAM_SPDTAB_INDEX, "PARAM_SPDTAB_INDEX"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_PIX_TIME, "PARAM_PIX_TIME"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_GAIN_INDEX, "PARAM_GAIN_INDEX"))
            return false;
        if (!IsParamAvailable(ctx->hcam, PARAM_BIT_DEPTH, "PARAM_BIT_DEPTH"))
            return false;

        rs_bool isGainNameAvailable;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_NAME, ATTR_AVAIL,
                                  (void *) &isGainNameAvailable)) {
            PrintError("Error reading ATTR_AVAIL of PARAM_GAIN_NAME");
            return false;
        }
        const bool isGainNameSupported = isGainNameAvailable != FALSE;

        // Iterate through available ports and their speeds
        for (size_t pi = 0; pi < ports.size(); pi++) {
            // Set readout port
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_READOUT_PORT,
                                      (void *) &ports[pi].value)) {
                PrintError(
                        "pl_set_param(PARAM_READOUT_PORT) error");
                return false;
            }

            // Get number of available speeds for this port
            uns32 speedCount;
            if (PV_OK != pl_get_param(ctx->hcam, PARAM_SPDTAB_INDEX, ATTR_COUNT,
                                      (void *) &speedCount)) {
                PrintError(
                        "pl_get_param(PARAM_SPDTAB_INDEX) error");
                return false;
            }

            SpdtabPort port;
            port.value = ports[pi].value;
            port.name = ports[pi].name;

            // Iterate through all the speeds
            for (int16 si = 0; si < (int16) speedCount; si++) {
                // Set camera to new speed index
                if (PV_OK != pl_set_param(ctx->hcam, PARAM_SPDTAB_INDEX, (void *) &si)) {
                    PrintError(
                            "pl_set_param(PARAM_SPDTAB_INDEX) error");
                    return false;
                }

                // Get pixel time (readout time of one pixel in nanoseconds) for the
                // current port/speed pair. This can be used to calculate readout
                // frequency of the port/speed pair.
                uns16 pixTime;
                if (PV_OK != pl_get_param(ctx->hcam, PARAM_PIX_TIME, ATTR_CURRENT,
                                          (void *) &pixTime)) {
                    PrintError(
                            "pl_get_param(PARAM_PIX_TIME) error");
                    return false;
                }

                uns32 gainCount;
                if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_INDEX, ATTR_COUNT,
                                          (void *) &gainCount)) {
                    PrintError(
                            "pl_get_param(PARAM_GAIN_INDEX) error");
                    return false;
                }

                SpdtabSpeed speed;
                speed.index = si;
                speed.pixTimeNs = pixTime;

                // Iterate through all the gains, notice it starts at value 1!
                for (int16 gi = 1; gi <= (int16) gainCount; gi++) {
                    // Set camera to new gain index
                    if (PV_OK != pl_set_param(ctx->hcam, PARAM_GAIN_INDEX, (void *) &gi)) {
                        PrintError(
                                "pl_set_param(PARAM_GAIN_INDEX) error");
                        return false;
                    }

                    // Get bit depth for the current gain
                    int16 bitDepth;
                    if (PV_OK != pl_get_param(ctx->hcam, PARAM_BIT_DEPTH, ATTR_CURRENT,
                                              (void *) &bitDepth)) {
                        PrintError(
                                "pl_get_param(PARAM_BIT_DEPTH) error");
                        return false;
                    }

                    SpdtabGain gain;
                    gain.index = gi;
                    gain.bitDepth = bitDepth;

                    if (isGainNameSupported) {
                        char gainName[MAX_GAIN_NAME_LEN];
                        if (PV_OK != pl_get_param(ctx->hcam, PARAM_GAIN_NAME,
                                                  ATTR_CURRENT, (void *) gainName)) {
                            PrintError(
                                    "pl_get_param(PARAM_GAIN_NAME) error");
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

    bool PhotometricsBackend::OpenCamera(std::unique_ptr<CameraContext> &ctx) {
        if (ctx->isCamOpen)
            return true;

        // Open a camera with a given name and retrieve its handle. From now on,
        // only the handle will be used when referring to this particular camera.
        // The name is obtained in InitPVCAM() function when listing all cameras
        // in the system.
        if (PV_OK != pl_cam_open(ctx->camName, &ctx->hcam, OPEN_EXCLUSIVE)) {
            PrintError("pl_cam_open() error");
            return false;
        }
        ctx->isCamOpen = true;
        spdlog::info("Camera {} '{}' opened", ctx->hcam, ctx->camName);

        // Read the version of the Device Driver
        if (!IsParamAvailable(ctx->hcam, PARAM_DD_VERSION, "PARAM_DD_VERSION"))
            return false;
        uns16 ddVersion;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_DD_VERSION, ATTR_CURRENT, (void *) &ddVersion)) {
            PrintError("pl_get_param(PARAM_DD_VERSION) error");
            return false;
        }
        spdlog::info("  Device driver version: {}.{}.{}",
                     (ddVersion >> 8) & 0xFF,
                     (ddVersion >> 4) & 0x0F,
                     (ddVersion >> 0) & 0x0F);

        // Historically, the chip name also included camera model name and it was commonly used 
        // as camera identifier. On recent cameras, the camera model name should be read from 
        // PARAM_PRODUCT_NAME. 
        if (!IsParamAvailable(ctx->hcam, PARAM_CHIP_NAME, "PARAM_CHIP_NAME"))
            return false;
        char chipName[CCD_NAME_LEN];
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_CHIP_NAME, ATTR_CURRENT, (void *) chipName)) {
            PrintError("pl_get_param(PARAM_CHIP_NAME) error");
            return false;
        }
        spdlog::info("  Sensor chip name: {}", chipName);

        // Read the camera firmware version
        if (!IsParamAvailable(ctx->hcam, PARAM_CAM_FW_VERSION, "PARAM_CAM_FW_VERSION"))
            return false;
        uns16 fwVersion;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_CAM_FW_VERSION, ATTR_CURRENT,
                                  (void *) &fwVersion)) {
            PrintError(
                    "pl_get_param(PARAM_CAM_FW_VERSION) error");
            return false;
        }
        // The camera major and minor firmware version is encoded in a 2-byte number.
        spdlog::info("  Camera firmware version: {}.{}",
                     (fwVersion >> 8) & 0xFF,
                     (fwVersion >> 0) & 0xFF);

        // Read the camera sensor serial size (sensor width/number of columns)
        if (!IsParamAvailable(ctx->hcam, PARAM_SER_SIZE, "PARAM_SER_SIZE"))
            return false;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_SER_SIZE, ATTR_CURRENT,
                                  (void *) &ctx->sensorResX)) {
            PrintError("Couldn't read CCD X-resolution");
            return false;
        }
        // Read the camera sensor parallel size (sensor height/number of rows)
        if (!IsParamAvailable(ctx->hcam, PARAM_PAR_SIZE, "PARAM_PAR_SIZE"))
            return false;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_PAR_SIZE, ATTR_CURRENT,
                                  (void *) &ctx->sensorResY)) {
            PrintError("Couldn't read CCD Y-resolution");
            return false;
        }
        spdlog::info("  Sensor size: {}x{} px", ctx->sensorResX, ctx->sensorResY);

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
        if (!GetSpeedTable(ctx, ctx->speedTable))
            return false;

        // Speed table has been created, print it out
        std::string table{"  Speed table:\n"};
        for (const auto &port: ctx->speedTable) {
            table.append(fmt::format("  - port '{}', value {}\n", port.name, port.value));
            for (const auto &speed: port.speeds) {
                table.append(fmt::format("    - speed index {}, running at {} MHz\n",
                                         speed.index, 1000 / (float) speed.pixTimeNs));
                for (const auto &gain: speed.gains) {
                    table.append(fmt::format("      - gain index {}, {}bit-depth {} bpp\n",
                                             gain.index,
                                             (gain.name.empty()) ? "" : "'" + gain.name + "', ",
                                             gain.bitDepth));

                }
            }
        }
        spdlog::info(table);

        // Initialize the camera to the first port, first speed and first gain

        if (PV_OK != pl_set_param(ctx->hcam, PARAM_READOUT_PORT,
                                  (void *) &ctx->speedTable[0].value)) {
            PrintError("Readout port could not be set");
            return false;
        }
        spdlog::info("  Setting readout port to '{}'", ctx->speedTable[0].name);

        if (PV_OK != pl_set_param(ctx->hcam, PARAM_SPDTAB_INDEX,
                                  (void *) &ctx->speedTable[0].speeds[0].index)) {
            PrintError("Readout port could not be set");
            return false;
        }
        spdlog::info("Setting readout speed index to {}", ctx->speedTable[0].speeds[0].index);

        if (PV_OK != pl_set_param(ctx->hcam, PARAM_GAIN_INDEX,
                                  (void *) &ctx->speedTable[0].speeds[0].gains[0].index)) {
            PrintError("Gain index could not be set");
            return false;
        }
        spdlog::info("  Setting gain index to {}", ctx->speedTable[0].speeds[0].gains[0].index);

        spdlog::info("");

        // Set the number of sensor clear cycles to 2 (default).
        // This is mostly relevant to CCD cameras only and it has
        // no effect with CLEAR_NEVER or CLEAR_AUTO clearing modes 
        // typically used with sCMOS cameras.
        if (!IsParamAvailable(ctx->hcam, PARAM_CLEAR_CYCLES, "PARAM_CLEAR_CYCLES"))
            return false;
        uns16 clearCycles = 2;
        if (PV_OK != pl_set_param(ctx->hcam, PARAM_CLEAR_CYCLES, (void *) &clearCycles)) {
            PrintError(
                    "pl_set_param(PARAM_CLEAR_CYCLES) error");
            return false;
        }

        // Find out if the sensor is a frame transfer or other (typically interline)
        // type. This process is relevant for CCD cameras only.
        ctx->isFrameTransfer = false;
        rs_bool isFrameTransfer;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_FRAME_CAPABLE, ATTR_AVAIL,
                                  (void *) &isFrameTransfer)) {
            isFrameTransfer = FALSE;
            PrintError(
                    "pl_get_param(PARAM_FRAME_CAPABLE) error");
            return false;
        }
        if (isFrameTransfer) {
            if (PV_OK != pl_get_param(ctx->hcam, PARAM_FRAME_CAPABLE, ATTR_CURRENT,
                                      (void *) &isFrameTransfer)) {
                isFrameTransfer = FALSE;
                PrintError(
                        "pl_get_param(PARAM_FRAME_CAPABLE) error");
                return false;
            }
            ctx->isFrameTransfer = isFrameTransfer == TRUE;
        }
        if (ctx->isFrameTransfer) {
            spdlog::info("  Camera with Frame Transfer capability sensor");
        } else {
            spdlog::info("  Camera without Frame Transfer capability sensor");
        }

        // If this is a Frame Transfer sensor set PARAM_PMODE to PMODE_FT.
        // The other common mode for these sensors is PMODE_ALT_FT.
        if (!IsParamAvailable(ctx->hcam, PARAM_PMODE, "PARAM_PMODE"))
            return false;
        if (ctx->isFrameTransfer) {
            int32 PMode = PMODE_FT;
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_PMODE, (void *) &PMode)) {
                PrintError("pl_set_param(PARAM_PMODE) error");
                return false;
            }
        }
            // If not a Frame Transfer sensor (i.e. Interline), set PARAM_PMODE to
            // PMODE_NORMAL, or PMODE_ALT_NORMAL.
        else {
            int32 PMode = PMODE_NORMAL;
            if (PV_OK != pl_set_param(ctx->hcam, PARAM_PMODE, (void *) &PMode)) {
                PrintError("pl_set_param(PARAM_PMODE) error");
                return false;
            }
        }

        // Check if the camera supports Smart Streaming feature.
        if (!IsParamAvailable(ctx->hcam, PARAM_SMART_STREAM_MODE, "PARAM_SMART_STREAM_MODE")) {
            spdlog::info("  Smart Streaming is not available");
            ctx->isSmartStreaming = false;
        } else {
            spdlog::info("  Smart Streaming is available");
            ctx->isSmartStreaming = true;

        }

        spdlog::info("");
        return true;
    }

    void PhotometricsBackend::CloseCamera(std::unique_ptr<CameraContext> &ctx) {
        if (!ctx->isCamOpen) {
            return;
        }

        if (PV_OK != pl_cam_close(ctx->hcam)) {
            PrintError("pl_cam_close() error");
        } else {
            spdlog::info("Camera {} '{}' closed\n", ctx->hcam, ctx->camName);
        }
        ctx->isCamOpen = false;
    }

    void PhotometricsBackend::CloseAllCamerasAndUninit() {
        printf("\n");

        for (auto &ctx: mCameraContexts) {
            CloseCamera(ctx);
        }

        UninitPVCAM();
    }

    void PhotometricsBackend::Init() {
        spdlog::info("Starting init");
        InitAndOpenOneCamera();
    }

    bool PhotometricsBackend::InitAndOpenOneCamera() {
        if (!ShowAppInfo(margc, margv)) {
            PrintError("Couldn't show app info");
            return PV_FAIL;
        }

        if (!InitPVCAM())
            return false;

        if (mCameraIndex >= mCameraContexts.size()) {
            PrintError("Camera index #{} is invalid", mCameraIndex);
            UninitPVCAM();
            return false;
        }

        if (!OpenCamera(mCameraContexts.at(mCameraIndex))) {
            CloseCamera(mCameraContexts.at(mCameraIndex));
            UninitPVCAM();
            return false;
        }

        return true;
    }

    bool PhotometricsBackend::ReadEnumeration(int16 hcam, NVPC *pNvpc, uns32 paramID, const char *paramName) {
        if (!pNvpc || !paramName) {
            return false;
        }

        if (!IsParamAvailable(hcam, paramID, paramName)) {
            return false;
        }

        uns32 count;
        if (PV_OK != pl_get_param(hcam, paramID, ATTR_COUNT, (void *) &count)) {
            PrintError("pl_get_param({}) error", paramName);
            return false;
        }

        NVPC nvpc;
        for (uns32 i = 0; i < count; ++i) {
            // Retrieve the enum string length
            uns32 strLength;
            if (PV_OK != pl_enum_str_length(hcam, paramID, i, &strLength)) {
                PrintError("pl_enum_str_length({}) error", paramName);
                return false;
            }

            // Allocate the destination string
            char *name = new(std::nothrow) char[strLength];
            if (!name) {
                PrintError("Unable to allocate memory for {} enum item name", paramName);
                return false;
            }

            // Get the string and value
            int32 value;
            if (PV_OK != pl_get_enum_param(hcam, paramID, i, &value, name, strLength)) {
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

    bool PhotometricsBackend::IsParamAvailable(int16 hcam, uns32 paramID, const char *paramName) {

        if (paramName == nullptr) {
            return PV_FAIL;
        }

        rs_bool isAvailable;
        if (PV_OK != pl_get_param(hcam, paramID, ATTR_AVAIL, (void *) &isAvailable)) {
            PrintError("Error reading ATTR_AVAIL of {}", paramName);
            return PV_FAIL;
        }
        if (isAvailable == FALSE) {
            spdlog::info("Parameter {} is not available", paramName);
            return PV_FAIL;
        }

        return PV_OK;
    }

    void PhotometricsBackend::UpdateCtxImageFormat(std::unique_ptr<CameraContext> &ctx) {
        ctx->imageFormat = PL_IMAGE_FORMAT_MONO8;

        rs_bool isAvailable;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_IMAGE_FORMAT, ATTR_AVAIL,
                                  (void *) &isAvailable))
            return;
        if (isAvailable == FALSE)
            return;

        int32 imageFormat;
        if (PV_OK != pl_get_param(ctx->hcam, PARAM_IMAGE_FORMAT, ATTR_CURRENT,
                                  (void *) &imageFormat))
            return;

        ctx->imageFormat = imageFormat;
    }

    bool PhotometricsBackend::SelectCameraExpMode(const std::unique_ptr<CameraContext> &ctx, int16 &expMode,
                                                  int16 legacyTrigMode, int16 extendedTrigMode) {
        NVPC triggerModes;
        if (!ReadEnumeration(ctx->hcam, &triggerModes, PARAM_EXPOSURE_MODE, "PARAM_EXPOSURE_MODE")) {
            return false;
        }
        // Try to find the legacy mode first
        for (const NVP &nvp: triggerModes) {
            if (nvp.value == legacyTrigMode) {
                // If this is a legacy (mostly CCD) camera, return the legacy mode
                expMode = legacyTrigMode;
                return true;
            }
        }

        // If not, select the extended mode and choose the first expose-out mode.
        for (const NVP &nvp: triggerModes) {
            if (nvp.value == extendedTrigMode) {
                // Modern cameras should all support the expose-out mode, but let's make sure.
                if (!IsParamAvailable(ctx->hcam, PARAM_EXPOSE_OUT_MODE, "PARAM_EXPOSE_OUT_MODE")) {
                    expMode = extendedTrigMode;
                    return true;
                }
                // Select the first available expose-out mode. For the SDK example purposes, the
                // expose-out mode is not crucial as it controls the expose-out hardware signal.
                NVPC expOutModes;
                if (!ReadEnumeration(ctx->hcam, &expOutModes, PARAM_EXPOSE_OUT_MODE, "PARAM_EXPOSE_OUT_MODE")) {
                    return false;
                }
                // Select the first one
                const auto expOutMode = static_cast<int16>(expOutModes[0].value);
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

    void PhotometricsBackend::LiveCapture(CAP_FORMAT format) {
        auto &ctx = mCameraContexts[0];
        uns32 exposureBytes;
        const uns32 exposureTime = 40; // milliseconds

        const uns16 circBufferFrames = 20;
        int16 bufferMode = CIRC_OVERWRITE;

        // Select the appropriate internal trigger mode for this camera.
        int16 expMode;
        if (!SelectCameraExpMode(ctx, expMode, TIMED_MODE, EXT_TRIG_INTERNAL)) {
            CloseAllCamerasAndUninit();
            return;
        }
        /**
        Prepare the continuous acquisition with circular buffer mode. The
        pl_exp_setup_cont() function returns the size of one frame (unlike
        the pl_exp_setup_seq() that returns a buffer size for the entire sequence).
        */
        if (PV_OK != pl_exp_setup_cont(ctx->hcam, 1, &ctx->region, expMode,
                                       exposureTime, &exposureBytes, bufferMode)) {
            PrintError("pl_exp_setup_cont() error\n");
            CloseAllCamerasAndUninit();
            return;
        }
        spdlog::info("Acquisition setup successful on camera {}\n", ctx->hcam);
        UpdateCtxImageFormat(ctx);

        const uns32 circBufferBytes = circBufferFrames * exposureBytes;
        /**
        Now allocate the buffer memory. The application is in control of the
        circular buffer and should allocate memory of appropriate size.
        */
        uns8 *circBufferInMemory = new(std::nothrow) uns8[circBufferBytes];
        if (!circBufferInMemory) {
            PrintError("Unable to allocate buffer for camera {}\n", ctx->hcam);
            CloseAllCamerasAndUninit();
            return;
        }
        /**
        Start the continuous acquisition. By passing the entire size of the buffer
        to pl_exp_start_cont() function, PVCAM can calculate the capacity of the circular buffer.
        */
        if (PV_OK != pl_exp_start_cont(ctx->hcam, circBufferInMemory, circBufferBytes)) {
            PrintError("pl_exp_start_cont() error\n");
            CloseAllCamerasAndUninit();
            delete[] circBufferInMemory;
            return;
        }
        spdlog::info("Acquisition started on camera {}\n", ctx->hcam);

        uns32 framesAcquired = 0;
        bool errorOccurred = false;
        while (framesAcquired < 50) {

            int16 status;
            uns32 byte_cnt;
            uns32 buffer_cnt;
            /**
            Keep checking the camera readout status. If readout succeeds, print the ADU values
            of the first five pixels. 'Sleep' is used to reduce CPU load while polling for status.
            WARNING:
            When acquisition is running, the status changes in the following order:
            EXPOSURE_IN_PROGRESS -> READOUT_IN_PROGRESS -> FRAME_AVAILABLE -> EXPOSURE_IN_PROGRESS
            and so on. This means that the FRAME_AVAILABLE status can be obtained only momentarily
            between the frames (between the last EOF and the next BOF). Due to this constraint, the polling
            approach introduces higher risk of lost frames than the callbacks approach because some frames
            may be lost unnoticed.
            */
            while (PV_OK == pl_exp_check_cont_status(ctx->hcam, &status, &byte_cnt, &buffer_cnt)
                   && status != FRAME_AVAILABLE && status != READOUT_NOT_ACTIVE
                   && !ctx->threadAbortFlag) {
                /**
                WARNING: Removing the sleep or setting it to a value too low may significantly increase
                the CPU load. Shorter sleeps do not guarantee the code does not miss a frame 'notification'
                if the frame rate is too high.
                */
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if (ctx->threadAbortFlag) {
                // This flag is set when user presses ctrl+c. In such case, break the loop
                // and abort the acquisition.
                spdlog::info("Processing aborted on camera {}\n", ctx->hcam);
                return;
            }
            if (status == READOUT_FAILED) {
                PrintError("Frame #{} readout failed on camera {}\n",
                           framesAcquired + 1, ctx->hcam);
                errorOccurred = true;
                return;
            }

            /**
            Obtain the address of the latest frame in the buffer with the pl_exp_get_latest_frame()
            function. Optionally, the pl_exp_get_latest_frame_ex() can be used to receive the address
            together with a #FRAME_INFO structure. This structure contains frame number and timestamps.
            */
            void *frameAddress;

            FRAME_INFO info{};
            if (PV_OK != pl_exp_get_latest_frame_ex(ctx->hcam, &frameAddress, &info)) {
                PrintError("pl_exp_get_latest_frame() error");
                errorOccurred = true;
                return;
            }

            spdlog::info("Frame #{} readout successfully completed on camera {}",
                         framesAcquired + 1, ctx->hcam);

            spdlog::info("Size is {} : {}", exposureBytes, ctx->sensorResX * ctx->sensorResY);
            //ShowImage(ctx, frameAddress, exposureBytes);

            framesAcquired++;
        }
    }

    void PhotometricsBackend::TerminateCapture() {
        spdlog::info("\n>>>\n"
                     ">>> CLI TERMINATION HANDLER");

        for (auto &ctx: mCameraContexts) {
            if (!ctx || !ctx->isCamOpen) {
                continue;
            }

            {
                std::scoped_lock lock(ctx->eofEvent.mutex);
                if (ctx->threadAbortFlag) {
                    continue;
                }
                ctx->threadAbortFlag = true;
                spdlog::info(">>> Requesting ABORT on camera {}\n", ctx->hcam);
            }
            ctx->eofEvent.cond.notify_all();
        }
        spdlog::info(">>>\n\n");
    }
}