#ifndef SOLAR_GAME_H
#define SOLAR_GAME_H

#include <SFML/Graphics.hpp>

#include <mutex>
#include <vector>
#include <string>

#include "master.h"
#include "pvcam.h"
#include "misc/Log.h"

namespace slr {
    typedef struct NVP {
        int32 value;
        std::string name;
    }
    NVP;
    /* Name-Value Pair Container type - an enumeration type */
    typedef std::vector<NVP> NVPC;

    typedef struct READOUT_OPTION {
        NVP port;
        int16 speedIndex;
        float readoutFrequency;
        int16 bitDepth;
        std::vector<int16> gains;
    }
    READOUT_OPTION;

    class Backend {
    public:
        Backend(sf::RenderWindow& window, sf::Clock& deltaClock, sf::Time& dt, Log& appLog) :
            mWindow(window), mDeltaClock(deltaClock), mDt(dt), mAppLog(appLog), mPvcamMutex() { }

        rs_bool InitAndOpenFirstCamera();

        rs_bool EnumerateCameras();

        void Update();

        void CloseCameraAndUninit();

        ~Backend() { CloseCameraAndUninit(); }

    private:
        void PollInput();

        void PrintError(const char* fmt, ...);

        rs_bool ShowAppInfo(int argc, char* argv[]);

        rs_bool ReadEnumeration(NVPC* nvpc, uns32 paramID, const char* paramName);

        rs_bool IsParamAvailable(uns32 paramID, const char* paramName);

        rs_bool OpenCamera();

        rs_bool InitPvcam();



    private:
        sf::RenderWindow&                       mWindow;

        sf::Clock&                              mDeltaClock;
        sf::Time&                               mDt;

        Log&                                    mAppLog;

        std::mutex                              mPvcamMutex;

        std::vector<READOUT_OPTION> mSpeedTable;
        int16 mHCam = -1;
        int16 mNrOfCameras = 0;
        uns16 mSensorResX = 0;
        uns16 mSensorResY = 0;
        int16 mSensorBitDepth = 0;
        uns32 mSensorColorMode = COLOR_NONE;
        char mCamera0_Name[CAM_NAME_LEN] = "";
        rs_bool mIsFrameTransfer = FALSE;
        rgn_type mRegion = { 0, 0, 0, 0, 0, 0 };
        FRAME_INFO* mpFrameInfo = NULL;

        rs_bool mIsPvcamInitialized = FALSE;
        rs_bool mIsCameraOpen = FALSE;
    };
}

#endif //SOLAR_GAME_H
