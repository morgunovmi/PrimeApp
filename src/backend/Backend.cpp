#include "backend/Backend.h"

#include "imgui-SFML.h"
#include <iostream>

namespace slr {
    void Backend::PrintError(const char* fmt, ...) {
        std::scoped_lock lock(mPvcamMutex);

        auto code = pl_error_code();
        char pvcamErrMsg[ERROR_MSG_LEN];
        pl_error_message(code, pvcamErrMsg);
        mAppLog.AddLog("-----Error code %zu-----\n", code);
        mAppLog.AddLog("%s\n", pvcamErrMsg);

        va_list args;
        va_start(args, fmt);
        mAppLog.AddLog(fmt, args);
        va_end(args);

        mAppLog.AddLog("------------------------\n");
    }

    rs_bool Backend::ShowAppInfo(int argc, char* argv[]) {
        std::scoped_lock lock(mPvcamMutex);

        const char* appName = "<unable to get name>";
        if (argc > 0 && argv != NULL && argv[0] != NULL) {
            appName = argv[0];
        }

        // Read PVCAM library version
        uns16 pvcamVersion;
        if (PV_OK != pl_pvcam_get_ver(&pvcamVersion)) {
            PrintError("pl_pvcam_get_ver() error\n");
            return PV_FAIL;
        }

        mAppLog.AddLog("************************************************************\n");
        mAppLog.AddLog("Application  : %s\n", appName);
        mAppLog.AddLog("PVCAM version: %d.%d.%d\n",
            (pvcamVersion >> 8) & 0xFF,
            (pvcamVersion >> 4) & 0x0F,
            (pvcamVersion >> 0) & 0x0F);
        mAppLog.AddLog("************************************************************\n\n");

        return PV_OK;
    }

    rs_bool Backend::InitPvcam() {
        std::scoped_lock lock(mPvcamMutex);

        mAppLog.AddLog("Initializing PVCAM and checking for cameras...\n");

        if (pl_pvcam_init() != PV_OK) {
            PrintError("Failed to init cameras\n");
            return PV_FAIL;
        }
        mIsPvcamInitialized = TRUE;
        mAppLog.AddLog("PVCAM initialized\n");

        if (PV_OK != pl_cam_get_total(&mNrOfCameras)) {
            PrintError("pl_cam_get_total() error\n");
            return PV_FAIL;
        }

        if (mNrOfCameras == 0) {
            mAppLog.AddLog("No cameras found in the system\n");
            return PV_FAIL;
        }
        mAppLog.AddLog("Number of cameras found: %d\n", mNrOfCameras);

        if (PV_OK != pl_cam_get_name(0, mCamera0_Name)) {
            PrintError("pl_cam_get_name() error\n");
            return PV_FAIL;
        }
        mAppLog.AddLog("Camera 0 name: %s\n", mCamera0_Name);

        return PV_OK;
    }

    rs_bool Backend::EnumerateCameras() {
        std::scoped_lock lock(mPvcamMutex);

        int16 nrOfCameras = 0;
        if (PV_OK != pl_cam_get_total(&nrOfCameras)) {
            PrintError("Error getting number of cameras\n");
            return PV_FAIL;
        }
        mAppLog.AddLog("Found %d cameras\n", nrOfCameras);

        char camName[CAM_NAME_LEN];
        for (int16 n = 0; n < nrOfCameras; ++n) {
            if (PV_OK != pl_cam_get_name(n, camName)) {
                PrintError("Error getting name of %dth camera\n", n);
                return PV_FAIL;
            }

            mAppLog.AddLog("Camera %d: %s", n, camName);
        }
        return PV_OK;
    }

