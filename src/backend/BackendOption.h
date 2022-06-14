#pragma once

#include "OpencvBackend.h"
#include "PhotometricsBackend.h"

namespace prm
{
    /// Enumeration for possible camera backend options
    enum BackendOption
    {
        OPENCV = 0, ///< Backend for webcams
        PVCAM = 1, ///< Backend for connected Teledyne Photometrics cameras
    };
}
