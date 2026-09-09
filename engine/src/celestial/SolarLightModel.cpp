#include "celestial/SolarLightModel.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {

SolarLightSample SampleSolarLight(const SolarLightSource& source, float sampleX, float sampleY) {
    const float dx = source.x - sampleX;
    const float dy = source.y - sampleY;
    const float distSq = std::max(1.0f, dx * dx + dy * dy);
    const float dist = std::sqrt(distSq);
    SolarLightSample sample;
    sample.dirX = dx / dist;
    sample.dirY = dy / dist;
    sample.intensity = std::max(source.safeMinimum, source.power * 8000.0f / distSq);
    sample.heatPressure = sample.intensity * (1.0f + std::max(0.0f, source.flarePressure));
    sample.solarChargeBonus = std::min(3.0f, sample.intensity * 0.5f);
    return sample;
}

std::string SolarLightSummary(const SolarLightSample& sample) {
    std::ostringstream out;
    out.precision(2);
    out << std::fixed << "light=" << sample.intensity << " heat=" << sample.heatPressure << " charge=" << sample.solarChargeBonus;
    return out.str();
}

} // namespace subspace
