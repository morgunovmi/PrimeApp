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

    struct SpdtabGain {
        // In PVCAM, gain indexes are 1-based.
        int32 index{ 1 };
        // Not all cameras support named gains. If not supported, this
        // string stays empty.
        std::string name{};
        // The bit-depth may be different for each gain, therefore it is stored
        // within this structure. For example, the Prime BSI camera has gains
        // with different bit-depths under the same speed.
        int16 bitDepth{ 0 };
    };

    struct SpdtabSpeed {
        // In PVCAM, speed indexes are 0-based.
        int32 index{ 0 };
        // Pixel time can be used to calculate the overall readout rate. This is less
        // relevant with sCMOS sensors, but the pix time is still reported to provide
        // an approximate readout rate of a particular speed. 
        uns16 pixTimeNs{ 1 };
        // List of gains under this particular speed.
        std::vector<SpdtabGain> gains;
    };

    struct SpdtabPort {
        // Please note that the PARAM_READOUT_PORT is an ENUM_TYPE parameter.
        // For this reason, the current port is not reported as an index, but
        // as a generic number. Applications that are written in a generic way
        // to support all Teledyne Photometrics cameras should not rely on a fact 
        // that port numbers are usually zero-based.
        int32 value{ 0 };
        // Name of this port, as retrieved by enumerating the PARAM_READOUT_PORT.
        std::string name{};
        // List of speeds under this particular port.
        std::vector<SpdtabSpeed> speeds;
    };

    struct Event {
        // Mutex that guards all other members
        std::mutex mutex{};
        // Condition that any thread could wait on
        std::condition_variable cond{};
        // A flag that helps with spurious wakeups
        bool flag{ false };
    };


    struct CameraContext {
        // Camera name, the only member valid before opening camera
        char camName[CAM_NAME_LEN]{ '\0' };

        // Set to true when PVCAM opens camera
        bool isCamOpen{ false };

        // All members below are initialized once the camera is successfully open

        // Camera handle
        int16 hcam{ -1 };

        Event eofEvent{};

        // Camera sensor serial size (sensor width)
        uns16 sensorResX{ 0 };
        // Camera sensor parallel size (sensor height)
        uns16 sensorResY{ 0 };
        // Sensor region and binning factors to be used for the acquisition,
        // initialized to full sensor size with 1x1 binning upon opening the camera.
        rgn_type region{ 0, 0, 0, 0, 0, 0 };

        // Vector of camera readout options, commonly referred to as 'speed table'
        std::vector<SpdtabPort> speedTable{};

        // Image format reported after acq. setup, value from PL_IMAGE_FORMATS
        int32 imageFormat{ PL_IMAGE_FORMAT_MONO16 };
        // Sensor type (if not Frame Transfer CCD then camera is Interline CCD or sCMOS).
        // Not relevant for sCMOS sensors.
        bool isFrameTransfer{ false };
        // Flag marking the camera as Smart Streaming capable
        bool isSmartStreaming{ false };

        // Event used for communication between acq. loop and EOF callback routine
        //Event eofEvent{};
        // Storage for a code sample specific context data if needed for callback acquisition
        void* eofContext{ nullptr };
        // Frame info structure used to store data, for example, in EOF callback handlers
        FRAME_INFO eofFrameInfo{};
        // The address of latest frame stored, for example, in EOF callback handlers
        void* eofFrame{ nullptr };

        // Used as an acquisition thread or for other independent tasks
        std::thread* thread{ nullptr };
        // Flag to be set to abort thread (used, for example, in multi-camera code samples)
        bool threadAbortFlag{ false };
    };

    class Backend {
    public:
        Backend(int argc, char **argv, sf::RenderWindow& window, sf::Clock& deltaClock, sf::Time& dt, sf::Texture& currentTexture, Log& appLog) :
            margc(argc), margv(argv), mWindow(window), mDeltaClock(deltaClock), mDt(dt), mAppLog(appLog), mPvcamMutex(), mCurrentTexture(currentTexture) {}

        bool InitAndOpenOneCamera();

        void Update();

        bool LiveCapture();

        void TerminateCapture();

        ~Backend() { CloseAllCamerasAndUninit();  }

    private:
        void PollInput();

        void PrintError(const char* fmt, ...);

        bool ShowAppInfo(int argc, char* argv[]);

        bool ReadEnumeration(int16 hcam, NVPC* pNvpc, uns32 paramID, const char* paramName);

        bool IsParamAvailable(int16 hcam, uns32 paramID, const char* paramName);

        bool GetSpeedTable(const std::unique_ptr<CameraContext>& ctx, std::vector<SpdtabPort>& speedTable);

        bool OpenCamera(std::unique_ptr<CameraContext>& ctx);

        void CloseCamera(std::unique_ptr<CameraContext>& ctx);

        bool InitPVCAM();

        void UninitPVCAM();

        void CloseAllCamerasAndUninit();

        void UpdateCtxImageFormat(std::unique_ptr<CameraContext>& ctx);

        bool SelectCameraExpMode(const std::unique_ptr<CameraContext>& ctx, int16& expMode,
            int16 legacyTrigMode, int16 extendedTrigMode);



    private:
        int margc;
        char** margv;
        sf::RenderWindow&                       mWindow;

        sf::Clock&                              mDeltaClock;
        sf::Time&                               mDt;

        Log&                                    mAppLog;

        std::mutex                              mPvcamMutex;

        sf::Texture&                            mCurrentTexture;

        const uns16                             mCameraIndex = 0;
        std::vector<std::unique_ptr<CameraContext>> mCameraContexts;

        rs_bool mIsPvcamInitialized = FALSE;
    };
}

#endif //SOLAR_GAME_H