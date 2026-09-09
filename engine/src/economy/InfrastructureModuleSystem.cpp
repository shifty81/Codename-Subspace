#include "economy/InfrastructureModuleSystem.h"
#include <algorithm>
namespace subspace {
std::vector<FuelDefinition> InfrastructureModuleSystem::FuelProgression() const{return {{"raw_carbon",PowerTechnology::Burner,1,12,1.2,true},{"refined_carbon",PowerTechnology::RefinedCarbon,2.2,34,.9,true},{"chemical_cell",PowerTechnology::Chemical,4.0,70,.8,true},{"fission_rod",PowerTechnology::Fission,22,1800,.55,true},{"fusion_pellet",PowerTechnology::Fusion,80,7200,.32,true},{"proton_capsule",PowerTechnology::ProtonPlasma,180,18000,.22,true},{"exotic_cell",PowerTechnology::Experimental,420,54000,.16,true},{"alien_core",PowerTechnology::Alien,1000,180000,.08,false}};}
double InfrastructureModuleSystem::EffectiveBurnTime(const FuelDefinition&f,double upgrade) const{return f.burnSecondsPerUnit*std::clamp(1.0+upgrade,1.0,6.0);}
bool InfrastructureModuleSystem::CanOperate(const InfrastructureModuleSpec&m,PowerTechnology unlocked,double p) const{return static_cast<int>(unlocked)>=static_cast<int>(m.minimumTech)&&p+m.powerGeneration>=m.powerUse;}
} // namespace subspace
