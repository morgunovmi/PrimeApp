#ifndef PRIME_APP_BACKENDOPTION_H
#define PRIME_APP_BACKENDOPTION_H

#include "OpencvBackend.h"
#include "PhotometricsBackend.h"

namespace prm
{
    enum BackendOption
    {
        OPENCV = 0,
        PVCAM = 1,
    };
}

#endif//PRIME_APP_BACKENDOPTION_H
