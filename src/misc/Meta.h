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
             {"binning", meta.binning},
             {"lens", meta.lens}};
}