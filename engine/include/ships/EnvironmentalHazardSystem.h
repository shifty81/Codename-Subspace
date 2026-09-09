#pragma once
#include "ships/HazardCertificationSystem.h"
#include <unordered_map>
#include <vector>
namespace subspace {
struct EnvironmentalCondition { HazardType type=HazardType::Radiation; double intensity=0; double exposurePerHour=1; };
struct EnvironmentalProtection { std::unordered_map<int,double> resistance; };
struct EnvironmentalExposureReport { double totalDamagePerHour=0; double sensorPenalty=0; double propulsionPenalty=0; std::vector<HazardType> dangerous; };
class EnvironmentalHazardSystem {
public:
 EnvironmentalExposureReport Evaluate(const std::vector<EnvironmentalCondition>& conditions,const EnvironmentalProtection& protection) const;
};
}