    rs_bool Backend::OpenCamera() {
        mAppLog.AddLog("Opening the camera and reading parameters...\n");

        // Open camera with the specified camera name obtained in InitPVCAM() function
        if (PV_OK != pl_cam_open(mCamera0_Name, &mHCam, OPEN_EXCLUSIVE)) {
            PrintError( "pl_cam_open() error");
            return PV_FAIL;
        }
        mIsCameraOpen = TRUE;
        mAppLog.AddLog("Camera %s opened\n", mCamera0_Name);

        // Read the version of Device Driver
        if (!IsParamAvailable(PARAM_DD_VERSION, "PARAM_DD_VERSION")) {
            return PV_FAIL;
        }
        uns16 ddVersion;
        if (PV_OK != pl_get_param(mHCam, PARAM_DD_VERSION, ATTR_CURRENT,
            (void*)&ddVersion)) {
            PrintError(
                "pl_get_param(PARAM_DD_VERSION) error");
            return PV_FAIL;
        }
        mAppLog.AddLog("Device driver version: %d.%d.%d\n",
            (ddVersion >> 8) & 0xFF,
            (ddVersion >> 4) & 0x0F,
            (ddVersion >> 0) & 0x0F);

        // Get camera chip name string. Typically holds both chip and camera model
        // name, therefore is the best camera identifier for most models
        if (!IsParamAvailable(PARAM_CHIP_NAME, "PARAM_CHIP_NAME")) {
            return PV_FAIL;
        }
        char chipName[CCD_NAME_LEN];
        if (PV_OK != pl_get_param(mHCam, PARAM_CHIP_NAME, ATTR_CURRENT,
            (void*)chipName)) {
            PrintError(
                "pl_get_param(PARAM_CHIP_NAME) error");
            return PV_FAIL;
        }
        mAppLog.AddLog("Sensor chip name: %s\n", chipName);

        // Get camera firmware version
        if (!IsParamAvailable(PARAM_CAM_FW_VERSION, "PARAM_CAM_FW_VERSION")) {
            return PV_FAIL;
        }
        uns16 fwVersion;
        if (PV_OK != pl_get_param(mHCam, PARAM_CAM_FW_VERSION, ATTR_CURRENT,
            (void*)&fwVersion)) {
            PrintError(
                "pl_get_param(PARAM_CAM_FW_VERSION) error");
            return PV_FAIL;
        }
        mAppLog.AddLog("Camera firmware version: %d.%d\n",
            (fwVersion >> 8) & 0xFF,
            (fwVersion >> 0) & 0xFF);

        // Find out if the sensor is a frame transfer or other (typically interline)
        // type. This is a two-step process.
        // Please, follow the procedure below in your applications.
        if (PV_OK != pl_get_param(mHCam, PARAM_FRAME_CAPABLE, ATTR_AVAIL,
            (void*)&mIsFrameTransfer)) {
            mIsFrameTransfer = 0;
            PrintError(
                "pl_get_param(PARAM_FRAME_CAPABLE) error");
            return PV_FAIL;
        }

        if (mIsFrameTransfer == TRUE) {
            if (PV_OK != pl_get_param(mHCam, PARAM_FRAME_CAPABLE, ATTR_CURRENT,
                (void*)&mIsFrameTransfer)) {
                mIsFrameTransfer = 0;
                PrintError(
                    "pl_get_param(PARAM_FRAME_CAPABLE) error");
                return PV_FAIL;
            }
            if (mIsFrameTransfer == TRUE) {
                mAppLog.AddLog("Camera with Frame Transfer capability sensor\n");
            }
        }
        if (mIsFrameTransfer == FALSE) {
            mIsFrameTransfer = 0;
            mAppLog.AddLog("Camera without Frame Transfer capability sensor\n");
        }

        // If this is a Frame Transfer sensor set PARAM_PMODE to PMODE_FT.
        // The other common mode for these sensors is PMODE_ALT_FT.
        if (!IsParamAvailable(PARAM_PMODE, "PARAM_PMODE")) {
            return PV_FAIL;
        }
        if (mIsFrameTransfer == TRUE) {
            int32 PMode = PMODE_FT;
            if (PV_OK != pl_set_param(mHCam, PARAM_PMODE, (void*)&PMode)) {
                PrintError(
                    "pl_set_param(PARAM_PMODE) error");
                return PV_FAIL;
            }
        }
        // If not a Frame Transfer sensor (i.e. Interline), set PARAM_PMODE to
        // PMODE_NORMAL, or PMODE_ALT_NORMAL.
        else {
            int32 PMode = PMODE_NORMAL;
            if (PV_OK != pl_set_param(mHCam, PARAM_PMODE, (void*)&PMode)) {
                PrintError(
                    "pl_set_param(PARAM_PMODE) error");
                return PV_FAIL;
            }
        }

        // If this is a color sensor, get and show the bayer pattern
        if (PV_OK != pl_get_param(mHCam, PARAM_COLOR_MODE, ATTR_CURRENT,
            (void*)&mSensorColorMode)) {
            PrintError(
                "pl_get_param(PARAM_COLOR_MODE) error");
            return PV_FAIL;
        }

        mAppLog.AddLog("Camera sensor is ");
        switch (mSensorColorMode) {
        case COLOR_NONE:
            mAppLog.AddLog("Mono\n");
            break;
        case COLOR_RGGB:
            mAppLog.AddLog("Color with bayer RGGB\n");
            break;
        case COLOR_GRBG:
            mAppLog.AddLog("Color with bayer GRBG\n");
            break;
        case COLOR_GBRG:
            mAppLog.AddLog("Color with bayer GBRG\n");
            break;
        case COLOR_BGGR:
            mAppLog.AddLog("Color with bayer BGGR\n");
            break;

            // Consider any others as Mono = no color
        default:
            mAppLog.AddLog("unrecognized type\n");
            break;
        }
        mAppLog.AddLog("\n");

        // This code iterates through all available camera ports and their readout
        // speeds and creates a Speed Table which holds indexes of ports and speeds,
        // readout frequencies and bit depths.

        NVPC ports;
        if (!ReadEnumeration(&ports, PARAM_READOUT_PORT, "PARAM_READOUT_PORT")) {
            return PV_FAIL;
        }

        if (!IsParamAvailable(PARAM_SPDTAB_INDEX, "PARAM_SPDTAB_INDEX")) {
            return PV_FAIL;
        }
        if (!IsParamAvailable(PARAM_PIX_TIME, "PARAM_PIX_TIME")) {
            return PV_FAIL;
        }
        if (!IsParamAvailable(PARAM_BIT_DEPTH, "PARAM_BIT_DEPTH")) {
            return PV_FAIL;
        }

        // Iterate through available ports and their speeds
        for (size_t pi = 0; pi < ports.size(); pi++) {
            // Set readout port
            if (PV_OK != pl_set_param(mHCam, PARAM_READOUT_PORT,
                (void*)&ports[pi].value)) {
                PrintError(
                    "pl_set_param(PARAM_READOUT_PORT) error");
                return PV_FAIL;
            }

            // Get number of available speeds for this port
            uns32 speedCount;
            if (PV_OK != pl_get_param(mHCam, PARAM_SPDTAB_INDEX, ATTR_COUNT,
                (void*)&speedCount)) {
                PrintError(
                    "pl_get_param(PARAM_SPDTAB_INDEX) error");
                return PV_FAIL;
            }

            // Iterate through all the speeds
            for (int16 si = 0; si < (int16)speedCount; si++) {
                // Set camera to new speed index
                if (PV_OK != pl_set_param(mHCam, PARAM_SPDTAB_INDEX, (void*)&si)) {
                    PrintError(
                        "pl_set_param(mHCam, PARAM_SPDTAB_INDEX) error");
                    return PV_FAIL;
                }

                // Get pixel time (readout time of one pixel in nanoseconds) for the
                // current port/speed pair. This can be used to calculate readout
                // frequency of the port/speed pair.
                uns16 pixTime;
                if (PV_OK != pl_get_param(mHCam, PARAM_PIX_TIME, ATTR_CURRENT,
                    (void*)&pixTime)) {
                    PrintError(
                        "pl_get_param(mHCam, PARAM_PIX_TIME) error");
                    return PV_FAIL;
                }

                // Get bit depth of the current readout port/speed pair
                int16 bitDepth;
                if (PV_OK != pl_get_param(mHCam, PARAM_BIT_DEPTH, ATTR_CURRENT,
                    (void*)&bitDepth)) {
                    PrintError(
                        "pl_get_param(PARAM_BIT_DEPTH) error");
                    return PV_FAIL;
                }

                int16 gainMin;
                if (PV_OK != pl_get_param(mHCam, PARAM_GAIN_INDEX, ATTR_MIN,
                    (void*)&gainMin)) {
                    PrintError(
                        "pl_get_param(PARAM_GAIN_INDEX) error");
                    return PV_FAIL;
                }

                int16 gainMax;
                if (PV_OK != pl_get_param(mHCam, PARAM_GAIN_INDEX, ATTR_MAX,
                    (void*)&gainMax)) {
                    PrintError(
                        "pl_get_param(PARAM_GAIN_INDEX) error");
                    return PV_FAIL;
                }

                int16 gainIncrement;
                if (PV_OK != pl_get_param(mHCam, PARAM_GAIN_INDEX, ATTR_INCREMENT,
                    (void*)&gainIncrement)) {
                    PrintError(
                        "pl_get_param(PARAM_GAIN_INDEX) error");
                    return PV_FAIL;
                }

                // Save the port/speed information to our Speed Table
                READOUT_OPTION ro;
                ro.port = ports[pi];
                ro.speedIndex = si;
                ro.readoutFrequency = 1000 / (float)pixTime;
                ro.bitDepth = bitDepth;
                ro.gains.clear();

                int16 gainValue = gainMin;

                while (gainValue <= gainMax) {
                    ro.gains.push_back(gainValue);
                    gainValue += gainIncrement;
                }

                mSpeedTable.push_back(ro);

                mAppLog.AddLog("mSpeedTable[%lu].Port = %lu (%d - %s)\n",
                    (unsigned long)(mSpeedTable.size() - 1),
                    (unsigned long)pi,
                    mSpeedTable[mSpeedTable.size() - 1].port.value,
                    mSpeedTable[mSpeedTable.size() - 1].port.name.c_str());
                mAppLog.AddLog("mSpeedTable[%lu].SpeedIndex = %d\n",
                    (unsigned long)(mSpeedTable.size() - 1),
                    mSpeedTable[mSpeedTable.size() - 1].speedIndex);
                mAppLog.AddLog("mSpeedTable[%lu].PortReadoutFrequency = %.3f MHz\n",
                    (unsigned long)(mSpeedTable.size() - 1),
                    mSpeedTable[mSpeedTable.size() - 1].readoutFrequency);
                mAppLog.AddLog("mSpeedTable[%lu].bitDepth = %d bit\n",
                    (unsigned long)(mSpeedTable.size() - 1),
                    mSpeedTable[mSpeedTable.size() - 1].bitDepth);
                for (int16 gi = 0; gi < (int16)ro.gains.size(); gi++) {
                    mAppLog.AddLog("mSpeedTable[%lu].gains[%d] = %d \n",
                        (unsigned long)(mSpeedTable.size() - 1),
                        (int)gi,
                        ro.gains[gi]);
                }
                mAppLog.AddLog("\n");
            }
        }

        // Speed Table has been created

        // Set camera to first port
        if (PV_OK != pl_set_param(mHCam, PARAM_READOUT_PORT,
            (void*)&mSpeedTable[0].port.value)) {
            PrintError( "Readout port could not be set");
            return PV_FAIL;
        }
        mAppLog.AddLog("Setting readout port to %s\n", mSpeedTable[0].port.name.c_str());

        // Set camera to speed 0
        if (PV_OK != pl_set_param(mHCam, PARAM_SPDTAB_INDEX,
            (void*)&mSpeedTable[0].speedIndex)) {
            PrintError( "Readout port could not be set");
            return PV_FAIL;
        }
        mAppLog.AddLog("Setting readout speed index to %d\n", mSpeedTable[0].speedIndex);

        // Set gain index to one (the first one)
        if (PV_OK != pl_set_param(mHCam, PARAM_GAIN_INDEX,
            (void*)&mSpeedTable[0].gains[0])) {
            PrintError( "Gain index could not be set");
            return PV_FAIL;
        }
        mAppLog.AddLog("Setting gain index to %d\n", mSpeedTable[0].gains[0]);

        // Get the sensor bit depth
        mSensorBitDepth = mSpeedTable[0].bitDepth;
        mAppLog.AddLog("Sensor bit depth is %d bits\n", mSensorBitDepth);
        mAppLog.AddLog("\n");

        // Get number of sensor columns
        if (!IsParamAvailable(PARAM_SER_SIZE, "PARAM_SER_SIZE")) {
            return PV_FAIL;
        }
        if (PV_OK != pl_get_param(mHCam, PARAM_SER_SIZE, ATTR_CURRENT,
            (void*)&mSensorResX)) {
            PrintError( "Couldn't read CCD X-resolution");
            return PV_FAIL;
        }
        // Get number of sensor lines
        if (!IsParamAvailable(PARAM_PAR_SIZE, "PARAM_PAR_SIZE")) {
            return PV_FAIL;
        }
        if (PV_OK != pl_get_param(mHCam, PARAM_PAR_SIZE, ATTR_CURRENT,
            (void*)&mSensorResY)) {
            PrintError( "Couldn't read CCD Y-resolution");
            return PV_FAIL;
        }
        mAppLog.AddLog("Sensor size: %dx%d\n", mSensorResX, mSensorResY);

        // Set number of sensor clear cycles to 2 (default)
        if (!IsParamAvailable(PARAM_CLEAR_CYCLES, "PARAM_CLEAR_CYCLES")) {
            return PV_FAIL;
        }
        uns16 ClearCycles = 2;
        if (PV_OK != pl_set_param(mHCam, PARAM_CLEAR_CYCLES, (void*)&ClearCycles)) {
            PrintError(
                "pl_set_param(PARAM_CLEAR_CYCLES) error");
            return PV_FAIL;
        }

        mAppLog.AddLog("\n");

        return PV_OK;
    }

