#include "ship_editor/ShipyardBuilderSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "editor/EditorDccShellLayoutSystem.h"

#include <algorithm>
#include <tuple>

namespace subspace {
namespace {

bool HasPlaced(const ShipyardBuilderRuntimeModel& model){return !model.recipe.modules.empty();}

bool IsAdvancedWorkspace(ShipyardWorkspaceMode mode){
    switch(mode){
    case ShipyardWorkspaceMode::Model:
    case ShipyardWorkspaceMode::Character:
    case ShipyardWorkspaceMode::Pcg:
    case ShipyardWorkspaceMode::World:
    case ShipyardWorkspaceMode::DevWorld:
    case ShipyardWorkspaceMode::ProjectTools:
    case ShipyardWorkspaceMode::Authoring:
        return true;
    default:
        return false;
    }
}

std::string ModuleSerial(const std::string& id){
    const std::string prefix="shipyard_a_";
    if(id.rfind(prefix,0)!=0)return {};
    const auto classEnd=id.find('_',prefix.size());
    if(classEnd==std::string::npos)return {};
    const auto serialEnd=id.find('_',classEnd+1);
    if(serialEnd==std::string::npos)return {};
    return id.substr(classEnd+1,serialEnd-classEnd-1);
}

std::string FriendlyModuleLabel(const ShipyardModuleRecord& record){
    std::string label=ShipyardPartTaxonomySystem::DisplayName(record);
    const auto serial=ModuleSerial(record.source.moduleId);
    if(!serial.empty() && label.find(serial)==std::string::npos)label+=" "+serial;
    return label;
}

ShipyardBuilderCommand WorkspaceCommand(ShipyardWorkspaceMode mode){
    switch(mode){
    case ShipyardWorkspaceMode::Build:return ShipyardBuilderCommand::WorkspaceBuild;
    case ShipyardWorkspaceMode::Model:return ShipyardBuilderCommand::WorkspaceModel;
    case ShipyardWorkspaceMode::Interior:return ShipyardBuilderCommand::WorkspaceInterior;
    case ShipyardWorkspaceMode::Appearance:return ShipyardBuilderCommand::WorkspaceAppearance;
    case ShipyardWorkspaceMode::Systems:return ShipyardBuilderCommand::WorkspaceSystems;
    case ShipyardWorkspaceMode::Character:return ShipyardBuilderCommand::WorkspaceCharacter;
    case ShipyardWorkspaceMode::Pcg:return ShipyardBuilderCommand::WorkspacePcg;
    case ShipyardWorkspaceMode::World:return ShipyardBuilderCommand::WorkspaceWorld;
    case ShipyardWorkspaceMode::DevWorld:return ShipyardBuilderCommand::WorkspaceDevWorld;
    case ShipyardWorkspaceMode::ProjectTools:return ShipyardBuilderCommand::WorkspaceProjectTools;
    case ShipyardWorkspaceMode::Authoring:return ShipyardBuilderCommand::WorkspaceAuthoring;
    case ShipyardWorkspaceMode::Test:return ShipyardBuilderCommand::WorkspaceDevWorld;
    }
    return ShipyardBuilderCommand::WorkspaceBuild;
}
std::size_t InspectorIndex(ShipyardInspectorTab tab){
    switch(tab){case ShipyardInspectorTab::Transform:return 0;case ShipyardInspectorTab::Assembly:return 1;case ShipyardInspectorTab::Sockets:return 2;case ShipyardInspectorTab::Authoring:return 3;case ShipyardInspectorTab::Appearance:return 4;}return 0;
}
ShipyardBuilderCommand InspectorCommand(std::size_t index){
    switch(index%5){case 0:return ShipyardBuilderCommand::InspectorTransform;case 1:return ShipyardBuilderCommand::InspectorAssembly;case 2:return ShipyardBuilderCommand::InspectorSockets;case 3:return ShipyardBuilderCommand::InspectorAuthoring;case 4:return ShipyardBuilderCommand::InspectorAppearance;}return ShipyardBuilderCommand::InspectorTransform;
}

} // namespace

bool ShipyardBuilderSystem::Activate(ShipyardBuilderCommand command,int value){
    switch(command){
    case ShipyardBuilderCommand::DccToggleAssetBrowser:model_.dcc.showAssetBrowser=!model_.dcc.showAssetBrowser;model_.status=model_.dcc.showAssetBrowser?"Asset Browser shown":"Asset Browser hidden";return true;
    case ShipyardBuilderCommand::DccToggleToolRail:model_.dcc.showToolRail=!model_.dcc.showToolRail;model_.status=model_.dcc.showToolRail?"Toolbar shown":"Toolbar hidden";return true;
    case ShipyardBuilderCommand::DccToggleSidebar:model_.dcc.showSidebar=!model_.dcc.showSidebar;model_.dcc.showProperties=model_.dcc.showSidebar;model_.status=model_.dcc.showSidebar?"Properties sidebar shown":"Properties sidebar hidden";return true;
    case ShipyardBuilderCommand::DccToggleOutliner:model_.dcc.showOutliner=!model_.dcc.showOutliner;model_.status=model_.dcc.showOutliner?"Outliner shown":"Outliner hidden";return true;
    case ShipyardBuilderCommand::DccToggleProperties:model_.dcc.showProperties=!model_.dcc.showProperties;model_.status=model_.dcc.showProperties?"Properties shown":"Properties hidden";return true;
    case ShipyardBuilderCommand::DccToggleStatusBar:model_.dcc.showStatusBar=!model_.dcc.showStatusBar;return true;
    case ShipyardBuilderCommand::DccToggleMaximizeViewport:model_.dcc.maximizeViewport=!model_.dcc.maximizeViewport;model_.status=model_.dcc.maximizeViewport?"3D View maximized - Ctrl+Space to restore":"3D View restored";return true;
    case ShipyardBuilderCommand::DccResetLayout:ShipyardDccUiSystem::ResetLayout(model_.dcc);model_.status="Blender DCC layout reset";return true;
    case ShipyardBuilderCommand::DccToggleGrid:model_.dcc.showGrid=!model_.dcc.showGrid;model_.status=model_.dcc.showGrid?"Viewport grid shown":"Viewport grid hidden";return true;
    case ShipyardBuilderCommand::DccToggleGizmos:model_.dcc.showGizmos=!model_.dcc.showGizmos;model_.status=model_.dcc.showGizmos?"Transform gizmos shown":"Transform gizmos hidden";return true;
    case ShipyardBuilderCommand::DccToggleSocketsOverlay:model_.dcc.showSocketOverlay=!model_.dcc.showSocketOverlay;model_.status=model_.dcc.showSocketOverlay?"Socket overlay shown":"Socket overlay hidden";return true;
    case ShipyardBuilderCommand::DccToggleStatsOverlay:model_.dcc.showStatsOverlay=!model_.dcc.showStatsOverlay;model_.status=model_.dcc.showStatsOverlay?"Viewport statistics shown":"Viewport statistics hidden";return true;
    case ShipyardBuilderCommand::DccToggleShieldPreview:model_.dcc.showShieldPreview=!model_.dcc.showShieldPreview;model_.status=model_.dcc.showShieldPreview?"Conformal shield preview shown":"Conformal shield preview hidden";return true;
    case ShipyardBuilderCommand::DccCycleShading:model_.dcc.shading=ShipyardDccUiSystem::NextShading(model_.dcc.shading);model_.status=std::string("Viewport shading: ")+ShipyardDccUiSystem::ShadingName(model_.dcc.shading);return true;
    case ShipyardBuilderCommand::DccCycleOutlinerMode:model_.dcc.outlinerMode=ShipyardDccUiSystem::NextOutlinerMode(model_.dcc.outlinerMode);model_.status=std::string("Outliner mode: ")+ShipyardDccUiSystem::OutlinerModeName(model_.dcc.outlinerMode);return true;
    case ShipyardBuilderCommand::DccCycleAssetDensity:model_.dcc.assetBrowser.density=ShipyardDccUiSystem::NextAssetDensity(model_.dcc.assetBrowser.density);model_.status=std::string("Asset density: ")+ShipyardDccUiSystem::AssetDensityName(model_.dcc.assetBrowser.density);return true;
    case ShipyardBuilderCommand::DccAssetZoomIn:model_.dcc.assetBrowser.thumbnailScale=ShipyardAssetBrowserSystem::ClampThumbnailScale(model_.dcc.assetBrowser.thumbnailScale+.15f);return true;
    case ShipyardBuilderCommand::DccAssetZoomOut:model_.dcc.assetBrowser.thumbnailScale=ShipyardAssetBrowserSystem::ClampThumbnailScale(model_.dcc.assetBrowser.thumbnailScale-.15f);return true;
    case ShipyardBuilderCommand::DccPreviousAssetPreset:{const auto n=ShipyardDccUiSystem::AssetPresetCount();model_.dcc.assetPreset=(model_.dcc.assetPreset+n-1)%n;ShipyardDccUiSystem::ApplyAssetPreset(model_.dcc,model_.dcc.assetPreset);model_.selectedFilteredModule=0;model_.catalogScrollStart=0;model_.status=std::string("Asset filter: ")+ShipyardDccUiSystem::AssetPresetName(model_.dcc.assetPreset);return true;}
    case ShipyardBuilderCommand::DccNextAssetPreset:{model_.dcc.assetPreset=(model_.dcc.assetPreset+1)%ShipyardDccUiSystem::AssetPresetCount();ShipyardDccUiSystem::ApplyAssetPreset(model_.dcc,model_.dcc.assetPreset);model_.selectedFilteredModule=0;model_.catalogScrollStart=0;model_.status=std::string("Asset filter: ")+ShipyardDccUiSystem::AssetPresetName(model_.dcc.assetPreset);return true;}
    case ShipyardBuilderCommand::DccToggleFavoriteSelected:{const auto* selected=SelectedCatalogModule();if(!selected)return false;const bool favorite=ShipyardAssetBrowserSystem::ToggleFavorite(model_.dcc.assetBrowser,selected->source.moduleId);model_.status=favorite?"Module added to favorites":"Module removed from favorites";return true;}
    case ShipyardBuilderCommand::DccClearAssetFilters:ShipyardDccUiSystem::ApplyAssetPreset(model_.dcc,0);model_.selectedFilteredModule=0;model_.catalogScrollStart=0;model_.status="Asset Browser filters cleared";return true;
    case ShipyardBuilderCommand::DccToggleCommandPalette:model_.dcc.commandPaletteOpen=!model_.dcc.commandPaletteOpen;model_.status=model_.dcc.commandPaletteOpen?"Command Search open":"Command Search closed";return true;
    case ShipyardBuilderCommand::DccWorkspacePrevious:
    case ShipyardBuilderCommand::DccWorkspaceNext:{
        const auto order=ShipyardDccUiSystem::WorkspaceCycle();auto it=std::find(order.begin(),order.end(),model_.testWorkspaceActive?ShipyardWorkspaceMode::Test:model_.workspaceMode);
        std::size_t index=it==order.end()?0:static_cast<std::size_t>(std::distance(order.begin(),it));
        if(command==ShipyardBuilderCommand::DccWorkspaceNext)index=(index+1)%order.size();else index=(index+order.size()-1)%order.size();
        const auto next=order[index];if(next==ShipyardWorkspaceMode::Test){model_.workspaceMode=ShipyardWorkspaceMode::Test;model_.testWorkspaceActive=true;model_.developerWorkspacesVisible=false;model_.status="Test workspace";return true;}
        model_.testWorkspaceActive=false;return LegacyActivate(WorkspaceCommand(next),0);}
    case ShipyardBuilderCommand::DccPropertiesPrevious:
    case ShipyardBuilderCommand::DccPropertiesNext:{const auto current=InspectorIndex(model_.inspectorTab);const auto next=(current+5+(command==ShipyardBuilderCommand::DccPropertiesNext?1:-1))%5;return LegacyActivate(InspectorCommand(next),0);}
    case ShipyardBuilderCommand::WorkspaceDevWorld:
        if(value==-1268){model_.workspaceMode=ShipyardWorkspaceMode::Test;model_.testWorkspaceActive=true;model_.developerWorkspacesVisible=false;model_.status="Test workspace - validate, frame, save draft, and prepare embodied playtest";return true;}
        break;
    case ShipyardBuilderCommand::WorkspaceAuthoring:
        if(value==-1268){model_.developerWorkspacesVisible=!model_.developerWorkspacesVisible;model_.testWorkspaceActive=false;model_.status=model_.developerWorkspacesVisible?"Developer workspaces expanded":"Developer workspaces collapsed";return true;}
        break;
    case ShipyardBuilderCommand::PcgReroll:
        if(value==-1268){const auto before=model_;model_.testWorkspaceActive=false;if(!ActivateInternal(ShipyardBuilderCommand::PcgReroll,0))return false;const bool generated=ActivateInternal(ShipyardBuilderCommand::GenerateVariant,0);if(generated)PushAuthoringSnapshot(before);return generated;}
        break;
    default:break;
    }

    const bool result=LegacyActivate(command,value);
    if(result){
        switch(command){
        case ShipyardBuilderCommand::WorkspaceBuild:case ShipyardBuilderCommand::WorkspaceInterior:case ShipyardBuilderCommand::WorkspaceSystems:case ShipyardBuilderCommand::WorkspaceAppearance:
            model_.testWorkspaceActive=false;model_.developerWorkspacesVisible=false;break;
        case ShipyardBuilderCommand::WorkspaceModel:case ShipyardBuilderCommand::WorkspaceCharacter:case ShipyardBuilderCommand::WorkspacePcg:case ShipyardBuilderCommand::WorkspaceWorld:case ShipyardBuilderCommand::WorkspaceDevWorld:case ShipyardBuilderCommand::WorkspaceProjectTools:case ShipyardBuilderCommand::WorkspaceAuthoring:
            model_.testWorkspaceActive=false;model_.developerWorkspacesVisible=true;break;
        case ShipyardBuilderCommand::InspectorSockets:model_.testWorkspaceActive=false;model_.developerWorkspacesVisible=true;break;
        default:break;
        }
    }
    return result;
}

ShipyardBuilderLayout ShipyardBuilderSystem::Layout(int w,int h){
    ShipyardBuilderLayout l;
    const auto dcc=EditorDccShellLayoutSystem::Compute(w,h);
    if(!dcc.valid)return l;
    l.valid=true;
    l.compact=dcc.compact;
    l.uiScale=dcc.uiScale;
    const float s=l.uiScale;

    // PASS1444-1453: visible Shipyard composition is now driven by the shared
    // DCC shell rather than a bespoke left-library/right-inspector dashboard.
    l.workspaceBarY=dcc.workspaceStrip.y;
    l.workspaceBarHeight=dcc.workspaceStrip.height;
    l.left=8.0f*s;
    l.top=dcc.viewport.y;
    l.leftWidth=std::clamp(static_cast<float>(w)*.155f,260.0f*s,330.0f*s); // legacy compatibility metric
    l.right=dcc.outliner.x;
    l.rightWidth=dcc.outliner.width;
    l.rowHeight=(l.compact?27.0f:30.0f)*s;
    l.rowGap=4.0f*s;
    l.tabRowY=l.workspaceBarY;
    l.tabHeight=l.workspaceBarHeight;

    l.viewportLeft=dcc.viewport.x;
    l.viewportTop=dcc.viewport.y;
    l.viewportRight=dcc.viewport.x+dcc.viewport.width;
    l.viewportBottom=dcc.viewport.y+dcc.viewport.height;
    l.assetShelfX=dcc.assetShelf.x;
    l.assetShelfY=dcc.assetShelf.y;
    l.assetShelfWidth=dcc.assetShelf.width;
    l.assetShelfHeight=dcc.assetShelf.height;
    l.outlinerX=dcc.outliner.x;
    l.outlinerY=dcc.outliner.y;
    l.outlinerWidth=dcc.outliner.width;
    l.outlinerHeight=dcc.outliner.height;
    l.propertiesX=dcc.properties.x;
    l.propertiesY=dcc.properties.y;
    l.propertiesWidth=dcc.properties.width;
    l.propertiesHeight=dcc.properties.height;

    l.toolRailX=dcc.toolRail.x;
    l.toolRailY=dcc.toolRail.y;
    l.toolRailWidth=dcc.toolRail.width;

    // Asset Browser is a bottom shelf. Legacy y-fields are intentionally
    // projected into that shelf for test/automation compatibility.
    l.libraryListY=l.assetShelfY+34.0f*s;
    l.moduleCardsY=l.assetShelfY+67.0f*s;
    l.moduleCardHeight=(l.compact?62.0f:78.0f)*s;
    l.leftActionsY=l.assetShelfY+8.0f*s;
    l.leftInfoY=l.assetShelfY+8.0f*s;

    // Outliner owns the upper-right area; Properties owns the lower-right.
    l.contentTopY=l.propertiesY;
    l.selectedSummaryY=l.propertiesY+42.0f*s;
    l.placedListY=l.outlinerY+36.0f*s;
    l.placedPageSize=l.compact?4u:6u;

    l.editLabelY=l.selectedSummaryY+70.0f*s;l.editRowY=l.editLabelY+22.0f*s;
    l.focusLabelY=l.editRowY+38.0f*s;l.focusRowY=l.focusLabelY+22.0f*s;
    l.moveLabelY=l.focusRowY+38.0f*s;l.moveRowY=l.moveLabelY+22.0f*s;l.moveRow2Y=l.moveRowY+38.0f*s;
    l.rotateLabelY=l.moveRow2Y+40.0f*s;l.rotateRowY=l.rotateLabelY+22.0f*s;l.yawRowY=l.rotateRowY+38.0f*s;l.rollRowY=l.yawRowY+38.0f*s;l.flipRowY=l.rollRowY+38.0f*s;
    l.blueprintLabelY=l.selectedSummaryY+70.0f*s;l.classRowY=l.blueprintLabelY+22.0f*s;l.sizeModeRowY=l.classRowY+38.0f*s;l.generateRowY=l.sizeModeRowY+38.0f*s;l.saveRowY=l.generateRowY+38.0f*s;
    l.liveryLabelY=l.selectedSummaryY+70.0f*s;l.liveryRowY=l.liveryLabelY+22.0f*s;l.liverySecondaryRowY=l.liveryRowY+38.0f*s;l.paintSecondaryRowY=l.liverySecondaryRowY+38.0f*s;l.paintTrimRowY=l.paintSecondaryRowY+38.0f*s;l.decalRowY=l.paintTrimRowY+38.0f*s;
    l.statusY=dcc.statusBar.y;
    l.validationY=l.statusY-72.0f*s;
    return l;
}

std::vector<ShipyardBuilderControl> ShipyardBuilderSystem::BuildControls(const ShipyardBuilderRuntimeModel& model,int w,int h){
    std::vector<ShipyardBuilderControl> out;
    const auto l=Layout(w,h);if(!l.valid)return out;
    const float s=l.uiScale,gap=5.0f*s;
    auto add=[&](ShipyardBuilderCommand c,int value,float x,float y,float cw,float ch,std::string label,bool active=false,bool enabled=true){out.push_back({c,value,x,y,cw,ch,std::move(label),active,enabled});};

    const bool maximized=model.dcc.maximizeViewport;
    const bool showAssetBrowser=model.dcc.showAssetBrowser&&!maximized;
    const bool showToolRail=model.dcc.showToolRail&&!maximized;
    const bool showSidebar=model.dcc.showSidebar&&!maximized;

    // Blender-like global workspace strip. Four daily workspaces remain visible;
    // DEV exposes advanced authoring workspaces without crowding the viewport.
    float bx=42.0f*s;const float tabW=88.0f*s,tabGap=2.0f*s;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active,bool enabled=true){add(c,0,bx,l.workspaceBarY,tabW,l.workspaceBarHeight,label,active,enabled);bx+=tabW+tabGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"LAYOUT",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Build);
    tab(ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Systems);
    tab(ShipyardBuilderCommand::WorkspaceAppearance,"PAINT",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Appearance);
    tab(ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Interior,model.capabilities.interior);
    tab(ShipyardBuilderCommand::WorkspaceDevWorld,"TEST",model.testWorkspaceActive,true);out.back().value=-1268;
    add(ShipyardBuilderCommand::WorkspaceAuthoring,-1268,std::max(bx,static_cast<float>(w)-70.0f*s),l.workspaceBarY,62.0f*s,l.workspaceBarHeight,"DEV",model.developerWorkspacesVisible||IsAdvancedWorkspace(model.workspaceMode),true);
    if(model.developerWorkspacesVisible){
        float dx=8.0f*s;const float y=l.workspaceBarY+l.workspaceBarHeight+3.0f*s;const float dw=84.0f*s;
        const std::vector<std::tuple<ShipyardBuilderCommand,const char*,bool,bool>> dev={
            {ShipyardBuilderCommand::WorkspaceModel,"MODEL",model.workspaceMode==ShipyardWorkspaceMode::Model,model.capabilities.model},
            {ShipyardBuilderCommand::InspectorSockets,"SOCKETS",model.inspectorTab==ShipyardInspectorTab::Sockets,model.capabilities.sockets},
            {ShipyardBuilderCommand::WorkspacePcg,"PCG",model.workspaceMode==ShipyardWorkspaceMode::Pcg,model.capabilities.pcgStudio},
            {ShipyardBuilderCommand::WorkspaceWorld,"WORLD",model.workspaceMode==ShipyardWorkspaceMode::World,model.capabilities.world},
            {ShipyardBuilderCommand::WorkspaceCharacter,"CHAR",model.workspaceMode==ShipyardWorkspaceMode::Character,model.capabilities.character},
            {ShipyardBuilderCommand::WorkspaceDevWorld,"DEV WORLD",model.workspaceMode==ShipyardWorkspaceMode::DevWorld,model.capabilities.devWorld},
            {ShipyardBuilderCommand::WorkspaceProjectTools,"PROJECT",model.workspaceMode==ShipyardWorkspaceMode::ProjectTools,true},
            {ShipyardBuilderCommand::WorkspaceAuthoring,"AUTHOR",model.workspaceMode==ShipyardWorkspaceMode::Authoring,model.capabilities.rawAuthoring}};
        for(const auto& d:dev){add(std::get<0>(d),0,dx,y,dw,24.0f*s,std::get<1>(d),std::get<2>(d),std::get<3>(d));dx+=dw+3.0f*s;}
    }

