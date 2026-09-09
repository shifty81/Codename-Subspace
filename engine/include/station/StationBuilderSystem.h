#pragma once

#include "station/StationConstructionSystem.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct StationGridCoord { int x=0,y=0; bool operator==(const StationGridCoord&o) const{return x==o.x&&y==o.y;} };
struct StationPlacedModule { std::uint64_t instanceId=0; StationModuleNative module; StationGridCoord grid; int rotationQuarterTurns=0; };
struct StationBuildPlan { std::string name; StationPlacementRequest placement; std::vector<StationPlacedModule> modules; };
struct StationBuildValidation { bool valid=false; std::vector<std::string> errors; std::vector<std::string> warnings; double structuralMaterial=0; double netPower=0; double storage=0; };

class StationBuilderSystem {
public:
    StationGridCoord Snap(double worldX,double worldY,double gridSize=4.0) const;
    bool AddModule(StationBuildPlan& plan,const StationModuleNative& module,StationGridCoord grid,int rotationQuarterTurns=0);
    bool RemoveModule(StationBuildPlan& plan,std::uint64_t instanceId);
    StationBuildValidation Validate(const StationBuildPlan& plan) const;
    std::unordered_map<std::string,double> MaterialBill(const StationBuildPlan& plan) const;
private:
    std::uint64_t nextId_=1;
};

} // namespace subspace
