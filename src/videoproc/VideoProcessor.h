#ifndef PRIME_APP_VIDEOPROCESSOR_H
#define PRIME_APP_VIDEOPROCESSOR_H

#include <pybind11/embed.h>

namespace py = pybind11;
using namespace py::literals;

class VideoProcessor {
public:
    VideoProcessor() { Init(); Test(); }

private:
    static void Init();
    static void Test();
    py::scoped_interpreter mGuard;
};

#endif //PRIME_APP_VIDEOPROCESSOR_H
