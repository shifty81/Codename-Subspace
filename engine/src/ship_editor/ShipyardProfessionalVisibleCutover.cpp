#include "ship_editor/ShipyardBuilderSystem.h"
#include "content/ShipyardPartTaxonomySystem.h"

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
    if(w<1120||h<740)return l;
    l.valid=true;
    l.uiScale=std::clamp(std::min(static_cast<float>(w)/1920.0f,static_cast<float>(h)/1080.0f),1.0f,1.60f);
    const float s=l.uiScale;
    l.compact=h<static_cast<int>(860.0f*s);

    l.workspaceBarY=29.0f*s;
    l.workspaceBarHeight=28.0f*s;
    l.left=8.0f*s;
    l.top=88.0f*s;
    l.leftWidth=std::clamp(static_cast<float>(w)*.17f,280.0f*s,340.0f*s);
    l.rightWidth=std::clamp(static_cast<float>(w)*.22f,340.0f*s,430.0f*s);
    l.right=static_cast<float>(w)-l.rightWidth-8.0f*s;
    l.toolRailWidth=42.0f*s;
    l.toolRailX=l.left+l.leftWidth+8.0f*s;
    l.toolRailY=l.top+10.0f*s;
    l.rowHeight=(l.compact?27.0f:30.0f)*s;
    l.rowGap=5.0f*s;

    l.libraryListY=l.top+70.0f*s;
    l.moduleCardsY=l.libraryListY+2.0f*(l.rowHeight+l.rowGap)+34.0f*s;
    l.moduleCardHeight=(l.compact?54.0f:62.0f)*s;
    l.leftActionsY=l.moduleCardsY+5.0f*(l.moduleCardHeight+l.rowGap)+10.0f*s;
    l.leftInfoY=l.leftActionsY+76.0f*s;

    l.tabRowY=l.workspaceBarY;
    l.tabHeight=l.workspaceBarHeight;
    l.contentTopY=l.top+(l.compact?132.0f:164.0f)*s;
    l.selectedSummaryY=l.contentTopY;
    l.placedListY=l.contentTopY+42.0f*s;
    l.placedPageSize=l.compact?4u:5u;

    const float outlinerBottom=l.placedListY+static_cast<float>(l.placedPageSize)*(l.rowHeight+l.rowGap)+14.0f*s;
    l.editLabelY=outlinerBottom+18.0f*s;l.editRowY=l.editLabelY+20.0f*s;
    l.focusLabelY=l.editRowY+38.0f*s;l.focusRowY=l.focusLabelY+20.0f*s;
    l.moveLabelY=l.focusRowY+38.0f*s;l.moveRowY=l.moveLabelY+20.0f*s;l.moveRow2Y=l.moveRowY+38.0f*s;
    l.rotateLabelY=l.moveRow2Y+40.0f*s;l.rotateRowY=l.rotateLabelY+20.0f*s;l.yawRowY=l.rotateRowY+38.0f*s;l.rollRowY=l.yawRowY+38.0f*s;l.flipRowY=l.rollRowY+38.0f*s;
    l.blueprintLabelY=outlinerBottom+20.0f*s;l.classRowY=l.blueprintLabelY+20.0f*s;l.sizeModeRowY=l.classRowY+40.0f*s;l.generateRowY=l.sizeModeRowY+40.0f*s;l.saveRowY=l.generateRowY+40.0f*s;
    l.liveryLabelY=outlinerBottom+20.0f*s;l.liveryRowY=l.liveryLabelY+22.0f*s;l.liverySecondaryRowY=l.liveryRowY+40.0f*s;l.paintSecondaryRowY=l.liverySecondaryRowY+40.0f*s;l.paintTrimRowY=l.paintSecondaryRowY+40.0f*s;l.decalRowY=l.paintTrimRowY+40.0f*s;
    l.statusY=static_cast<float>(h)-32.0f*s;
    l.validationY=l.statusY-102.0f*s;
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
    float bx=8.0f*s;const float tabW=102.0f*s,tabGap=3.0f*s;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active,bool enabled=true){add(c,0,bx,l.workspaceBarY,tabW,l.workspaceBarHeight,label,active,enabled);bx+=tabW+tabGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"ASSEMBLY",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Build);
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
        const float vx=l.toolRailX+l.toolRailWidth+8.0f*s;const float vy=l.top-31.0f*s;
        add(ShipyardBuilderCommand::DccToggleGrid,0,vx,vy,50*s,25*s,"GRID",model.dcc.showGrid,true);
        add(ShipyardBuilderCommand::DccToggleGizmos,0,vx+54*s,vy,58*s,25*s,"GIZMO",model.dcc.showGizmos,true);
        add(ShipyardBuilderCommand::DccCycleShading,0,vx+116*s,vy,86*s,25*s,ShipyardDccUiSystem::ShadingName(model.dcc.shading),true,true);
        add(ShipyardBuilderCommand::DccToggleStatsOverlay,0,vx+206*s,vy,56*s,25*s,"STATS",model.dcc.showStatsOverlay,true);
        add(ShipyardBuilderCommand::DccToggleMaximizeViewport,0,vx+266*s,vy,48*s,25*s,"MAX",false,true);
    }

    if(showAssetBrowser){
        const float left=l.left,libraryW=l.leftWidth,rowH=l.rowHeight;
        add(ShipyardBuilderCommand::DccPreviousAssetPreset,0,left+7*s,l.top+7*s,28*s,25*s,"<",false,true);
        add(ShipyardBuilderCommand::DccNextAssetPreset,0,left+39*s,l.top+7*s,libraryW-105*s,25*s,ShipyardDccUiSystem::AssetPresetName(model.dcc.assetPreset),true,true);
        add(ShipyardBuilderCommand::DccCycleAssetDensity,0,left+libraryW-62*s,l.top+7*s,55*s,25*s,ShipyardDccUiSystem::AssetDensityName(model.dcc.assetBrowser.density),false,true);
        const float categoryW=(libraryW-14.0f*s-3*3.0f*s)/4.0f;
        for(int ci=0;ci<8;++ci){const auto cls=static_cast<ShipyardModuleClass>(ci);const int col=ci%4,row=ci/4;std::size_t classCount=0;for(const auto& rec:model.catalog)if(rec.moduleClass==cls)++classCount;add(ShipyardBuilderCommand::SelectClass,ci,left+7*s+col*(categoryW+3*s),l.libraryListY+row*(rowH+gap),categoryW,rowH,std::string(ShipyardModuleSystem::ClassName(cls))+" ("+std::to_string(classCount)+")",static_cast<int>(model.selectedClass)==ci,true);}
        const auto filtered=ShipyardBuilderSystem::VisibleCatalogIndices(model);
        float densityScale=1.0f;if(model.dcc.assetBrowser.density==ShipyardAssetBrowserDensity::Compact)densityScale=.78f;else if(model.dcc.assetBrowser.density==ShipyardAssetBrowserDensity::Large)densityScale=1.18f;densityScale*=std::clamp(model.dcc.assetBrowser.thumbnailScale,.70f,1.35f);
        const float assetCardHeight=std::clamp(l.moduleCardHeight*densityScale,42.0f*s,90.0f*s);
        const float cardSpace=std::max(150.0f*s,l.leftActionsY-l.moduleCardsY-10.0f*s);
        const std::size_t pageSize=std::clamp<std::size_t>(static_cast<std::size_t>(cardSpace/(assetCardHeight+gap)),3u,8u);
        const std::size_t selected=filtered.empty()?0:std::min(model.selectedFilteredModule,filtered.size()-1);const std::size_t maxStart=filtered.size()>pageSize?filtered.size()-pageSize:0;const std::size_t start=filtered.empty()?0:std::min(model.catalogScrollStart,maxStart);
        for(std::size_t i=0;i<pageSize&&start+i<filtered.size();++i){const auto fi=start+i;const auto& rec=model.catalog[filtered[fi]];add(ShipyardBuilderCommand::SelectModule,static_cast<int>(fi),left+7*s,l.moduleCardsY+i*(assetCardHeight+gap),libraryW-14*s,assetCardHeight,FriendlyModuleLabel(rec),fi==selected,true);}
        const float actionY=l.leftActionsY,inner=libraryW-14*s;
        add(ShipyardBuilderCommand::PreviousModule,0,left+7*s,actionY,32*s,28*s,"<");add(ShipyardBuilderCommand::NextModule,0,left+43*s,actionY,32*s,28*s,">");add(ShipyardBuilderCommand::AddModule,0,left+79*s,actionY,72*s,28*s,"PLACE",false,!filtered.empty());add(ShipyardBuilderCommand::ReplaceModule,0,left+155*s,actionY,inner-148*s,28*s,"REPLACE",false,!filtered.empty()&&HasPlaced(model));
        add(ShipyardBuilderCommand::DccToggleFavoriteSelected,0,left+7*s,actionY+32*s,68*s,27*s,"FAVORITE",false,!filtered.empty());add(ShipyardBuilderCommand::DccClearAssetFilters,0,left+79*s,actionY+32*s,72*s,27*s,"CLEAR",false,true);add(ShipyardBuilderCommand::Validate,0,left+155*s,actionY+32*s,inner-148*s,27*s,"VALIDATE",false,true);
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
        const float rx=l.right+7*s,rw=l.rightWidth-14*s;
        if(model.dcc.showOutliner){
            add(ShipyardBuilderCommand::DccCycleOutlinerMode,0,rx,l.top+7*s,rw,25*s,ShipyardDccUiSystem::OutlinerModeName(model.dcc.outlinerMode),true,true);
            const auto rows=ShipyardDccUiSystem::BuildOutlinerRows(model.catalog,model.recipe,model.selectedPlacedModule,model.dcc.outlinerMode);
            const std::size_t page=l.compact?4u:5u;const std::size_t start=rows.empty()?0:std::min(model.placedScrollStart,rows.size()>page?rows.size()-page:0u);
            for(std::size_t i=0;i<page&&start+i<rows.size();++i){const auto& item=rows[start+i];std::string displayLabel=item.label;if(item.moduleIndex<model.recipe.modules.size()){const auto& placedId=model.recipe.modules[item.moduleIndex].moduleId;const auto found=std::find_if(model.catalog.begin(),model.catalog.end(),[&](const auto& record){return record.source.moduleId==placedId;});if(found!=model.catalog.end())displayLabel=FriendlyModuleLabel(*found);}std::string label=model.dcc.outlinerMode==ShipyardDccOutlinerMode::Hierarchy?std::string(item.depth*2,' '):std::string{};if(model.dcc.outlinerMode!=ShipyardDccOutlinerMode::Hierarchy)label=item.group+" | ";label+=(item.attached?"|_ ":"o  ")+displayLabel;add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(item.moduleIndex),rx,l.placedListY+i*(l.rowHeight+gap),rw,l.rowHeight,label,item.selected,true);}
        }
        if(model.dcc.showProperties){
            const float py=l.editRowY;const float rail=32*s;
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
