#pragma once

#include "station/StationModuleRole.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct StationSocket { std::string id; std::string type="standard"; bool occupied=false; };
struct ModularStationNode { std::uint64_t id=0; StationModuleFunction function=StationModuleFunction::Core; std::string name; double powerUse=0; double powerGeneration=0; double storage=0; std::vector<StationSocket> sockets; };
struct StationConnection { std::uint64_t a=0,b=0; std::string socketA,socketB; };
struct ModularStationPlan { std::vector<ModularStationNode> modules; std::vector<StationConnection> connections; };
struct ModularStationValidation { bool valid=false; double netPower=0; double storage=0; int docks=0; std::vector<std::string> errors; };
class ModularStationAssemblySystem {
public:
    bool Connect(ModularStationPlan& plan,std::uint64_t a,const std::string& socketA,std::uint64_t b,const std::string& socketB) const;
    ModularStationValidation Validate(const ModularStationPlan& plan) const;
};

} // namespace subspace