	void Backend::CloseCameraAndUninit() {
		std::scoped_lock lock(mPvcamMutex);

		printf("\nClosing the camera and uninitializing PVCAM...\n");

		// Do not close camera if none has been detected and open
		if (mIsCameraOpen == TRUE) {
			if (PV_OK != pl_cam_close(mHCam)) {
				printf("pl_cam_close() error");
			} else {
				printf("Camera closed\n");
			}
		}

		// Uninitialize PVCAM library
		if (mIsPvcamInitialized == TRUE) {
			if (PV_OK != pl_pvcam_uninit()) {
				printf("pl_pvcam_uninit() error");
			} else {
				printf("PVCAM uninitialized\n");
			}
		}

		// Release frame info
		if (mpFrameInfo != NULL) {
			if (PV_OK != pl_release_frame_info_struct(mpFrameInfo)) {
				printf(
					"pl_release_frame_info_struct() error");
			}
		}
	}

    rs_bool Backend::InitAndOpenFirstCamera() {
        if (!InitPvcam()) {
            CloseCameraAndUninit();
            return PV_FAIL;
        }

        if (!OpenCamera()) {
            CloseCameraAndUninit();
            return PV_FAIL;
        }

        // Create this structure that will be used to received extended information
        // about the frame.
        // Support on interfaces may vary, full support on Firewire at the moment,
        // partial support on PCIe LVDS and USB interfaces, no support on legacy
        // LVDS.
        // FRAME_INFO can be allocated on stack bu we demonstrate here how to do the
        // same on heap.

        if (PV_OK != pl_create_frame_info_struct(&mpFrameInfo)) {
            printf(
                "pl_create_frame_info_struct() error");
            CloseCameraAndUninit();
            return PV_FAIL;
        }

        return PV_OK;
    }