    if(maximized){
        add(ShipyardBuilderCommand::DccToggleMaximizeViewport,0,w-92.0f*s,l.top+4.0f*s,82.0f*s,26.0f*s,"RESTORE",true,true);
    }else{
        // 3D-view header and viewport display controls.
        const float vx=l.viewportLeft+8.0f*s;const float vy=l.viewportTop-25.0f*s;
        add(ShipyardBuilderCommand::DccToggleGrid,0,vx,vy,50*s,25*s,"GRID",model.dcc.showGrid,true);
        add(ShipyardBuilderCommand::DccToggleGizmos,0,vx+54*s,vy,58*s,25*s,"GIZMO",model.dcc.showGizmos,true);
        add(ShipyardBuilderCommand::DccCycleShading,0,vx+116*s,vy,86*s,25*s,ShipyardDccUiSystem::ShadingName(model.dcc.shading),true,true);
        add(ShipyardBuilderCommand::DccToggleStatsOverlay,0,vx+206*s,vy,56*s,25*s,"STATS",model.dcc.showStatsOverlay,true);
        add(ShipyardBuilderCommand::DccToggleMaximizeViewport,0,vx+266*s,vy,48*s,25*s,"MAX",false,true);
    }

    if(showAssetBrowser){
        const float ax=l.assetShelfX,ay=l.assetShelfY,aw=l.assetShelfWidth,ah=l.assetShelfHeight;
        const float headerH=30.0f*s;
        // Bottom Asset Browser shelf: the central viewport keeps its width while
        // all ship/module assets remain one click away, matching a modern DCC.
        add(ShipyardBuilderCommand::DccPreviousAssetPreset,0,ax+96*s,ay+3*s,24*s,24*s,"<",false,true);
        add(ShipyardBuilderCommand::DccNextAssetPreset,0,ax+123*s,ay+3*s,152*s,24*s,ShipyardDccUiSystem::AssetPresetName(model.dcc.assetPreset),true,true);
        add(ShipyardBuilderCommand::DccCycleAssetDensity,0,ax+279*s,ay+3*s,74*s,24*s,ShipyardDccUiSystem::AssetDensityName(model.dcc.assetBrowser.density),false,true);
        add(ShipyardBuilderCommand::DccToggleFavoriteSelected,0,ax+aw-322*s,ay+3*s,64*s,24*s,"FAV",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::DccClearAssetFilters,0,ax+aw-254*s,ay+3*s,64*s,24*s,"CLEAR",false,true);
        add(ShipyardBuilderCommand::AddModule,0,ax+aw-186*s,ay+3*s,82*s,24*s,"PLACE",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::Validate,0,ax+aw-100*s,ay+3*s,92*s,24*s,"VALIDATE",false,true);

        const float categoryY=ay+headerH+2.0f*s;
        const float categoryW=(aw-14.0f*s-7*3.0f*s)/8.0f;
        for(int ci=0;ci<8;++ci){
            const auto cls=static_cast<ShipyardModuleClass>(ci);std::size_t classCount=0;
            for(const auto& rec:model.catalog)if(rec.moduleClass==cls)++classCount;
            add(ShipyardBuilderCommand::SelectClass,ci,ax+7*s+ci*(categoryW+3*s),categoryY,categoryW,25.0f*s,
                std::string(ShipyardModuleSystem::ClassName(cls))+" "+std::to_string(classCount),static_cast<int>(model.selectedClass)==ci,true);
        }

        const auto filtered=ShipyardBuilderSystem::VisibleCatalogIndices(model);
        float densityScale=1.0f;
        if(model.dcc.assetBrowser.density==ShipyardAssetBrowserDensity::Compact)densityScale=.82f;
        else if(model.dcc.assetBrowser.density==ShipyardAssetBrowserDensity::Large)densityScale=1.18f;
        densityScale*=std::clamp(model.dcc.assetBrowser.thumbnailScale,.70f,1.35f);
        const float cardY=categoryY+29.0f*s;
        const float cardH=std::max(54.0f*s,ah-(cardY-ay)-7.0f*s);
        const float desiredW=std::clamp(205.0f*s*densityScale,156.0f*s,270.0f*s);
        const std::size_t pageSize=std::clamp<std::size_t>(static_cast<std::size_t>((aw-14.0f*s)/(desiredW+gap)),4u,8u);
        const float cardW=(aw-14.0f*s-gap*static_cast<float>(pageSize-1))/static_cast<float>(pageSize);
        const std::size_t selected=filtered.empty()?0:std::min(model.selectedFilteredModule,filtered.size()-1);
        const std::size_t maxStart=filtered.size()>pageSize?filtered.size()-pageSize:0;
        const std::size_t start=filtered.empty()?0:std::min(model.catalogScrollStart,maxStart);
        for(std::size_t i=0;i<pageSize&&start+i<filtered.size();++i){
            const auto fi=start+i;const auto& rec=model.catalog[filtered[fi]];
            add(ShipyardBuilderCommand::SelectModule,static_cast<int>(fi),ax+7*s+i*(cardW+gap),cardY,cardW,cardH,FriendlyModuleLabel(rec),fi==selected,true);
        }
        add(ShipyardBuilderCommand::PreviousModule,0,ax+4*s,ay+3*s,24*s,24*s,"<");
        add(ShipyardBuilderCommand::NextModule,0,ax+31*s,ay+3*s,24*s,24*s,">");
    }

