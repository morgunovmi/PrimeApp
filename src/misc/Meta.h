#pragma once

enum Binning
{
    ONE,
    TWO
};

enum Lens
{
    X10,
    X20
};

struct TifStackMeta
{
    std::uint32_t numFrames;
    std::uint16_t exposure;
    double fps;
    Binning binning;
    Lens lens;
};
