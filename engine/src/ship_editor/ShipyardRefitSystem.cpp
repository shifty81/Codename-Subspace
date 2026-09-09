#include "ship_editor/ShipyardRefitSystem.h"
#include <algorithm>
#include <unordered_map>
namespace subspace {
ShipyardRefitSession ShipyardRefitSystem::Begin(const ProceduralShipVisualRecipe& current,std::vector<ShipyardRefitPart> inventory){ShipyardRefitSession s;s.active=true;s.original=current;s.inventory=std::move(inventory);return s;}
ShipyardRefitDelta ShipyardRefitSystem::Preview(const ShipyardRefitSession& s,const ProceduralShipVisualRecipe& w){
    ShipyardRefitDelta d;if(!s.active){d.valid=false;d.reason="No docked Shipyard refit transaction";return d;}
    std::unordered_map<std::string,int> before,after,stock;for(const auto&p:s.original.modules)++before[p.moduleId];for(const auto&p:w.modules)++after[p.moduleId];for(const auto&i:s.inventory)stock[i.moduleId]+=std::max(0,i.quantity);
    for(const auto& [id,n]:after){int add=std::max(0,n-before[id]);for(int i=0;i<add;++i)d.incoming.push_back(id);if(add>stock[id]){d.valid=false;d.reason="Required refit part is not in ship/station/loot/crafted inventory: "+id;}}
    for(const auto& [id,n]:before){int rem=std::max(0,n-after[id]);for(int i=0;i<rem;++i)d.outgoing.push_back(id);}return d;
}
bool ShipyardRefitSystem::Commit(ShipyardRefitSession& s,const ProceduralShipVisualRecipe& w,ShipyardRefitDelta* out){auto d=Preview(s,w);if(out)*out=d;if(!d.valid)return false;for(const auto& id:d.incoming){for(auto& i:s.inventory)if(i.moduleId==id&&i.quantity>0){--i.quantity;break;}}for(const auto& id:d.outgoing)s.inventory.push_back({id,1,1.0f,ShipyardRefitPartSource::ShipCargo});s.original=w;s.active=false;return true;}
std::vector<std::string> ShipyardRefitSystem::AvailableModuleIds(const ShipyardRefitSession& s){std::vector<std::string> v;for(const auto&i:s.inventory)if(i.quantity>0&&std::find(v.begin(),v.end(),i.moduleId)==v.end())v.push_back(i.moduleId);for(const auto&p:s.original.modules)if(std::find(v.begin(),v.end(),p.moduleId)==v.end())v.push_back(p.moduleId);return v;}
}
