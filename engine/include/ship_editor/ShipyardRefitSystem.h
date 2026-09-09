#pragma once
#include "rendering/ProceduralVisualVariantSystem.h"
#include <string>
#include <vector>
namespace subspace {
enum class ShipyardRefitPartSource { ShipCargo, StationStorage, Looted, Crafted, StationMarket };
struct ShipyardRefitPart { std::string moduleId; int quantity=1; float condition=1.0f; ShipyardRefitPartSource source=ShipyardRefitPartSource::ShipCargo; };
struct ShipyardRefitDelta { std::vector<std::string> incoming; std::vector<std::string> outgoing; bool valid=true; std::string reason; };
struct ShipyardRefitSession { bool active=false; ProceduralShipVisualRecipe original; std::vector<ShipyardRefitPart> inventory; };
class ShipyardRefitSystem {
public:
    static ShipyardRefitSession Begin(const ProceduralShipVisualRecipe& current,std::vector<ShipyardRefitPart> inventory);
    static ShipyardRefitDelta Preview(const ShipyardRefitSession& session,const ProceduralShipVisualRecipe& working);
    static bool Commit(ShipyardRefitSession& session,const ProceduralShipVisualRecipe& working,ShipyardRefitDelta* delta=nullptr);
    static void Rollback(ShipyardRefitSession& session){session.active=false;}
    static std::vector<std::string> AvailableModuleIds(const ShipyardRefitSession& session);
};
}
