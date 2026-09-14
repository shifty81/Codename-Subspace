#include "ship_editor/ShipyardAssetBrowserSystem.h"

#include "content/ShipyardPartTaxonomySystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {
std::string Lower(std::string value){
    std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return value;
}
bool Contains(const std::vector<std::string>& values,const std::string& value){return std::find(values.begin(),values.end(),value)!=values.end();}
bool MatchesAllTags(const std::vector<std::string>& tags,const std::vector<std::string>& required){
    for(const auto& requiredTag:required){
        const auto needle=Lower(requiredTag);bool found=false;
        for(const auto& tag:tags)if(Lower(tag)==needle){found=true;break;}
        if(!found)return false;
    }
    return true;
}
}

ShipyardAssetBrowserState ShipyardAssetBrowserSystem::DefaultState(){
    ShipyardAssetBrowserState out;
    out.savedFilters={
        {"certified-structure","Certified Structure","",{"STRUCTURAL"},false,false},
        {"generator-ready","Generator Ready","",{"GENERATOR ELIGIBLE"},false,false},
        {"compatible-selection","Compatible With Selection","",{},true,false},
        {"favorites","Favorites","",{},false,true}
    };
    return out;
}

std::vector<std::string> ShipyardAssetBrowserSystem::TagsFor(const ShipyardModuleRecord& record){
    std::vector<std::string> out;
    out.push_back(ShipyardModuleSystem::ClassName(record.moduleClass));
    out.push_back(ShipyardModuleSystem::SemanticName(record.semantic));
    out.push_back(ShipyardModuleSystem::SizeName(record.size));
    out.push_back(ShipyardPartTaxonomySystem::CategoryName(record.moduleClass));
    if(record.primaryHull)out.push_back("PRIMARY HULL");
    if(record.generatorEligible)out.push_back("GENERATOR ELIGIBLE");
    if(record.pairedPlacement||record.mirrorPreferred)out.push_back("SYMMETRY FRIENDLY");
    if(record.surfaceOnly)out.push_back("SURFACE ONLY");
    if(record.functional)out.push_back("FUNCTIONAL");
    if(record.placementRole!="EXCLUDED")out.push_back(record.placementRole);
    if(record.semantic==ShipyardModuleSemantic::StructuralFrame)out.push_back("STRUCTURAL");
    for(const auto& role:record.preferredRoles)out.push_back(role);
    std::sort(out.begin(),out.end());out.erase(std::unique(out.begin(),out.end()),out.end());
    return out;
}

bool ShipyardAssetBrowserSystem::CompatibleWith(const ShipyardModuleRecord& candidate,
                                                 const ShipyardModuleRecord* selectedParent){
    if(!selectedParent)return true;
    for(const auto& parentSocket:selectedParent->sockets)
        for(const auto& childSocket:candidate.sockets)
            if(ShipyardModuleSystem::CanMate(parentSocket.type,childSocket.type))return true;
    return false;
}

std::vector<ShipyardAssetBrowserItem> ShipyardAssetBrowserSystem::Query(const std::vector<ShipyardModuleRecord>& catalog,
                                                                        const ShipyardAssetBrowserState& state,
                                                                        const ShipyardModuleRecord* selectedParent){
    std::vector<ShipyardAssetBrowserItem> out;
    const auto needle=Lower(state.search);
    for(const auto& record:catalog){
        ShipyardAssetBrowserItem item;
        item.moduleId=record.source.moduleId;
        item.label=ShipyardPartTaxonomySystem::DisplayName(record);
        item.category=ShipyardPartTaxonomySystem::CategoryName(record.moduleClass);
        item.semantic=ShipyardModuleSystem::SemanticName(record.semantic);
        item.size=ShipyardModuleSystem::SizeName(record.size);
        item.tags=TagsFor(record);
        item.favorite=Contains(state.favorites,item.moduleId);
        item.compatible=CompatibleWith(record,selectedParent);
        item.generatorEligible=record.generatorEligible;
        if(state.favoritesOnly&&!item.favorite)continue;
        if(state.generatorEligibleOnly&&!item.generatorEligible)continue;
        if(state.compatibleOnly&&!item.compatible)continue;
        if(!state.showIncompatible&&!item.compatible)continue;
        if(!MatchesAllTags(item.tags,state.requiredTags))continue;
        const std::string haystack=Lower(item.moduleId+" "+item.label+" "+item.category+" "+item.semantic+" "+item.size);
        if(!needle.empty()&&haystack.find(needle)==std::string::npos){
            bool tagMatch=false;for(const auto& tag:item.tags)if(Lower(tag).find(needle)!=std::string::npos){tagMatch=true;break;}
            if(!tagMatch)continue;
        }
        item.score=(item.favorite?8.0f:0.0f)+(item.compatible?4.0f:0.0f)+(item.generatorEligible?1.0f:0.0f);
        if(!needle.empty()&&Lower(item.label).find(needle)==0)item.score+=6.0f;
        out.push_back(std::move(item));
    }
    std::stable_sort(out.begin(),out.end(),[](const auto& a,const auto& b){
        if(a.score!=b.score)return a.score>b.score;
        if(a.category!=b.category)return a.category<b.category;
        return a.label<b.label;
    });
    return out;
}

bool ShipyardAssetBrowserSystem::ToggleFavorite(ShipyardAssetBrowserState& state,const std::string& moduleId){
    const auto it=std::find(state.favorites.begin(),state.favorites.end(),moduleId);
    if(it==state.favorites.end()){state.favorites.push_back(moduleId);return true;}
    state.favorites.erase(it);return false;
}

void ShipyardAssetBrowserSystem::MarkRecent(ShipyardAssetBrowserState& state,const std::string& moduleId,std::size_t limit){
    state.recent.erase(std::remove(state.recent.begin(),state.recent.end(),moduleId),state.recent.end());
    state.recent.insert(state.recent.begin(),moduleId);
    if(state.recent.size()>limit)state.recent.resize(limit);
}

bool ShipyardAssetBrowserSystem::ApplySavedFilter(ShipyardAssetBrowserState& state,const std::string& filterId){
    const auto it=std::find_if(state.savedFilters.begin(),state.savedFilters.end(),[&](const auto& f){return f.id==filterId;});
    if(it==state.savedFilters.end())return false;
    state.search=it->query;state.requiredTags=it->requiredTags;state.compatibleOnly=it->compatibleOnly;state.favoritesOnly=it->favoritesOnly;return true;
}

float ShipyardAssetBrowserSystem::ClampThumbnailScale(float value){return std::clamp(value,.65f,1.85f);}

} // namespace subspace
