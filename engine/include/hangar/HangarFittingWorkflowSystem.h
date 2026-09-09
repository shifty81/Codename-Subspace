#pragma once

#include "ships/FittingSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct HangarFitState { FittingCapacity capacity; std::vector<std::string> installed; double repairPercent=1.0; double fuel=100.0; double ammunition=0.0; };
struct HangarServiceQuote { double repairCost=0; double refuelCost=0; double ammoCost=0; double total=0; };

class HangarFittingWorkflowSystem {
public:
    FittingResult Install(HangarFitState& state,const std::string& moduleId) const;
    bool Remove(HangarFitState& state,const std::string& moduleId) const;
    HangarServiceQuote Quote(const HangarFitState& state,double targetFuel,double targetAmmo) const;
    void Service(HangarFitState& state,double targetFuel,double targetAmmo) const;
private:
    FittingSystem fitting_;
};

} // namespace subspace
