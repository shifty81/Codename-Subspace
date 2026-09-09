#include "navigation/UniverseSystemMapSystem.h"
#include <unordered_map>

namespace subspace {
UniverseSystemMapSnapshot UniverseSystemMapSystem::Build(const std::vector<OrbitalBodyRecord>& bodies,double t,int samples) const {UniverseSystemMapSnapshot out;out.simulationSeconds=t;OrbitalDynamicsSystem dyn;std::unordered_map<std::uint64_t,Vector3> pos;for(const auto&b:bodies){Vector3 parent{};auto it=pos.find(b.parentId);if(it!=pos.end())parent=it->second;const Vector3 p=dyn.Evaluate(b,t,parent);pos[b.id]=p;UniverseMapNode n;n.id=b.id;n.name=b.name;n.kind=b.kind;n.currentPosition=p;n.dockable=b.dockable;if(b.kind!=OrbitalBodyKind::Star&&samples>3){n.orbitTrack.reserve(samples);for(int i=0;i<samples;++i){const double f=double(i)/samples;n.orbitTrack.push_back(dyn.Evaluate(b,b.orbit.epochSeconds+b.orbit.orbitalPeriodSeconds*f,parent));}}out.nodes.push_back(std::move(n));}return out;}
} // namespace subspace
