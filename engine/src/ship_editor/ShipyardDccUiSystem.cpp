#include "ship_editor/ShipyardDccUiSystem.h"

#include "content/ShipyardPartTaxonomySystem.h"

#include <algorithm>
#include <limits>
#include <tuple>

namespace subspace {
namespace {
const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){
    const auto it=std::find_if(catalog.begin(),catalog.end(),[&](const auto& record){return record.source.moduleId==id;});
    return it==catalog.end()?nullptr:&*it;
}
std::size_t ParentOf(const ProceduralShipVisualRecipe& recipe,std::size_t child){
    for(const auto& edge:recipe.attachments)if(edge.childModuleIndex==child&&edge.parentModuleIndex<recipe.modules.size())return edge.parentModuleIndex;
    return std::numeric_limits<std::size_t>::max();
}
std::size_t DepthOf(const ProceduralShipVisualRecipe& recipe,std::size_t index){
    std::size_t depth=0,current=index;
    for(std::size_t guard=0;guard<recipe.modules.size();++guard){
        const auto parent=ParentOf(recipe,current);if(parent==std::numeric_limits<std::size_t>::max()||parent==current)break;
        ++depth;current=parent;
    }
    return std::min<std::size_t>(depth,5);
}
std::string GroupFor(const ShipyardModuleRecord* record,ShipyardDccOutlinerMode mode){
    if(!record)return "UNCLASSIFIED";
    if(mode==ShipyardDccOutlinerMode::FunctionalRole)return record->placementRole.empty()?"GENERAL":record->placementRole;
    if(mode==ShipyardDccOutlinerMode::ModuleClass)return ShipyardModuleSystem::ClassName(record->moduleClass);
    return "HIERARCHY";
}
}

ShipyardDccUiState ShipyardDccUiSystem::DefaultState(){
    ShipyardDccUiState out;
    out.assetBrowser=ShipyardAssetBrowserSystem::DefaultState();
    out.assetBrowser.density=ShipyardAssetBrowserDensity::Comfortable;
    out.assetBrowser.thumbnailScale=1.0f;
    return out;
}

void ShipyardDccUiSystem::ResetLayout(ShipyardDccUiState& state){
    const auto favorites=state.assetBrowser.favorites;
    const auto recent=state.assetBrowser.recent;
    state=DefaultState();
    state.assetBrowser.favorites=favorites;
    state.assetBrowser.recent=recent;
}

const char* ShipyardDccUiSystem::OutlinerModeName(ShipyardDccOutlinerMode mode){
    switch(mode){
    case ShipyardDccOutlinerMode::Hierarchy:return "HIERARCHY";
    case ShipyardDccOutlinerMode::FunctionalRole:return "ROLE";
    case ShipyardDccOutlinerMode::ModuleClass:return "CLASS";
    }
    return "HIERARCHY";
}

const char* ShipyardDccUiSystem::ShadingName(ShipyardDccViewportShading mode){
    switch(mode){
    case ShipyardDccViewportShading::Solid:return "SOLID";
    case ShipyardDccViewportShading::Material:return "MATERIAL";
    case ShipyardDccViewportShading::Wireframe:return "WIREFRAME";
    }
    return "MATERIAL";
}

const char* ShipyardDccUiSystem::AssetDensityName(ShipyardAssetBrowserDensity density){
    switch(density){
    case ShipyardAssetBrowserDensity::Compact:return "COMPACT";
    case ShipyardAssetBrowserDensity::Comfortable:return "COMFORTABLE";
    case ShipyardAssetBrowserDensity::Large:return "LARGE";
    }
    return "COMFORTABLE";
}

ShipyardDccOutlinerMode ShipyardDccUiSystem::NextOutlinerMode(ShipyardDccOutlinerMode mode){
    switch(mode){
    case ShipyardDccOutlinerMode::Hierarchy:return ShipyardDccOutlinerMode::FunctionalRole;
    case ShipyardDccOutlinerMode::FunctionalRole:return ShipyardDccOutlinerMode::ModuleClass;
    case ShipyardDccOutlinerMode::ModuleClass:return ShipyardDccOutlinerMode::Hierarchy;
    }
    return ShipyardDccOutlinerMode::Hierarchy;
}

ShipyardDccViewportShading ShipyardDccUiSystem::NextShading(ShipyardDccViewportShading mode){
    switch(mode){
    case ShipyardDccViewportShading::Solid:return ShipyardDccViewportShading::Material;
    case ShipyardDccViewportShading::Material:return ShipyardDccViewportShading::Wireframe;
    case ShipyardDccViewportShading::Wireframe:return ShipyardDccViewportShading::Solid;
    }
    return ShipyardDccViewportShading::Material;
}

