#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class PowerTechnology { Burner, RefinedCarbon, Chemical, Solar, Wind, Fission, Fusion, ProtonPlasma, Experimental, Alien };
struct FuelDefinition { std::string id; PowerTechnology tech=PowerTechnology::Burner; double energyDensity=1; double burnSecondsPerUnit=10; double heat=1; bool manufactured=true; };
struct InfrastructureModuleSpec { std::string id; double powerUse=0; double powerGeneration=0; double storage=0; double maintenance=0; double efficiency=1; PowerTechnology minimumTech=PowerTechnology::Burner; std::vector<std::string> inputs,outputs; };
class InfrastructureModuleSystem {
public:
    std::vector<FuelDefinition> FuelProgression() const;
    double EffectiveBurnTime(const FuelDefinition& fuel,double efficiencyUpgrade) const;
    bool CanOperate(const InfrastructureModuleSpec& module,PowerTechnology unlocked,double availablePower) const;
};

} // namespace subspace