    rs_bool Backend::ReadEnumeration(NVPC* nvpc, uns32 paramID, const char* paramName) {
        std::scoped_lock lock(mPvcamMutex);

        if (nvpc == NULL && paramName == NULL) {
            return PV_FAIL;
        }

        if (!IsParamAvailable(paramID, paramName)) {
            return PV_FAIL;
        }

        uns32 count;
        if (PV_OK != pl_get_param(mHCam, paramID, ATTR_COUNT, (void*)&count)) {
            const std::string msg =
                "pl_get_param(" + std::string(paramName) + ") error";
            PrintError(msg.c_str());
            return PV_FAIL;
        }

        // Actually get the enumeration names
        for (uns32 i = 0; i < count; ++i) {
            // Ask how long the string is
            uns32 strLength;
            if (PV_OK != pl_enum_str_length(mHCam, paramID, i, &strLength)) {
                const std::string msg =
                    "pl_enum_str_length(" + std::string(paramName) + ") error";
                PrintError(msg.c_str());
                return PV_FAIL;
            }

            // Allocate the destination string
            // Let app crash on exception if not enough memory
            char* name = new char[strLength];

            // Actually get the string and value
            int32 value;
            if (PV_OK != pl_get_enum_param(mHCam, paramID, i, &value, name,
                strLength)) {
                const std::string msg =
                    "pl_get_enum_param(" + std::string(paramName) + ") error";
                PrintError(msg.c_str());
                delete[] name;
                return PV_FAIL;
            }

            NVP nvp;
            nvp.value = value;
            nvp.name = name;
            nvpc->push_back(nvp);

            delete[] name;
        }

        if (nvpc->empty()) {
            return PV_FAIL;
        }

        return PV_OK;
    }

    rs_bool Backend::IsParamAvailable(uns32 paramID, const char* paramName) {
        std::scoped_lock lock(mPvcamMutex);

        if (paramName == NULL) {
            return PV_FAIL;
        }

        rs_bool isAvailable;
        if (PV_OK != pl_get_param(mHCam, paramID, ATTR_AVAIL, (void*)&isAvailable)) {
            PrintError("Error reading ATTR_AVAIL of %s\n", paramName);
            return PV_FAIL;
        }
        if (isAvailable == FALSE) {
            mAppLog.AddLog("Parameter %s is not available\n", paramName);
            return PV_FAIL;
        }

        return PV_OK;
    }

    void Backend::Update() {
        mDt = mDeltaClock.restart();
        PollInput();
    }

    void Backend::PollInput() {
        sf::Event event{};

        while (mWindow.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type) {
                case sf::Event::Closed:
                    mWindow.close();
                    break;

                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            mWindow.close();
                            break;
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}