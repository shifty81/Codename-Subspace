#include "interior/InteriorKitRegistrySystem.h"

#include <algorithm>
#include <utility>

namespace subspace {

bool InteriorKitRegistrySystem::Contains(const std::vector<std::string>&values,const std::string&value){return value.empty()||std::find(values.begin(),values.end(),value)!=values.end();}
bool InteriorKitRegistrySystem::SatisfiesKinds(const InteriorModuleKit&kit,const std::vector<InteriorModuleKind>&kinds){for(const auto required:kinds){if(std::none_of(kit.modules.begin(),kit.modules.end(),[&](const InteriorModuleAssetDef&m){return m.kind==required;}))return false;}return true;}
bool InteriorKitRegistrySystem::Register(InteriorKitRecord record){
    ModularInteriorKitSystem validator;const auto validation=validator.Validate(record.kit);if(!validation.valid)return false;
    if(record.kit.kitId.empty()||Find(record.kit.kitId))return false;_records.push_back(std::move(record));return true;
}
bool InteriorKitRegistrySystem::Remove(const std::string&id){const auto old=_records.size();_records.erase(std::remove_if(_records.begin(),_records.end(),[&](const InteriorKitRecord&r){return r.kit.kitId==id;}),_records.end());return _records.size()!=old;}
const InteriorKitRecord* InteriorKitRegistrySystem::Find(const std::string&id)const{for(const auto&r:_records)if(r.kit.kitId==id)return &r;return nullptr;}
InteriorKitSelectionResult InteriorKitRegistrySystem::Resolve(const InteriorKitSelectionRequest&request)const{
    InteriorKitSelectionResult out;const InteriorKitRecord*best=nullptr;int bestScore=-1000000;
    for(const auto&r:_records){
        if(request.requireHydrated&&!r.kit.hydrated)continue;if(!Contains(r.contexts,request.context))continue;if(!SatisfiesKinds(r.kit,request.requiredKinds))continue;
        int score=r.priority;if(!request.preferredFamily.empty()&&r.kit.family==request.preferredFamily)score+=1000;
        if(score>bestScore){best=&r;bestScore=score;}
    }
    if(best){out.kit=&best->kit;out.valid=true;out.status="RESOLVED";return out;}
    // A non-hydrated candidate is still useful to the authoring/hydration lane,
    // but never masquerades as runtime-ready.
    if(request.requireHydrated){
        InteriorKitSelectionRequest hydration=request;hydration.requireHydrated=false;auto pending=Resolve(hydration);
        if(pending.kit){out.kit=pending.kit;out.status="HYDRATION REQUIRED";out.warnings.push_back("matching interior kit exists but source meshes are not hydrated");return out;}
    }
    out.status="NO COMPATIBLE INTERIOR KIT";return out;
}

} // namespace subspace