ShipyardAssetBrowserDensity ShipyardDccUiSystem::NextAssetDensity(ShipyardAssetBrowserDensity density){
    switch(density){
    case ShipyardAssetBrowserDensity::Compact:return ShipyardAssetBrowserDensity::Comfortable;
    case ShipyardAssetBrowserDensity::Comfortable:return ShipyardAssetBrowserDensity::Large;
    case ShipyardAssetBrowserDensity::Large:return ShipyardAssetBrowserDensity::Compact;
    }
    return ShipyardAssetBrowserDensity::Comfortable;
}

std::size_t ShipyardDccUiSystem::AssetPresetCount(){return 5;}
const char* ShipyardDccUiSystem::AssetPresetName(std::size_t preset){
    switch(preset%AssetPresetCount()){
    case 0:return "ALL";
    case 1:return "CERTIFIED STRUCTURE";
    case 2:return "GENERATOR READY";
    case 3:return "COMPATIBLE";
    case 4:return "FAVORITES";
    }
    return "ALL";
}

bool ShipyardDccUiSystem::ApplyAssetPreset(ShipyardDccUiState& state,std::size_t preset){
    state.assetPreset=preset%AssetPresetCount();
    state.assetBrowser.search.clear();
    state.assetBrowser.requiredTags.clear();
    state.assetBrowser.compatibleOnly=false;
    state.assetBrowser.favoritesOnly=false;
    state.assetBrowser.generatorEligibleOnly=false;
    state.assetBrowser.showIncompatible=true;
    switch(state.assetPreset){
    case 0:return true;
    case 1:state.assetBrowser.requiredTags={"STRUCTURAL"};return true;
    case 2:state.assetBrowser.generatorEligibleOnly=true;return true;
    case 3:state.assetBrowser.compatibleOnly=true;state.assetBrowser.showIncompatible=false;return true;
    case 4:state.assetBrowser.favoritesOnly=true;return true;
    default:return true;
    }
}

std::vector<ShipyardDccOutlinerRow> ShipyardDccUiSystem::BuildOutlinerRows(
    const std::vector<ShipyardModuleRecord>& catalog,
    const ProceduralShipVisualRecipe& recipe,
    std::size_t selectedModule,
    ShipyardDccOutlinerMode mode){
    std::vector<ShipyardDccOutlinerRow> rows;
    rows.reserve(recipe.modules.size());
    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto* record=FindRecord(catalog,recipe.modules[i].moduleId);
        ShipyardDccOutlinerRow row;
        row.moduleIndex=i;
        row.depth=mode==ShipyardDccOutlinerMode::Hierarchy?DepthOf(recipe,i):0;
        row.label=record?ShipyardPartTaxonomySystem::DisplayName(*record):recipe.modules[i].moduleId;
        row.group=GroupFor(record,mode);
        row.selected=i==selectedModule;
        row.attached=ParentOf(recipe,i)!=std::numeric_limits<std::size_t>::max();
        rows.push_back(std::move(row));
    }
    if(mode!=ShipyardDccOutlinerMode::Hierarchy){
        std::stable_sort(rows.begin(),rows.end(),[](const auto& a,const auto& b){return std::tie(a.group,a.label,a.moduleIndex)<std::tie(b.group,b.label,b.moduleIndex);});
    }
    return rows;
}

std::vector<ShipyardDccCommandPaletteItem> ShipyardDccUiSystem::CommandPalette(){
    return {
        {"Grab / Move","G","Transform"},{"Rotate","R","Transform"},{"Scale","S","Transform"},
        {"Frame Selected","F","View"},{"Frame Ship","Home","View"},{"Toggle Toolbar","T","View"},
        {"Toggle Properties Sidebar","N","View"},{"Maximize 3D View","Ctrl+Space","View"},{"Command Search","F3","View"},
        {"Undo","Ctrl+Z","Edit"},{"Redo","Ctrl+Shift+Z","Edit"},{"Delete Module","Delete","Edit"},
        {"Constrain X","X","Transform"},{"Constrain Y","Y","Transform"},{"Constrain Z","Z","Transform"},
        {"Assembly Workspace","1","Workspace"},{"Model Workspace","2","Workspace"},{"Interior Workspace","3","Workspace"},
        {"Systems Workspace","4","Workspace"},{"Paint Workspace","5","Workspace"},{"Test Workspace","6","Workspace"},
        {"Cycle Asset Filter","Shift+F","Asset Browser"}
    };
}

std::vector<ShipyardWorkspaceMode> ShipyardDccUiSystem::WorkspaceCycle(){
    return {ShipyardWorkspaceMode::Build,ShipyardWorkspaceMode::Model,ShipyardWorkspaceMode::Interior,
            ShipyardWorkspaceMode::Systems,ShipyardWorkspaceMode::Appearance,ShipyardWorkspaceMode::Test,
            ShipyardWorkspaceMode::Authoring,ShipyardWorkspaceMode::Pcg,ShipyardWorkspaceMode::ProjectTools};
}

} // namespace subspace
