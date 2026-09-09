#pragma once

#include "ships/ShipConstructionSystem.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ShipyardJobState { WaitingMaterials, Queued, Building, Complete, Cancelled };
struct MaterialSubstitutionRule { ConstructionMaterial from=ConstructionMaterial::StructuralSteel;ConstructionMaterial to=ConstructionMaterial::TitaniumAlloy;double quantityMultiplier=1;double massMultiplier=1;double timeMultiplier=1; };
struct ShipyardJob { std::uint64_t id=0;std::string blueprintName;ShipBillOfMaterials bill;ShipyardJobState state=ShipyardJobState::WaitingMaterials;double progress=0;double materialEfficiency=1;double timeEfficiency=1; };

class ShipProductionWorkflowSystem {
public:
    ShipBillOfMaterials ApplySubstitution(const ShipBillOfMaterials& bill,const MaterialSubstitutionRule& rule) const;
    std::uint64_t Queue(const ModularShipDesign& design,double materialEfficiency=1.0,double timeEfficiency=1.0);
    bool Supply(std::uint64_t jobId,std::unordered_map<int,double>& inventory);
    void Advance(std::uint64_t jobId,double minutes);
    const ShipyardJob* Get(std::uint64_t jobId) const;
    std::vector<ShipyardJob> Jobs() const;
private:
    std::uint64_t nextId_=1;std::unordered_map<std::uint64_t,ShipyardJob> jobs_;ShipConstructionSystem construction_;
};

} // namespace subspace
