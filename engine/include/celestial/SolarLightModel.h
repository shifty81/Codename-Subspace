#pragma once

#include <cstdint>
#include <string>

namespace subspace {

struct SolarLightSource {
    std::string bodyId = "home_star";
    float x = 0.0f;
    float y = 0.0f;
    float power = 1.0f;
    float safeMinimum = 0.05f;
    float flarePressure = 0.0f;
    std::uint32_t color = 0xFFE7A0u;
};

struct SolarLightSample {
    float intensity = 0.0f;
    float heatPressure = 0.0f;
    float solarChargeBonus = 0.0f;
    float dirX = 1.0f;
    float dirY = 0.0f;
};

SolarLightSample SampleSolarLight(const SolarLightSource& source, float sampleX, float sampleY);
std::string SolarLightSummary(const SolarLightSample& sample);

} // namespace subspace
