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
    j.at("nFrames").get_to(m.numFrames);
    j.at("exposure").get_to(m.exposure);
    j.at("fps").get_to(m.fps);
    j.at("frametimeAvg").get_to(m.frametimeAvg);
    j.at("frametimeMin").get_to(m.frametimeMin);
    j.at("frametimeMax").get_to(m.frametimeMax);
    j.at("frametimeStd").get_to(m.frametimeStd);
    j.at("binning").get_to(m.binning);
    j.at("lens").get_to(m.lens);
}