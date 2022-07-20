#pragma once

#include <nlohmann/json.hpp>
using nlohmann::json;

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
    double frametimeAvg;
    double frametimeMin;
    double frametimeMax;
    double frametimeStd;
    Binning binning;
    Lens lens;
};

NLOHMANN_JSON_SERIALIZE_ENUM(Binning, {{ONE, "1x1"}, {TWO, "2x2"}})
NLOHMANN_JSON_SERIALIZE_ENUM(Lens, {{X10, "x10"}, {X20, "x20"}})

inline void to_json(json& j, const TifStackMeta& meta)
{
    j = json{{"nFrames", meta.numFrames},
             {"exposure", meta.exposure},
             {"fps", meta.fps},
             {"frametimeAvg", meta.frametimeAvg},
             {"frametimeMin", meta.frametimeMin},
             {"frametimeMax", meta.frametimeMax},
             {"frametimeStd", meta.frametimeStd},
             {"binning", meta.binning},
             {"lens", meta.lens}};
}

inline void from_json(const json& j, TifStackMeta& m)
{
    j[0].at("nFrames").get_to(m.numFrames);
    j[0].at("exposure").get_to(m.exposure);
    j[0].at("fps").get_to(m.fps);
    j[0].at("frametimeAvg").get_to(m.frametimeAvg);
    j[0].at("frametimeMin").get_to(m.frametimeMin);
    j[0].at("frametimeMax").get_to(m.frametimeMax);
    j[0].at("frametimeStd").get_to(m.frametimeStd);
    j[0].at("binning").get_to(m.binning);
    j[0].at("lens").get_to(m.lens);
}

struct VideoProcessorMeta
{
    int minMass;
    double ecc;
    int size;
    int diameter;
    int searchRange;
    int memory;
    int minTrajLen;
    int driftSmoothing;
    int minDiagSize;
    int maxDiagSize;
    Lens lens;
    Binning binning;
    double scale;
    double fps;

    bool operator==(const VideoProcessorMeta& o) const
    {
        return minMass == o.minMass && ecc == o.ecc && size == o.size &&
               diameter == o.diameter && searchRange == o.searchRange &&
               memory == o.memory && minTrajLen == o.minTrajLen &&
               driftSmoothing == o.driftSmoothing &&
               minDiagSize == o.minDiagSize && maxDiagSize == o.maxDiagSize &&
               lens == o.lens && binning == o.binning && scale == o.scale &&
               fps == o.fps;
    }
};

inline void to_json(json& j, const VideoProcessorMeta& meta)
{
    j = json{{"Min mass", meta.minMass},
             {"Eccentricity", meta.ecc},
             {"Size", meta.size},
             {"Diameter", meta.diameter},
             {"Search range", meta.searchRange},
             {"Memory", meta.memory},
             {"Min traj. len", meta.minTrajLen},
             {"Drift smoothing", meta.driftSmoothing},
             {"Min diag. size", meta.minDiagSize},
             {"Max diag size", meta.maxDiagSize},
             {"Lens", meta.lens},
             {"Binning", meta.binning},
             {"Scale", meta.scale},
             {"FPS", meta.fps}};
}