#include "navigation/GalaxyMapSystem.h"
#include <algorithm>
namespace subspace {
std::vector<const GalaxySystemRecord*> GalaxyMapSystem::Filter(const std::vector<GalaxySystemRecord>&c,GalaxyOverlay o,float t) const {std::vector<const GalaxySystemRecord*> out;for(const auto&r:c){bool keep=true;switch(o){case GalaxyOverlay::Security:keep=r.security>=t;break;case GalaxyOverlay::Economy:keep=r.economy>=t;break;case GalaxyOverlay::Resources:keep=r.resourceRichness>=t;break;case GalaxyOverlay::Population:keep=r.populationTier>=int(t);break;case GalaxyOverlay::Shipyards:keep=r.hasShipyard;break;case GalaxyOverlay::Exploration:keep=r.discovered;break;case GalaxyOverlay::Anomalies:keep=r.hasAnomaly;break;default:break;}if(keep)out.push_back(&r);}return out;}
GalaxyMapSelection GalaxyMapSystem::Select(const std::vector<GalaxySystemRecord>&c,std::uint32_t id) const {GalaxyMapSelection s;for(const auto&r:c)if(r.id==id){s.valid=true;s.systemId=id;s.title=r.name;s.details={"Security "+std::to_string(int(r.security*100))+"%","Economy "+std::to_string(int(r.economy*100))+"%","Resources "+std::to_string(int(r.resourceRichness*100))+"%","Population Tier "+std::to_string(r.populationTier),r.hasShipyard?"Shipyard: Known":"Shipyard: None known",r.discovered?"Survey: Known":"Survey: Unknown"};break;}return s;}
void GalaxyMapSystem::Orbit(GalaxyMapCamera&c,float y,float p) const {c.yaw+=y;c.pitch=std::clamp(c.pitch+p,-85.0f,85.0f);}
void GalaxyMapSystem::Zoom(GalaxyMapCamera&c,float d) const {c.distance=std::clamp(c.distance-d*240.0f,120.0f,12000.0f);}
} // namespace subspace