    if(showToolRail){
        const float tx=l.toolRailX,tw=l.toolRailWidth,th=36.0f*s;
        add(ShipyardBuilderCommand::ToolSelect,0,tx,l.toolRailY,tw,th,"Q",model.transformTool==ShipyardTransformTool::Select,true);
        add(ShipyardBuilderCommand::ToolMove,0,tx,l.toolRailY+(th+gap),tw,th,"G",model.transformTool==ShipyardTransformTool::Move,HasPlaced(model));
        add(ShipyardBuilderCommand::ToolRotate,0,tx,l.toolRailY+2*(th+gap),tw,th,"R",model.transformTool==ShipyardTransformTool::Rotate,HasPlaced(model));
        add(ShipyardBuilderCommand::ToolScale,0,tx,l.toolRailY+3*(th+gap),tw,th,"S",model.transformTool==ShipyardTransformTool::Scale,HasPlaced(model));
        add(ShipyardBuilderCommand::ToggleTransformSnap,0,tx,l.toolRailY+4*(th+gap),tw,th,"SNAP",model.transformSnap,HasPlaced(model));
        add(ShipyardBuilderCommand::FrameSelected,0,tx,l.toolRailY+5*(th+gap),tw,th,"F",false,HasPlaced(model));
    }

    if(showSidebar){
        const float rx=l.outlinerX+7*s,rw=l.outlinerWidth-14*s;
        if(model.dcc.showOutliner){
            add(ShipyardBuilderCommand::DccCycleOutlinerMode,0,rx,l.outlinerY+5*s,rw,24*s,ShipyardDccUiSystem::OutlinerModeName(model.dcc.outlinerMode),true,true);
            const auto rows=ShipyardDccUiSystem::BuildOutlinerRows(model.catalog,model.recipe,model.selectedPlacedModule,model.dcc.outlinerMode);
            const std::size_t page=l.compact?4u:5u;const std::size_t start=rows.empty()?0:std::min(model.placedScrollStart,rows.size()>page?rows.size()-page:0u);
            for(std::size_t i=0;i<page&&start+i<rows.size();++i){const auto& item=rows[start+i];std::string displayLabel=item.label;if(item.moduleIndex<model.recipe.modules.size()){const auto& placedId=model.recipe.modules[item.moduleIndex].moduleId;const auto found=std::find_if(model.catalog.begin(),model.catalog.end(),[&](const auto& record){return record.source.moduleId==placedId;});if(found!=model.catalog.end())displayLabel=FriendlyModuleLabel(*found);}std::string label=model.dcc.outlinerMode==ShipyardDccOutlinerMode::Hierarchy?std::string(item.depth*2,' '):std::string{};if(model.dcc.outlinerMode!=ShipyardDccOutlinerMode::Hierarchy)label=item.group+" | ";label+=(item.attached?"|_ ":"o  ")+displayLabel;add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(item.moduleIndex),rx,l.placedListY+i*(l.rowHeight+gap),rw,l.rowHeight,label,item.selected,true);}
        }
        if(model.dcc.showProperties){
            const float py=l.propertiesY+34.0f*s;const float rail=30*s;
            add(ShipyardBuilderCommand::InspectorTransform,0,rx,py,rail,30*s,"T",model.inspectorTab==ShipyardInspectorTab::Transform,true);
            add(ShipyardBuilderCommand::InspectorAssembly,0,rx,py+34*s,rail,30*s,"SYS",model.inspectorTab==ShipyardInspectorTab::Assembly,true);
            add(ShipyardBuilderCommand::InspectorSockets,0,rx,py+68*s,rail,30*s,"SCK",model.inspectorTab==ShipyardInspectorTab::Sockets,model.capabilities.sockets);
            add(ShipyardBuilderCommand::InspectorAuthoring,0,rx,py+102*s,rail,30*s,"DNA",model.inspectorTab==ShipyardInspectorTab::Authoring,true);
            add(ShipyardBuilderCommand::InspectorAppearance,0,rx,py+136*s,rail,30*s,"MAT",model.inspectorTab==ShipyardInspectorTab::Appearance,true);
            const float px=rx+rail+5*s,pw=rw-rail-5*s;
            add(ShipyardBuilderCommand::FrameShip,0,px,py,pw,30*s,"FRAME SHIP",false,HasPlaced(model));
            add(ShipyardBuilderCommand::ToggleLiveSymmetry,0,px,py+34*s,pw,30*s,model.symmetryFrame.live?"SYMMETRY ON":"SYMMETRY OFF",model.symmetryFrame.live,true);
            add(ShipyardBuilderCommand::DccToggleSocketsOverlay,0,px,py+68*s,pw,30*s,"SOCKET OVERLAY",model.dcc.showSocketOverlay,true);
            add(ShipyardBuilderCommand::DccToggleShieldPreview,0,px,py+102*s,pw,30*s,"SHIELD PREVIEW",model.dcc.showShieldPreview,true);
            add(ShipyardBuilderCommand::SaveBlueprint,0,px,py+136*s,pw,30*s,model.validation.valid?"SAVE BLUEPRINT":"SAVE DRAFT",false,HasPlaced(model));
        }
    }

    // Keep the complete historical command surface discoverable off-screen so
    // old automation and project-authored workflows remain compatible.
    const auto compatibility=LegacyBuildControls(model,w,h);constexpr float kCompatibilityStride=100000.0f;std::size_t compatibilityIndex=0;
    for(auto c:compatibility){
        switch(c.command){
        case ShipyardBuilderCommand::WorkspaceBuild:case ShipyardBuilderCommand::WorkspaceInterior:case ShipyardBuilderCommand::WorkspaceAppearance:case ShipyardBuilderCommand::WorkspaceSystems:case ShipyardBuilderCommand::WorkspaceModel:case ShipyardBuilderCommand::WorkspaceCharacter:case ShipyardBuilderCommand::WorkspacePcg:case ShipyardBuilderCommand::WorkspaceWorld:case ShipyardBuilderCommand::WorkspaceDevWorld:case ShipyardBuilderCommand::WorkspaceProjectTools:case ShipyardBuilderCommand::WorkspaceAuthoring:continue;
        default:break;
        }
        const bool visibleDuplicate=std::any_of(out.begin(),out.end(),[&](const auto& current){return current.command==c.command&&current.value==c.value&&current.width>0.0f&&current.height>0.0f;});if(visibleDuplicate)continue;const float offset=kCompatibilityStride*static_cast<float>(compatibilityIndex++);c.x=-kCompatibilityStride-offset;c.y=-kCompatibilityStride-offset;out.push_back(std::move(c));}
    return out;
}

ShipyardBuilderControl ShipyardBuilderSystem::HitTest(const ShipyardBuilderRuntimeModel& model,int w,int h,float x,float y){
    const auto controls=BuildControls(model,w,h);
    for(auto it=controls.rbegin();it!=controls.rend();++it)if(it->Contains(x,y))return *it;
    return {};
}

} // namespace subspace
