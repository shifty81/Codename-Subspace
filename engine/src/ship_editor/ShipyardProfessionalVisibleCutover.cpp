#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardCatalogViewport.h"
#include "content/ShipyardPartTaxonomySystem.h"
#include "editor/EditorDccShellLayoutSystem.h"
#include "editor/EditorForgeGuiStyleSystem.h"
#include "ship_editor/ShipyardPanelCompositorSystem.h"

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
const char* DockPanelIdFromValue(int value){
    switch(value){
    case 0:return "asset_browser";
    case 1:return "outliner";
    case 2:return "properties";
    case 3:return "history";
    case 4:return "validation";
    case 5:return "console";
    case 6:return "interior_program";
    case 7:return "systems";
    case 8:return "tool_rail";
    default:return "properties";
    }
}

bool LegacyDccPanelShown(const ShipyardBuilderRuntimeModel& model,const char* id){
    const std::string key=id?id:"";
    if(key=="asset_browser")return model.dcc.showAssetBrowser;
    if(key=="tool_rail")return model.dcc.showToolRail;
    if(key=="outliner")return model.dcc.showSidebar&&model.dcc.showOutliner;
    if(key=="properties")return model.dcc.showSidebar&&model.dcc.showProperties;
    return false;
}
bool DockPanelShown(const ShipyardBuilderRuntimeModel& model,const char* id){
    const auto* panel=SubspaceDockSystem::FindPanel(model.dockWorkspace,id);
    return panel?panel->visible:LegacyDccPanelShown(model,id);
}
bool DockPanelVisible(const ShipyardBuilderRuntimeModel& model,const char* id){
    const auto* panel=SubspaceDockSystem::FindPanel(model.dockWorkspace,id);
    return panel?(panel->visible&&!panel->collapsed):LegacyDccPanelShown(model,id);
}
bool DockPanelCollapsed(const ShipyardBuilderRuntimeModel& model,const char* id){
    const auto* panel=SubspaceDockSystem::FindPanel(model.dockWorkspace,id);
    return panel&&panel->visible&&panel->collapsed;
}
bool DockPanelPinned(const ShipyardBuilderRuntimeModel& model,const char* id){
    const auto* panel=SubspaceDockSystem::FindPanel(model.dockWorkspace,id);
    return panel&&panel->pinned;
}

} // namespace

bool ShipyardBuilderSystem::Activate(ShipyardBuilderCommand command,int value){
    if(model_.assetSearchFocused&&command!=ShipyardBuilderCommand::DccAssetFocusSearch&&
       command!=ShipyardBuilderCommand::DccAssetClearSearch)
        model_.assetSearchFocused=false;
    if(command!=ShipyardBuilderCommand::MenuFile&&command!=ShipyardBuilderCommand::MenuEdit&&
       command!=ShipyardBuilderCommand::MenuView&&command!=ShipyardBuilderCommand::MenuHelp)
        model_.openMenu=-1;
    switch(command){
    case ShipyardBuilderCommand::MenuFile:
    case ShipyardBuilderCommand::MenuEdit:
    case ShipyardBuilderCommand::MenuView:
    case ShipyardBuilderCommand::MenuHelp:{
        const int menu=static_cast<int>(command)-static_cast<int>(ShipyardBuilderCommand::MenuFile);
        model_.openMenu=model_.openMenu==menu?-1:menu;
        return true;
    }
    case ShipyardBuilderCommand::DccCycleStudioView:{
        const auto requested=ShipyardStudioViewSystem::Next(model_.studioViewMode);
        const bool placing=model_.workspaceMode==ShipyardWorkspaceMode::Build&&
            (model_.dragPreview.active||model_.dragPreview.staged);
        // Do not turn off the only visible hull/ghost during an active placement.
        model_.studioViewMode=placing?ShipyardStudioViewSystem::ForPlacement(requested):requested;
        model_.status=placing&&requested==ShipyardStudioViewMode::InteriorOnly
            ?"EXTERIOR: hull placement active; interior-only would hide the drag preview"
            :std::string("Viewport: ")+ShipyardStudioViewSystem::Name(model_.studioViewMode)+
                " / interior shell uses generated cavities; invalid cavities fail closed";
        model_.openMenu=-1;
        return true;
    }
    case ShipyardBuilderCommand::DccRevealAssetBrowser:{
        auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,"asset_browser");
        if(!panel)return false;
        // The global Assets recovery control must work even if the user closed
        // the panel, collapsed it, selected a different bottom tab, floated it,
        // or maximized the viewport. Never depend on the old visibility flag.
        model_.dcc.maximizeViewport=false;
        panel->collapsed=false;
        panel->autoHide=false;
        panel->hoverReveal=true;
        if(!SubspaceDockSystem::OpenPanel(model_.dockWorkspace,panel->id))return false;
        SubspaceDockSystem::RaiseFloatingPanel(model_.dockWorkspace,panel->id);
        model_.dcc.showAssetBrowser=true;
        model_.status="Asset Browser restored (RESET UI restores all default panels)";
        return true;
    }
    case ShipyardBuilderCommand::DccToggleAssetBrowser:{auto* p=SubspaceDockSystem::FindPanel(model_.dockWorkspace,"asset_browser");if(!p)return false;const bool show=!p->visible;const bool ok=show?SubspaceDockSystem::OpenPanel(model_.dockWorkspace,p->id):SubspaceDockSystem::ClosePanel(model_.dockWorkspace,p->id);model_.dcc.showAssetBrowser=show;if(ok)model_.status=show?"Asset Browser shown":"Asset Browser hidden";return ok;}
    case ShipyardBuilderCommand::DccToggleToolRail:{auto* p=SubspaceDockSystem::FindPanel(model_.dockWorkspace,"tool_rail");if(!p)return false;const bool show=!p->visible;const bool ok=show?SubspaceDockSystem::OpenPanel(model_.dockWorkspace,p->id):SubspaceDockSystem::ClosePanel(model_.dockWorkspace,p->id);model_.dcc.showToolRail=show;if(ok)model_.status=show?"Tool rail shown":"Tool rail hidden";return ok;}
    case ShipyardBuilderCommand::DccToggleSidebar:{const bool show=!(DockPanelVisible(model_,"outliner")||DockPanelVisible(model_,"properties"));for(const char* id:{"outliner","properties"}){if(show)SubspaceDockSystem::OpenPanel(model_.dockWorkspace,id);else SubspaceDockSystem::ClosePanel(model_.dockWorkspace,id);}model_.dcc.showSidebar=show;model_.dcc.showOutliner=show;model_.dcc.showProperties=show;model_.status=show?"Context panels shown":"Context panels hidden";return true;}
    case ShipyardBuilderCommand::DccToggleOutliner:{auto* p=SubspaceDockSystem::FindPanel(model_.dockWorkspace,"outliner");if(!p)return false;const bool show=!p->visible;const bool ok=show?SubspaceDockSystem::OpenPanel(model_.dockWorkspace,p->id):SubspaceDockSystem::ClosePanel(model_.dockWorkspace,p->id);model_.dcc.showOutliner=show;model_.dcc.showSidebar=DockPanelVisible(model_,"outliner")||DockPanelVisible(model_,"properties");if(ok)model_.status=show?"Outliner shown":"Outliner hidden";return ok;}
    case ShipyardBuilderCommand::DccToggleProperties:{auto* p=SubspaceDockSystem::FindPanel(model_.dockWorkspace,"properties");if(!p)return false;const bool show=!p->visible;const bool ok=show?SubspaceDockSystem::OpenPanel(model_.dockWorkspace,p->id):SubspaceDockSystem::ClosePanel(model_.dockWorkspace,p->id);model_.dcc.showProperties=show;model_.dcc.showSidebar=DockPanelVisible(model_,"outliner")||DockPanelVisible(model_,"properties");if(ok)model_.status=show?"Properties shown":"Properties hidden";return ok;}
    case ShipyardBuilderCommand::DccToggleStatusBar:model_.dcc.showStatusBar=!model_.dcc.showStatusBar;return true;
    case ShipyardBuilderCommand::DccToggleMaximizeViewport:model_.dcc.maximizeViewport=!model_.dcc.maximizeViewport;model_.status=model_.dcc.maximizeViewport?"3D View maximized - Ctrl+Space to restore":"3D View restored";return true;
    case ShipyardBuilderCommand::DccResetLayout:
        ShipyardDccUiSystem::ResetLayout(model_.dcc);
        model_.dockWorkspace=ShipyardWorkspaceSystem::BuildDefaultDockWorkspace();
        model_.status="Shipyard workspace layout reset";
        return true;
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
    case ShipyardBuilderCommand::DccAssetFocusSearch:
        model_.assetSearchFocused=!model_.assetSearchFocused;
        model_.status=model_.assetSearchFocused?"Search assets: type text, Backspace deletes, Enter closes":"Asset search closed";
        return true;
    case ShipyardBuilderCommand::DccAssetClearSearch:
        model_.dcc.assetBrowser.search.clear();model_.selectedFilteredModule=0;model_.catalogScrollStart=0;
        model_.status="Asset search cleared";return true;
    case ShipyardBuilderCommand::DccAssetPrevious:
    case ShipyardBuilderCommand::DccAssetNext:{
        const auto count=FilteredCatalogIndices().size();
        if(!count){model_.catalogScrollStart=0;return true;}
        if(command==ShipyardBuilderCommand::DccAssetPrevious){if(model_.catalogScrollStart) --model_.catalogScrollStart;}
        else model_.catalogScrollStart=std::min(count-1,model_.catalogScrollStart+1);
        model_.selectedFilteredModule=model_.catalogScrollStart;
        model_.status="Asset Browser: "+std::to_string(model_.catalogScrollStart+1)+" / "+std::to_string(count);
        return true;}
    case ShipyardBuilderCommand::DccOutlinerPrevious:
    case ShipyardBuilderCommand::DccOutlinerNext:{
        const auto count=model_.workspaceMode==ShipyardWorkspaceMode::Model?model_.modeling.recipe.primitives.size():model_.recipe.modules.size();
        if(!count){model_.placedScrollStart=0;return true;}
        if(command==ShipyardBuilderCommand::DccOutlinerPrevious){if(model_.placedScrollStart)--model_.placedScrollStart;}
        else model_.placedScrollStart=std::min(count-1,model_.placedScrollStart+1);
        model_.status="Outliner: "+std::to_string(model_.placedScrollStart+1)+" / "+std::to_string(count);
        return true;}
    case ShipyardBuilderCommand::DccToggleFavoriteSelected:{const auto* selected=SelectedCatalogModule();if(!selected)return false;const bool favorite=ShipyardAssetBrowserSystem::ToggleFavorite(model_.dcc.assetBrowser,selected->source.moduleId);model_.status=favorite?"Module added to favorites":"Module removed from favorites";return true;}
    case ShipyardBuilderCommand::DccClearAssetFilters:ShipyardDccUiSystem::ApplyAssetPreset(model_.dcc,0);model_.selectedFilteredModule=0;model_.catalogScrollStart=0;model_.status="Asset Browser filters cleared";return true;
    case ShipyardBuilderCommand::DccToggleCommandPalette:model_.dcc.commandPaletteOpen=!model_.dcc.commandPaletteOpen;model_.status=model_.dcc.commandPaletteOpen?"Command Search open":"Command Search closed";return true;
    case ShipyardBuilderCommand::DccWorkspacePrevious:
    case ShipyardBuilderCommand::DccWorkspaceNext:{
        const auto order=ShipyardDccUiSystem::WorkspaceCycle();auto it=std::find(order.begin(),order.end(),model_.testWorkspaceActive?ShipyardWorkspaceMode::Test:model_.workspaceMode);
        std::size_t index=it==order.end()?0:static_cast<std::size_t>(std::distance(order.begin(),it));
        if(command==ShipyardBuilderCommand::DccWorkspaceNext)index=(index+1)%order.size();else index=(index+order.size()-1)%order.size();
        const auto next=order[index];
        if(next==ShipyardWorkspaceMode::Test){
            model_.studioViewMode=ShipyardStudioViewSystem::ForWorkspace(model_.studioViewMode,false);
            model_.workspaceMode=ShipyardWorkspaceMode::Test;model_.testWorkspaceActive=true;
            model_.developerWorkspacesVisible=false;model_.status="Test workspace";return true;
        }
        model_.testWorkspaceActive=false;
        const bool changed=LegacyActivate(WorkspaceCommand(next),0);
        if(changed)model_.studioViewMode=ShipyardStudioViewSystem::ForWorkspace(model_.studioViewMode,next==ShipyardWorkspaceMode::Interior);
        return changed;}
    case ShipyardBuilderCommand::DccPropertiesPrevious:
    case ShipyardBuilderCommand::DccPropertiesNext:{const auto current=InspectorIndex(model_.inspectorTab);const auto next=(current+5+(command==ShipyardBuilderCommand::DccPropertiesNext?1:-1))%5;return LegacyActivate(InspectorCommand(next),0);}
    case ShipyardBuilderCommand::TransformConstraintX:return SetTransformConstraint(model_.transformConstraint==ShipyardTransformConstraint::YZ?ShipyardTransformConstraint::X:ShipyardTransformConstraint::X);
    case ShipyardBuilderCommand::TransformConstraintY:return SetTransformConstraint(ShipyardTransformConstraint::Y);
    case ShipyardBuilderCommand::TransformConstraintZ:return SetTransformConstraint(ShipyardTransformConstraint::Z);
    case ShipyardBuilderCommand::TransformConstraintClear:ClearTransformConstraint();model_.status="Transform constraint cleared";return true;
    case ShipyardBuilderCommand::DccPanelToggleVisible:{
        const auto id=DockPanelIdFromValue(value);auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,id);if(!panel)return false;
        const bool next=!panel->visible;
        const bool ok=next?SubspaceDockSystem::OpenPanel(model_.dockWorkspace,id):SubspaceDockSystem::ClosePanel(model_.dockWorkspace,id);
        if(ok)model_.status=std::string(panel->title)+(next?" shown":" hidden");
        return ok;}
    case ShipyardBuilderCommand::DccPanelToggleCollapse:{
        const auto id=DockPanelIdFromValue(value);auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,id);if(!panel)return false;
        panel->collapsed=!panel->collapsed;model_.status=panel->title+(panel->collapsed?std::string(" collapsed"):std::string(" expanded"));return true;}
    case ShipyardBuilderCommand::DccPanelToggleFloat:{
        const auto id=DockPanelIdFromValue(value);auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,id);if(!panel||!panel->floatable)return false;
        bool floating=false;for(const auto& f:model_.dockWorkspace.floatingPanels)if(f.panelId==id){floating=true;break;}
        if(floating){const bool ok=SubspaceDockSystem::DockPanel(model_.dockWorkspace,id,panel->defaultLeafId,true);if(ok)model_.status=panel->title+" docked";return ok;}
        const float ox=90.0f+static_cast<float>(value)*24.0f,oy=110.0f+static_cast<float>(value)*18.0f;
        const bool ok=SubspaceDockSystem::FloatPanel(model_.dockWorkspace,id,{ox,oy,panel->preferredWidth,panel->preferredHeight});if(ok)model_.status=panel->title+" floating";return ok;}
    case ShipyardBuilderCommand::DccPanelTogglePin:{
        const auto id=DockPanelIdFromValue(value);auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,id);if(!panel)return false;
        const bool ok=SubspaceDockSystem::TogglePinned(model_.dockWorkspace,id);if(ok)model_.status=panel->title+(panel->pinned?" pinned":" unpinned");return ok;}
    case ShipyardBuilderCommand::DccPanelToggleAutoHide:{
        const auto id=DockPanelIdFromValue(value);auto* panel=SubspaceDockSystem::FindPanel(model_.dockWorkspace,id);if(!panel)return false;
        const bool next=!panel->autoHide;const bool ok=SubspaceDockSystem::SetAutoHide(model_.dockWorkspace,id,next);if(ok)model_.status=panel->title+(next?" auto-hide enabled":" auto-hide disabled");return ok;}
    case ShipyardBuilderCommand::DccPanelResetWorkspace:model_.dockWorkspace=ShipyardWorkspaceSystem::BuildDefaultDockWorkspace();model_.status="Dock workspace restored";return true;
    case ShipyardBuilderCommand::DccToggleGuidedWorkflow:model_.guidedWorkflow=!model_.guidedWorkflow;model_.status=model_.guidedWorkflow?"Guided workflow enabled":"Guided workflow hidden";return true;
    case ShipyardBuilderCommand::WorkspaceDevWorld:
        if(value==-1268){
            // The visible TEST tab bypasses LegacyActivate; restore the hull here.
            model_.studioViewMode=ShipyardStudioViewSystem::ForWorkspace(model_.studioViewMode,false);
            model_.workspaceMode=ShipyardWorkspaceMode::Test;model_.testWorkspaceActive=true;
            model_.developerWorkspacesVisible=false;
            model_.status="Test workspace - validate, frame, save draft, and prepare embodied playtest";
            return true;
        }
        break;
    case ShipyardBuilderCommand::WorkspaceAuthoring:
        if(value==-1268){model_.developerWorkspacesVisible=!model_.developerWorkspacesVisible;model_.testWorkspaceActive=false;model_.status=model_.developerWorkspacesVisible?"Developer workspaces expanded":"Developer workspaces collapsed";return true;}
        break;
    case ShipyardBuilderCommand::PcgReroll:
        if(value==-1268){
            const auto before=model_;model_.testWorkspaceActive=false;
            if(!ActivateInternal(ShipyardBuilderCommand::PcgReroll,0))return false;
            const bool generated=ActivateInternal(ShipyardBuilderCommand::GenerateVariant,0);
            if(generated){
                PushAuthoringSnapshot(before);
                if(model_.workspaceMode!=ShipyardWorkspaceMode::Interior)
                    model_.studioViewMode=ShipyardStudioViewSystem::ForPlacement(model_.studioViewMode);
            }
            return generated;
        }
        break;
    default:break;
    }

    const auto priorWorkspace=model_.workspaceMode;
    const bool result=LegacyActivate(command,value);
    if(result&&(model_.workspaceMode!=priorWorkspace||
                command==ShipyardBuilderCommand::WorkspaceInterior||command==ShipyardBuilderCommand::WorkspaceBuild))
        model_.studioViewMode=ShipyardStudioViewSystem::ForWorkspace(
            model_.studioViewMode,model_.workspaceMode==ShipyardWorkspaceMode::Interior);
    // These user-facing actions must reveal a hull in non-Interior workspaces.
    // Preserve intentional Cutaway and X-Ray; never alter a failed action.
    const bool revealExterior=command==ShipyardBuilderCommand::AddModule||
        command==ShipyardBuilderCommand::ConfirmPlacement||
        command==ShipyardBuilderCommand::SelectPlaced||
        command==ShipyardBuilderCommand::FrameSelected||
        command==ShipyardBuilderCommand::FrameShip||
        command==ShipyardBuilderCommand::GenerateVariant||
        command==ShipyardBuilderCommand::NewEmptyDocument;
    if(result&&revealExterior&&model_.workspaceMode!=ShipyardWorkspaceMode::Interior)
        model_.studioViewMode=ShipyardStudioViewSystem::ForPlacement(model_.studioViewMode);
    if(result&&command==ShipyardBuilderCommand::SelectModule){
        // Both native mouse branches call SelectModule on catalog-card press,
        // BEFORE BeginCatalogDrag. Do not wait for release/PLACE to reveal a
        // first hull ghost after the user manually selected Interior-only.
        const auto priorView=model_.studioViewMode;
        model_.studioViewMode=ShipyardStudioViewSystem::ForCatalogPress(
            priorView,model_.workspaceMode==ShipyardWorkspaceMode::Interior);
        if(model_.studioViewMode!=priorView)
            model_.status="EXTERIOR: catalog part selected; drag preview visible";
    }
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
    const auto forgeMetrics=EditorForgeGuiStyleSystem::Metrics(l.compact);
    l.rowHeight=forgeMetrics.propertyRowHeight*s;
    l.rowGap=forgeMetrics.outerGap*s+2.0f*s;
    l.tabRowY=l.workspaceBarY;
    l.tabHeight=std::max(26.0f*s,l.workspaceBarHeight);

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
    l.toolRailHeight=dcc.toolRail.height;

    // Asset Browser is a bottom shelf. Legacy y-fields are intentionally
    // projected into that shelf for test/automation compatibility.
    l.libraryListY=l.assetShelfY+34.0f*s;
    l.moduleCardsY=l.assetShelfY+67.0f*s;
    l.moduleCardHeight=(l.compact?62.0f:78.0f)*s;
    l.leftActionsY=l.assetShelfY+8.0f*s;
    l.leftInfoY=l.assetShelfY+8.0f*s;

    // Outliner owns the upper-right area; Properties owns the lower-right.
    // Reserve a ForgeGUI-style panel header and selected-object header before
    // any interactive property rows. PASS1453 drew controls directly over the
    // selection summary; this explicit vertical contract removes that overlap.
    const float panelHeaderH=forgeMetrics.panelHeaderHeight*s;
    const float objectHeaderH=forgeMetrics.inspectorObjectHeaderHeight*s;
    l.contentTopY=l.propertiesY+panelHeaderH;
    l.selectedSummaryY=l.contentTopY+6.0f*s;
    l.placedListY=l.outlinerY+panelHeaderH+8.0f*s;
    l.placedPageSize=l.compact?4u:6u;

    const float inspectorStart=l.selectedSummaryY+objectHeaderH+8.0f*s;
    l.editLabelY=inspectorStart;l.editRowY=l.editLabelY+24.0f*s;
    l.focusLabelY=l.editRowY+34.0f*s;l.focusRowY=l.focusLabelY+24.0f*s;
    l.moveLabelY=l.focusRowY+34.0f*s;l.moveRowY=l.moveLabelY+24.0f*s;l.moveRow2Y=l.moveRowY+34.0f*s;
    l.rotateLabelY=l.moveRow2Y+36.0f*s;l.rotateRowY=l.rotateLabelY+24.0f*s;l.yawRowY=l.rotateRowY+34.0f*s;l.rollRowY=l.yawRowY+34.0f*s;l.flipRowY=l.rollRowY+34.0f*s;
    l.blueprintLabelY=inspectorStart;l.classRowY=l.blueprintLabelY+24.0f*s;l.sizeModeRowY=l.classRowY+34.0f*s;l.generateRowY=l.sizeModeRowY+34.0f*s;l.saveRowY=l.generateRowY+34.0f*s;
    l.liveryLabelY=inspectorStart;l.liveryRowY=l.liveryLabelY+24.0f*s;l.liverySecondaryRowY=l.liveryRowY+34.0f*s;l.paintSecondaryRowY=l.liverySecondaryRowY+34.0f*s;l.paintTrimRowY=l.paintSecondaryRowY+34.0f*s;l.decalRowY=l.paintTrimRowY+34.0f*s;
    l.statusY=dcc.statusBar.y;
    l.validationY=l.statusY-72.0f*s;
    return l;
}


ShipyardBuilderLayout ShipyardBuilderSystem::Layout(const ShipyardBuilderRuntimeModel& model,int w,int h){
    auto l=Layout(w,h);if(!l.valid)return l;
    // Direct construction/test callers may provide a runtime model without a
    // materialized dock workspace. Preserve the canonical DCC geometry in that
    // case instead of collapsing every panel to width zero.
    if(model.dockWorkspace.panels.empty()||model.dockWorkspace.nodes.empty())return l;

    if(model.dcc.maximizeViewport){
        l.viewportLeft=4.0f*l.uiScale;
        l.viewportRight=static_cast<float>(w)-4.0f*l.uiScale;
        l.viewportTop=l.top;
        l.viewportBottom=l.statusY-2.0f*l.uiScale;
        l.assetShelfHeight=0;l.outlinerWidth=0;l.propertiesWidth=0;l.toolRailWidth=0;l.toolRailHeight=0;
        return l;
    }

    const auto layouts=SubspaceDockSystem::Materialize(model.dockWorkspace,w,std::max(1,static_cast<int>(l.statusY)),l.viewportTop);
    auto find=[&](const char* id)->const SubspaceDockLayout*{
        for(const auto& d:layouts)if(d.panelId==id&&d.visible)return &d;
        return nullptr;
    };
    if(const auto* d=find("viewport")){
        l.viewportLeft=d->rect.x;l.viewportTop=d->rect.y;
        l.viewportRight=d->rect.x+d->rect.width;l.viewportBottom=d->rect.y+d->rect.height;
    } // Full-size canvas is independent of every docked or floating overlay.
    if(const auto* d=find("tool_rail")){
        l.toolRailX=d->rect.x;l.toolRailY=d->rect.y;
        l.toolRailWidth=d->rect.width;l.toolRailHeight=d->rect.height;
    }else {l.toolRailWidth=0;l.toolRailHeight=0;}
    if(const auto* d=find("asset_browser")){
        l.assetShelfX=d->rect.x;l.assetShelfY=d->rect.y;l.assetShelfWidth=d->rect.width;l.assetShelfHeight=d->rect.height;
        l.libraryListY=l.assetShelfY+34.0f*l.uiScale;
        l.moduleCardsY=l.assetShelfY+67.0f*l.uiScale;
    }else l.assetShelfHeight=0;
    if(const auto* d=find("outliner")){
        l.outlinerX=d->rect.x;l.outlinerY=d->rect.y;l.outlinerWidth=d->rect.width;l.outlinerHeight=d->rect.height;
    }else l.outlinerWidth=0;
    if(const auto* d=find("properties")){
        l.propertiesX=d->rect.x;l.propertiesY=d->rect.y;l.propertiesWidth=d->rect.width;l.propertiesHeight=d->rect.height;
    }else l.propertiesWidth=0;

    l.right=l.propertiesX;l.rightWidth=l.propertiesWidth;
    const auto metrics=EditorForgeGuiStyleSystem::Metrics(l.compact);
    const float panelHeaderH=metrics.panelHeaderHeight*l.uiScale;
    const float objectHeaderH=metrics.inspectorObjectHeaderHeight*l.uiScale;
    l.contentTopY=l.propertiesY+panelHeaderH;
    l.selectedSummaryY=l.contentTopY+6.0f*l.uiScale;
    l.placedListY=l.outlinerY+panelHeaderH+8.0f*l.uiScale;
    const float inspectorStart=l.selectedSummaryY+objectHeaderH+8.0f*l.uiScale;
    l.editLabelY=inspectorStart;l.editRowY=l.editLabelY+24.0f*l.uiScale;
    l.focusLabelY=l.editRowY+34.0f*l.uiScale;l.focusRowY=l.focusLabelY+24.0f*l.uiScale;
    l.moveLabelY=l.focusRowY+34.0f*l.uiScale;l.moveRowY=l.moveLabelY+24.0f*l.uiScale;l.moveRow2Y=l.moveRowY+34.0f*l.uiScale;
    l.rotateLabelY=l.moveRow2Y+36.0f*l.uiScale;l.rotateRowY=l.rotateLabelY+24.0f*l.uiScale;l.yawRowY=l.rotateRowY+34.0f*l.uiScale;l.rollRowY=l.yawRowY+34.0f*l.uiScale;l.flipRowY=l.rollRowY+34.0f*l.uiScale;
    l.blueprintLabelY=inspectorStart;l.classRowY=l.blueprintLabelY+24.0f*l.uiScale;l.sizeModeRowY=l.classRowY+34.0f*l.uiScale;l.generateRowY=l.sizeModeRowY+34.0f*l.uiScale;l.saveRowY=l.generateRowY+34.0f*l.uiScale;
    l.liveryLabelY=inspectorStart;l.liveryRowY=l.liveryLabelY+24.0f*l.uiScale;l.liverySecondaryRowY=l.liveryRowY+34.0f*l.uiScale;l.paintSecondaryRowY=l.liverySecondaryRowY+34.0f*l.uiScale;l.paintTrimRowY=l.paintSecondaryRowY+34.0f*l.uiScale;l.decalRowY=l.paintTrimRowY+34.0f*l.uiScale;
    return l;
}


std::vector<ShipyardBuilderControl> ShipyardBuilderSystem::BuildControls(const ShipyardBuilderRuntimeModel& model,int w,int h){
    std::vector<ShipyardBuilderControl> out;
    const auto l=Layout(model,w,h);if(!l.valid)return out;
    const float s=l.uiScale,gap=5.0f*s;
    std::string currentPanelId;
    auto add=[&](ShipyardBuilderCommand c,int value,float x,float y,float cw,float ch,std::string label,bool active=false,bool enabled=true){out.push_back({c,value,x,y,cw,ch,std::move(label),active,enabled});out.back().panelId=currentPanelId;};

    const bool maximized=model.dcc.maximizeViewport;
    // PASS1508: inactive tabs have no materialized rectangle. Do not create
    // phantom hit targets for their controls just because panel.visible=true.
    const bool showAssetBrowser=DockPanelShown(model,"asset_browser")&&l.assetShelfHeight>1.0f&&!maximized;
    const bool assetBrowserContent=showAssetBrowser&&DockPanelVisible(model,"asset_browser");
    const bool showToolRail=DockPanelShown(model,"tool_rail")&&l.toolRailWidth>1.0f&&!maximized;
    const bool toolRailContent=showToolRail&&DockPanelVisible(model,"tool_rail");
    const bool showOutliner=DockPanelShown(model,"outliner")&&l.outlinerWidth>1.0f&&!maximized;
    const bool outlinerContent=showOutliner&&DockPanelVisible(model,"outliner");
    const bool showProperties=DockPanelShown(model,"properties")&&l.propertiesWidth>1.0f&&!maximized;
    const bool propertiesContent=showProperties&&DockPanelVisible(model,"properties");
    const bool showSidebar=(showOutliner||showProperties)&&!maximized;
    // The same panel bounds gate both painting AND hit targets. A row that
    // spills out of a short floating panel must not select a ship behind it.
    auto clipControls=[&](std::size_t first,float x,float y,float width,float height){
        out.erase(std::remove_if(out.begin()+static_cast<std::ptrdiff_t>(first),out.end(),
            [&](const ShipyardBuilderControl& c){
                constexpr float epsilon=.25f;
                return c.x<x-epsilon||c.y<y-epsilon||
                       c.x+c.width>x+width+epsilon||c.y+c.height>y+height+epsilon;
            }),out.end());
    };

    // Application menus are live controls, not painted text placeholders.
    // Expose only actions that actually have an implementation. Open/Save As
    // require a document chooser and are not falsely represented here.
    const ShipyardBuilderCommand menus[4]={ShipyardBuilderCommand::MenuFile,
        ShipyardBuilderCommand::MenuEdit,ShipyardBuilderCommand::MenuView,
        ShipyardBuilderCommand::MenuHelp};
    const char* menuNames[4]={"FILE","EDIT","VIEW","HELP"};
    for(int i=0;i<4;++i)
        add(menus[i],0,(180.0f+i*58.0f)*s,l.top+2.0f*s,54.0f*s,
            std::max(16.0f,l.workspaceBarY-l.top-4.0f*s),menuNames[i],model.openMenu==i,true);
    // First-class authoring flow: assembly -> model -> interior -> systems ->
    // paint -> test. Developer-only workspaces remain under DEV.
    float bx=8.0f*s;const float tabW=82.0f*s,tabGap=2.0f*s;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active,bool enabled=true){add(c,0,bx,l.workspaceBarY,tabW,l.workspaceBarHeight,label,active,enabled);bx+=tabW+tabGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"ASSEMBLY",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Build);
    tab(ShipyardBuilderCommand::WorkspaceModel,"MODEL",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Model,model.capabilities.model);
    tab(ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Interior,model.capabilities.interior);
    tab(ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Systems);
    tab(ShipyardBuilderCommand::WorkspaceAppearance,"PAINT",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Appearance);
    tab(ShipyardBuilderCommand::WorkspaceDevWorld,"TEST",model.testWorkspaceActive,true);out.back().value=-1268;
    add(ShipyardBuilderCommand::WorkspaceAuthoring,-1268,std::max(bx,static_cast<float>(w)-70.0f*s),l.workspaceBarY,62.0f*s,l.workspaceBarHeight,"DEV",model.developerWorkspacesVisible||IsAdvancedWorkspace(model.workspaceMode),true);
    if(model.developerWorkspacesVisible){
        float dx=8.0f*s;const float y=l.workspaceBarY+l.workspaceBarHeight+3.0f*s;const float dw=84.0f*s;
        const std::vector<std::tuple<ShipyardBuilderCommand,const char*,bool,bool>> dev={
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
        // Global recovery is available even if the viewport was maximized
        // while Assets was closed. No invisible-panel dependency.
        add(ShipyardBuilderCommand::DccRevealAssetBrowser,0,8.0f*s,l.top+4.0f*s,83.0f*s,26.0f*s,"ASSETS",false,true);
        add(ShipyardBuilderCommand::DccResetLayout,0,95.0f*s,l.top+4.0f*s,89.0f*s,26.0f*s,"RESET UI",false,true);
        if(model.standaloneDesign)
            add(ShipyardBuilderCommand::NewEmptyDocument,0,188.0f*s,l.top+4.0f*s,100.0f*s,26.0f*s,"NEW EMPTY",false,true);
        add(ShipyardBuilderCommand::DccToggleMaximizeViewport,0,w-92.0f*s,l.top+4.0f*s,82.0f*s,26.0f*s,"RESTORE",true,true);
    }else{
        // Real global recovery/new-document controls occupy the left header;
        // display toggles are placed on the right without overlap.
        // The toolbar belongs to the viewport-header strip, immediately below
        // the workspace tabs. Do not subtract an assumed 25px from canvas Y:
        // the shared GUI metrics can make that overlap the DEV tab at 1280/1852.
        const float vy=l.workspaceBarY+l.workspaceBarHeight;
        const float buttonH=std::min(25.0f*s,std::max(1.0f,l.viewportTop-vy));
        const float controlsW=(48+54+84+54+62+46)*s+5*3.0f*s;
        const float vx=std::max(l.viewportLeft+300.0f*s,l.viewportRight-controlsW-8.0f*s);
        // These are global chrome, not controls inside the Asset Browser.
        // They remain clickable after Close, collapse, tab switches or reload.
        add(ShipyardBuilderCommand::DccRevealAssetBrowser,0,8.0f*s,vy,83.0f*s,buttonH,"ASSETS",false,true);
        add(ShipyardBuilderCommand::DccResetLayout,0,95.0f*s,vy,89.0f*s,buttonH,"RESET UI",false,true);
        if(model.standaloneDesign)
            add(ShipyardBuilderCommand::NewEmptyDocument,0,188.0f*s,vy,100.0f*s,buttonH,"NEW EMPTY",false,true);
        if(model.standaloneDesign&&!HasPlaced(model)&&387.0f*s<=vx-4.0f*s)
            add(ShipyardBuilderCommand::GenerateVariant,0,292.0f*s,vy,95.0f*s,buttonH,"GENERATE",false,true);
        add(ShipyardBuilderCommand::DccToggleGrid,0,vx,vy,48*s,buttonH,"GRID",model.dcc.showGrid,true);
        add(ShipyardBuilderCommand::DccToggleGizmos,0,vx+51*s,vy,54*s,buttonH,model.standaloneDesign?"AXES":"GIZMO",model.dcc.showGizmos,true);
        add(ShipyardBuilderCommand::DccCycleShading,0,vx+108*s,vy,84*s,buttonH,ShipyardDccUiSystem::ShadingName(model.dcc.shading),true,true);
        add(ShipyardBuilderCommand::DccToggleStatsOverlay,0,vx+195*s,vy,54*s,buttonH,"STATS",model.dcc.showStatsOverlay,true);
        add(ShipyardBuilderCommand::DccCycleStudioView,0,vx+252*s,vy,62*s,buttonH,
            ShipyardStudioViewSystem::Name(model.studioViewMode),model.studioViewMode!=ShipyardStudioViewMode::Exterior,true);
        add(ShipyardBuilderCommand::DccToggleMaximizeViewport,0,vx+317*s,vy,46*s,buttonH,"MAX",false,true);
    }

    currentPanelId="asset_browser";
    const std::size_t assetControlStart=out.size();
    if(showAssetBrowser){
        const float ax=l.assetShelfX,ay=l.assetShelfY,aw=l.assetShelfWidth,ah=l.assetShelfHeight;
        // Two-row panel chrome keeps panel-management controls separate from
        // asset actions at narrow widths; PASS1466-1505 treats overlap as a
        // real responsive-layout defect, not something tests should ignore.
        const float headerH=58.0f*s;
        const float actionY=ay+31.0f*s;
        // Every editor area owns the same compact panel controls: collapse,
        // float/dock, pin and close. The global menu is the only fixed chrome.
        const float hb=std::max(24.0f,22.0f*s),hgap=2.0f*s;
        add(ShipyardBuilderCommand::DccPanelToggleCollapse,0,ax+aw-(hb*4+hgap*3)-5*s,ay+4*s,hb,hb,"-",DockPanelCollapsed(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleFloat,0,ax+aw-(hb*3+hgap*2)-5*s,ay+4*s,hb,hb,"[]",false,true);
        add(ShipyardBuilderCommand::DccPanelTogglePin,0,ax+aw-(hb*2+hgap)-5*s,ay+4*s,hb,hb,"P",DockPanelPinned(model,"asset_browser"),true);
        add(ShipyardBuilderCommand::DccPanelToggleVisible,0,ax+aw-hb-5*s,ay+4*s,hb,hb,"X",false,true);
        if(assetBrowserContent){
        // The search field is clickable and keeps native keyboard focus until
        // Enter/Escape or clicking the canvas. It filters the existing catalog.
        if(aw>350*s){
            const float searchW=std::min(230*s,std::max(90*s,aw-205*s));
            add(ShipyardBuilderCommand::DccAssetFocusSearch,0,ax+75*s,ay+4*s,searchW,24*s,
                std::string(model.assetSearchFocused?"SEARCH> ":"SEARCH: ")+
                (model.dcc.assetBrowser.search.empty()?"type to filter":model.dcc.assetBrowser.search),
                model.assetSearchFocused,true);
            if(!model.dcc.assetBrowser.search.empty())
                add(ShipyardBuilderCommand::DccAssetClearSearch,0,ax+75*s+searchW+3*s,ay+4*s,24*s,24*s,"x",false,true);
        }
        // Bottom Asset Browser shelf: the central viewport keeps its width while
        // all ship/module assets remain one click away, matching a modern DCC.
        add(ShipyardBuilderCommand::DccPreviousAssetPreset,0,ax+72*s,actionY,24*s,24*s,"<",false,true);
        add(ShipyardBuilderCommand::DccNextAssetPreset,0,ax+99*s,actionY,144*s,24*s,ShipyardDccUiSystem::AssetPresetName(model.dcc.assetPreset),true,true);
        add(ShipyardBuilderCommand::DccCycleAssetDensity,0,ax+246*s,actionY,82*s,24*s,ShipyardDccUiSystem::AssetDensityName(model.dcc.assetBrowser.density),false,true);
        add(ShipyardBuilderCommand::DccToggleFavoriteSelected,0,ax+aw-270*s,actionY,52*s,24*s,"FAV",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::DccClearAssetFilters,0,ax+aw-215*s,actionY,54*s,24*s,"CLEAR",false,true);
        add(ShipyardBuilderCommand::AddModule,0,ax+aw-158*s,actionY,72*s,24*s,"PLACE",false,!model.catalog.empty());
        add(ShipyardBuilderCommand::Validate,0,ax+aw-83*s,actionY,76*s,24*s,"CHECK",false,true);

        const float categoryY=ay+headerH+2.0f*s;
        const float categoryW=(aw-14.0f*s-7*3.0f*s)/8.0f;
        for(int ci=0;ci<8;++ci){
            const auto cls=static_cast<ShipyardModuleClass>(ci);std::size_t classCount=0;
            for(const auto& rec:model.catalog)if(rec.moduleClass==cls&&
                (model.standaloneDesign||model.availableModuleIds.empty()||
                std::find(model.availableModuleIds.begin(),model.availableModuleIds.end(),rec.source.moduleId)!=model.availableModuleIds.end()))++classCount;
            add(ShipyardBuilderCommand::SelectClass,ci,ax+7*s+ci*(categoryW+3*s),categoryY,categoryW,25.0f*s,
                std::string(ShipyardModuleSystem::ClassName(cls))+" "+std::to_string(classCount),static_cast<int>(model.selectedClass)==ci,true);
        }

        const auto filtered=ShipyardBuilderSystem::VisibleCatalogIndices(model);
        const auto v=ShipyardCatalogViewport::Compute(ax,ay,aw,ah,s,
            model.dcc.assetBrowser.density,model.dcc.assetBrowser.thumbnailScale,
            filtered.size(),model.catalogScrollStart);
        const std::size_t selected=filtered.empty()?0:std::min(model.selectedFilteredModule,filtered.size()-1);
        add(ShipyardBuilderCommand::DccAssetPrevious,0,v.prevX,v.navY,v.navW,v.navH,"<",false,v.start>0);
        add(ShipyardBuilderCommand::DccAssetNext,0,v.nextX,v.navY,v.navW,v.navH,">",false,v.start<v.maxStart);
        for(std::size_t i=0;i<v.pageSize&&v.start+i<filtered.size();++i){
            const auto fi=v.start+i;const auto& rec=model.catalog[filtered[fi]];
            add(ShipyardBuilderCommand::SelectModule,static_cast<int>(fi),v.cardX+i*(v.cardW+v.gap),v.cardY,v.cardW,v.cardH,FriendlyModuleLabel(rec),fi==selected,true);
        }
        }
        // A clipped card must never retain a phantom hit target outside its
        // panel. Shrinking a floating shelf keeps the viewport interactive.
        clipControls(assetControlStart,ax,ay,aw,ah);
    }

    currentPanelId="tool_rail";
    if(showToolRail&&toolRailContent){
        const float tx=l.toolRailX,tw=l.toolRailWidth,th=36.0f*s;
        const float railButtonsY=l.toolRailY+24.0f*s;
        if(model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.testWorkspaceActive){
            const bool hasShape=!model.modeling.recipe.primitives.empty();
            add(ShipyardBuilderCommand::ToolSelect,0,tx,railButtonsY,tw,th,"SELECT",model.transformTool==ShipyardTransformTool::Select,true);
            add(ShipyardBuilderCommand::ToolMove,0,tx,railButtonsY+(th+gap),tw,th,"MOVE",model.transformTool==ShipyardTransformTool::Move,hasShape);
            add(ShipyardBuilderCommand::ToolRotate,0,tx,railButtonsY+2*(th+gap),tw,th,"ROTATE",model.transformTool==ShipyardTransformTool::Rotate,hasShape);
            add(ShipyardBuilderCommand::ToolScale,0,tx,railButtonsY+3*(th+gap),tw,th,"SCALE",model.transformTool==ShipyardTransformTool::Scale,hasShape);
            add(ShipyardBuilderCommand::ModelAddBox,0,tx,railButtonsY+4*(th+gap),tw,th,"ADD BOX",false,model.capabilities.model);
            add(ShipyardBuilderCommand::ModelRemovePrimitive,0,tx,railButtonsY+5*(th+gap),tw,th,"DEL",false,hasShape);
        }else if(model.workspaceMode==ShipyardWorkspaceMode::Interior&&!model.testWorkspaceActive){
            add(ShipyardBuilderCommand::GenerateInteriorProgram,0,tx,railButtonsY,tw,th,"GENERATE",false,HasPlaced(model));
            add(ShipyardBuilderCommand::InteriorAddFloor,0,tx,railButtonsY+(th+gap),tw,th,"FLOOR",false,model.interiorProgram.valid);
            add(ShipyardBuilderCommand::InteriorAddWall,0,tx,railButtonsY+2*(th+gap),tw,th,"WALL",false,model.interiorProgram.valid);
            add(ShipyardBuilderCommand::InteriorAddDoor,0,tx,railButtonsY+3*(th+gap),tw,th,"DOOR DRAFT",false,model.interiorProgram.valid);
            add(ShipyardBuilderCommand::InteriorPreviousElement,0,tx,railButtonsY+4*(th+gap),tw,th,"PREV",false,!model.interiorStructure.elements.empty());
            add(ShipyardBuilderCommand::InteriorNextElement,0,tx,railButtonsY+5*(th+gap),tw,th,"NEXT",false,!model.interiorStructure.elements.empty());
        }else if(model.workspaceMode==ShipyardWorkspaceMode::Appearance&&!model.testWorkspaceActive){
            add(ShipyardBuilderCommand::NextLiveryPreset,0,tx,railButtonsY,tw,th,"LIVERY",false,HasPlaced(model));
            add(ShipyardBuilderCommand::NextPrimaryPaint,0,tx,railButtonsY+(th+gap),tw,th,"PRIMARY",false,HasPlaced(model));
            add(ShipyardBuilderCommand::NextSecondaryPaint,0,tx,railButtonsY+2*(th+gap),tw,th,"SECOND",false,HasPlaced(model));
            add(ShipyardBuilderCommand::NextTrimPaint,0,tx,railButtonsY+3*(th+gap),tw,th,"TRIM",false,HasPlaced(model));
            add(ShipyardBuilderCommand::NextPrimaryFinish,0,tx,railButtonsY+4*(th+gap),tw,th,"FINISH",false,HasPlaced(model));
            add(ShipyardBuilderCommand::AddDecal,0,tx,railButtonsY+5*(th+gap),tw,th,"DECAL",false,HasPlaced(model));
        }else{
            add(ShipyardBuilderCommand::ToolSelect,0,tx,railButtonsY,tw,th,model.standaloneDesign?"SELECT":"Q",model.transformTool==ShipyardTransformTool::Select,true);
            add(ShipyardBuilderCommand::ToolMove,0,tx,railButtonsY+(th+gap),tw,th,model.standaloneDesign?"MOVE":"G",model.transformTool==ShipyardTransformTool::Move,HasPlaced(model));
            add(ShipyardBuilderCommand::ToolRotate,0,tx,railButtonsY+2*(th+gap),tw,th,model.standaloneDesign?"ROTATE":"R",model.transformTool==ShipyardTransformTool::Rotate,HasPlaced(model));
            add(ShipyardBuilderCommand::ToolScale,0,tx,railButtonsY+3*(th+gap),tw,th,model.standaloneDesign?"SCALE":"S",model.transformTool==ShipyardTransformTool::Scale,HasPlaced(model));
            add(ShipyardBuilderCommand::ToggleTransformSnap,0,tx,railButtonsY+4*(th+gap),tw,th,model.standaloneDesign?(model.transformSnap?"SNAP ON":"SNAP OFF"):"SNAP",model.transformSnap,HasPlaced(model));
            add(ShipyardBuilderCommand::FrameSelected,0,tx,railButtonsY+5*(th+gap),tw,th,model.standaloneDesign?"FRAME":"F",false,HasPlaced(model));
        }
    }

    currentPanelId="outliner";
    if(showSidebar){
        const float rx=l.outlinerX+7*s,rw=l.outlinerWidth-14*s;
        const std::size_t outlinerControlStart=out.size();
        if(showOutliner){
            const float hb=std::max(24.0f,21.0f*s),hgap=2.0f*s;const float hx=l.outlinerX+l.outlinerWidth-(hb*4+hgap*3)-5*s;
            add(ShipyardBuilderCommand::DccPanelToggleCollapse,1,hx,l.outlinerY+3*s,hb,hb,"-",DockPanelCollapsed(model,"outliner"),true);
            add(ShipyardBuilderCommand::DccPanelToggleFloat,1,hx+hb+hgap,l.outlinerY+3*s,hb,hb,"[]",false,true);
            add(ShipyardBuilderCommand::DccPanelTogglePin,1,hx+2*(hb+hgap),l.outlinerY+3*s,hb,hb,"P",DockPanelPinned(model,"outliner"),true);
            add(ShipyardBuilderCommand::DccPanelToggleVisible,1,hx+3*(hb+hgap),l.outlinerY+3*s,hb,hb,"X",false,true);
        }
        if(outlinerContent){
            const auto metrics=EditorForgeGuiStyleSystem::Metrics(l.compact);
            const auto navY=l.outlinerY+metrics.panelHeaderHeight*s+3*s;
            add(ShipyardBuilderCommand::DccCycleOutlinerMode,0,rx,navY,std::max(24*s,rw-55*s),24*s,ShipyardDccUiSystem::OutlinerModeName(model.dcc.outlinerMode),true,true);
            add(ShipyardBuilderCommand::DccOutlinerPrevious,0,rx+rw-51*s,navY,24*s,24*s,"<",false,model.placedScrollStart>0);
            add(ShipyardBuilderCommand::DccOutlinerNext,0,rx+rw-25*s,navY,24*s,24*s,">",false,model.placedScrollStart+1<model.recipe.modules.size());
            const std::size_t page=l.compact?4u:5u;const float rowsY=l.outlinerY+metrics.panelHeaderHeight*s+31.0f*s;
            if(model.workspaceMode==ShipyardWorkspaceMode::Model){
                const auto& shapes=model.modeling.recipe.primitives;const std::size_t start=shapes.empty()?0:std::min(model.placedScrollStart,shapes.size()>page?shapes.size()-page:0u);
                for(std::size_t i=0;i<page&&start+i<shapes.size();++i){const auto index=start+i;const auto& p=shapes[index];
                    const std::string label=std::to_string(index+1)+" | "+ShipyardModelingSystem::PrimitiveName(p.type)+" | "+p.id;
                    add(ShipyardBuilderCommand::ModelSelectPrimitive,static_cast<int>(index),rx,rowsY+i*(l.rowHeight+gap),rw,l.rowHeight,label,index==model.modeling.selectedPrimitiveIndex,true);}
            }else{
                const auto rows=ShipyardDccUiSystem::BuildOutlinerRows(model.catalog,model.recipe,model.selectedPlacedModule,model.dcc.outlinerMode);
                const std::size_t start=rows.empty()?0:std::min(model.placedScrollStart,rows.size()>page?rows.size()-page:0u);
                for(std::size_t i=0;i<page&&start+i<rows.size();++i){const auto& item=rows[start+i];std::string displayLabel=item.label;if(item.moduleIndex<model.recipe.modules.size()){const auto& placedId=model.recipe.modules[item.moduleIndex].moduleId;const auto found=std::find_if(model.catalog.begin(),model.catalog.end(),[&](const auto& record){return record.source.moduleId==placedId;});if(found!=model.catalog.end())displayLabel=FriendlyModuleLabel(*found);}std::string label=model.dcc.outlinerMode==ShipyardDccOutlinerMode::Hierarchy?std::string(item.depth*2,' '):std::string{};if(model.dcc.outlinerMode!=ShipyardDccOutlinerMode::Hierarchy)label=item.group+" | ";label+=(item.attached?"|_ ":"o  ")+displayLabel;add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(item.moduleIndex),rx,rowsY+i*(l.rowHeight+gap),rw,l.rowHeight,label,item.selected,true);}
            }
        }
        clipControls(outlinerControlStart,l.outlinerX,l.outlinerY,l.outlinerWidth,l.outlinerHeight);
        currentPanelId="properties";
        const std::size_t propertiesControlStart=out.size();
        if(showProperties){
            const float hb=std::max(24.0f,21.0f*s),hgap=2.0f*s;const float hx=l.propertiesX+l.propertiesWidth-(hb*4+hgap*3)-5*s;
            add(ShipyardBuilderCommand::DccPanelToggleCollapse,2,hx,l.propertiesY+3*s,hb,hb,"-",DockPanelCollapsed(model,"properties"),true);
            add(ShipyardBuilderCommand::DccPanelToggleFloat,2,hx+hb+hgap,l.propertiesY+3*s,hb,hb,"[]",false,true);
            add(ShipyardBuilderCommand::DccPanelTogglePin,2,hx+2*(hb+hgap),l.propertiesY+3*s,hb,hb,"P",DockPanelPinned(model,"properties"),true);
            add(ShipyardBuilderCommand::DccPanelToggleVisible,2,hx+3*(hb+hgap),l.propertiesY+3*s,hb,hb,"X",false,true);
        }
        if(propertiesContent){
            // Use the independent Properties rectangle, never the Outliner origin.
            // Floating one panel must not teleport the other panel's controls.
            const float rx=l.propertiesX+7*s,rw=l.propertiesWidth-14*s;
            // PASS1454-1465: the right side is now a real contextual Inspector.
            // A narrow context rail selects the workflow; only that workflow's
            // actions are projected into the property surface.
            const auto metrics=EditorForgeGuiStyleSystem::Metrics(l.compact);
            const float rail=34.0f*s;
            const float py=l.editLabelY;
            const float tabH=27.0f*s;
            const float tabGap=3.0f*s;
            add(ShipyardBuilderCommand::InspectorTransform,0,rx,py,rail,tabH,model.standaloneDesign?"XYZ":"T",model.inspectorTab==ShipyardInspectorTab::Transform,true);
            add(ShipyardBuilderCommand::InspectorAssembly,0,rx,py+(tabH+tabGap),rail,tabH,model.standaloneDesign?"ASM":"A",model.inspectorTab==ShipyardInspectorTab::Assembly,true);
            add(ShipyardBuilderCommand::InspectorSockets,0,rx,py+2*(tabH+tabGap),rail,tabH,"SCK",model.inspectorTab==ShipyardInspectorTab::Sockets,model.capabilities.sockets);
            add(ShipyardBuilderCommand::InspectorAuthoring,0,rx,py+3*(tabH+tabGap),rail,tabH,model.standaloneDesign?"DEF":"D",model.inspectorTab==ShipyardInspectorTab::Authoring,true);
            add(ShipyardBuilderCommand::InspectorAppearance,0,rx,py+4*(tabH+tabGap),rail,tabH,model.standaloneDesign?"MAT":"M",model.inspectorTab==ShipyardInspectorTab::Appearance,true);

            const float px=rx+rail+7.0f*s;
            const float pw=rw-rail-7.0f*s;
            const float rowH=metrics.propertyRowHeight*s;
            const float rowGap=4.0f*s;
            const float half=(pw-rowGap)*.5f;
            auto two=[&](ShipyardBuilderCommand a,const std::string& al,bool aa,bool ae,
                         ShipyardBuilderCommand b,const std::string& bl,bool ba,bool be,float y){
                add(a,0,px,y,half,rowH,al,aa,ae);
                add(b,0,px+half+rowGap,y,half,rowH,bl,ba,be);
            };

            float ay=l.editRowY;
            if(model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.testWorkspaceActive){
                const bool hasShape=!model.modeling.recipe.primitives.empty();
                two(ShipyardBuilderCommand::ModelPreviousPrimitive,"PREV SHAPE",false,model.capabilities.model,
                    ShipyardBuilderCommand::ModelNextPrimitive,"NEXT SHAPE",false,model.capabilities.model,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ModelAddShape,"CREATE SHAPE",false,model.capabilities.model,
                    ShipyardBuilderCommand::ModelDuplicatePrimitive,"DUPLICATE",false,hasShape,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ModelStretchXNegative,"WIDTH -",false,hasShape,
                    ShipyardBuilderCommand::ModelStretchXPositive,"WIDTH +",false,hasShape,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ModelStretchYNegative,"LENGTH -",false,hasShape,
                    ShipyardBuilderCommand::ModelStretchYPositive,"LENGTH +",false,hasShape,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ModelStretchZNegative,"HEIGHT -",false,hasShape,
                    ShipyardBuilderCommand::ModelStretchZPositive,"HEIGHT +",false,hasShape,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ModelValidate,"VALIDATE DRAFT",false,model.capabilities.model,
                    ShipyardBuilderCommand::ModelPublishCanonical,"PUBLISH UNWIRED",false,false,ay);
            }else if(model.workspaceMode==ShipyardWorkspaceMode::Interior&&!model.testWorkspaceActive){
                add(ShipyardBuilderCommand::GenerateInteriorProgram,0,px,ay,pw,rowH,"GENERATE INTERIOR",false,HasPlaced(model));
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::InteriorPreviousElement,"PREV ELEMENT",false,!model.interiorStructure.elements.empty(),
                    ShipyardBuilderCommand::InteriorNextElement,"NEXT ELEMENT",false,!model.interiorStructure.elements.empty(),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::InteriorAddAirlock,"AIRLOCK DRAFT",false,model.interiorProgram.valid,
                    ShipyardBuilderCommand::InteriorAddHatch,"HATCH DRAFT",false,model.interiorProgram.valid,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::InteriorRemoveElement,"REMOVE",false,!model.interiorStructure.elements.empty(),
                    ShipyardBuilderCommand::Validate,"VALIDATE",false,HasPlaced(model),ay);
            }else switch(model.inspectorTab){
            case ShipyardInspectorTab::Transform:
                two(ShipyardBuilderCommand::FrameSelected,"FRAME PART",false,HasPlaced(model),
                    ShipyardBuilderCommand::FrameShip,"FRAME SHIP",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ToggleTransformSnap,model.transformSnap?"SNAP ON":"SNAP OFF",model.transformSnap,HasPlaced(model),
                    ShipyardBuilderCommand::ToggleLiveSymmetry,model.symmetryFrame.live?"SYMMETRY ON":"SYMMETRY OFF",model.symmetryFrame.live,true,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::DccToggleSocketsOverlay,"SOCKETS",model.dcc.showSocketOverlay,true,
                    ShipyardBuilderCommand::DccToggleShieldPreview,"SHIELD",model.dcc.showShieldPreview,true,ay);
                break;
            case ShipyardInspectorTab::Assembly:
                two(ShipyardBuilderCommand::GenerateVariant,"GENERATE VARIANT",false,true,
                    ShipyardBuilderCommand::Validate,"VALIDATE",false,true,ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::SaveBlueprint,model.validation.valid?"SAVE BLUEPRINT":"SAVE DRAFT",false,HasPlaced(model),
                    ShipyardBuilderCommand::Apply,"APPLY ASSEMBLY",false,HasPlaced(model)&&model.validation.valid&&model.liveApplyEnabled,ay);
                break;
            case ShipyardInspectorTab::Sockets:
                two(ShipyardBuilderCommand::PreviousSocket,"PREV SOCKET",false,HasPlaced(model),
                    ShipyardBuilderCommand::NextSocket,"NEXT SOCKET",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::AddSocket,"ADD SOCKET",false,HasPlaced(model),
                    ShipyardBuilderCommand::RemoveSocket,"REMOVE SOCKET",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::DccToggleSocketsOverlay,"OVERLAY",model.dcc.showSocketOverlay,true,
                    ShipyardBuilderCommand::SaveSocketOverrides,"SAVE SOCKETS",false,HasPlaced(model),ay);
                break;
            case ShipyardInspectorTab::Authoring:
                two(ShipyardBuilderCommand::PreviousSemantic,"PREV SEMANTIC",false,HasPlaced(model),
                    ShipyardBuilderCommand::NextSemantic,"NEXT SEMANTIC",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::ToggleGeneratorEligible,"GENERATOR",false,HasPlaced(model),
                    ShipyardBuilderCommand::TogglePairedPlacement,"PAIRING",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                add(ShipyardBuilderCommand::SaveDefinitionOverrides,0,px,ay,pw,rowH,"SAVE DEFINITION",false,HasPlaced(model));
                break;
            case ShipyardInspectorTab::Appearance:
                two(ShipyardBuilderCommand::PreviousLiveryPreset,"PREV LIVERY",false,HasPlaced(model),
                    ShipyardBuilderCommand::NextLiveryPreset,"NEXT LIVERY",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::NextPrimaryPaint,"PRIMARY",false,HasPlaced(model),
                    ShipyardBuilderCommand::NextSecondaryPaint,"SECONDARY",false,HasPlaced(model),ay);
                ay+=rowH+rowGap;
                two(ShipyardBuilderCommand::NextTrimPaint,"ACCENT",false,HasPlaced(model),
                    ShipyardBuilderCommand::AddDecal,"ADD DECAL",false,HasPlaced(model),ay);
                break;
            }
        }
        clipControls(propertiesControlStart,l.propertiesX,l.propertiesY,l.propertiesWidth,l.propertiesHeight);
    }

    currentPanelId="studio_menu";
    if(model.openMenu>=0&&model.openMenu<4){
        const float x=(180.0f+model.openMenu*58.0f)*s, y=l.workspaceBarY;
        const float width=184.0f*s,height=25.0f*s;
        auto menuRow=[&](ShipyardBuilderCommand cmd,const char* label,bool enabled,int row){
            add(cmd,0,x,y+row*height,width,height,label,false,enabled);
        };
        if(model.openMenu==0){
            menuRow(ShipyardBuilderCommand::NewEmptyDocument,"NEW EMPTY",model.standaloneDesign,0);
            menuRow(ShipyardBuilderCommand::SaveBlueprint,"SAVE BLUEPRINT",HasPlaced(model),1);
            menuRow(ShipyardBuilderCommand::GenerateVariant,"GENERATE SHIP",model.standaloneDesign,2);
        }else if(model.openMenu==1){
            menuRow(ShipyardBuilderCommand::UndoAuthoring,"UNDO",true,0);
            menuRow(ShipyardBuilderCommand::RedoAuthoring,"REDO",true,1);
            menuRow(ShipyardBuilderCommand::RemoveModule,"DELETE MODULE",HasPlaced(model),2);
        }else if(model.openMenu==2){
            menuRow(ShipyardBuilderCommand::DccCycleStudioView,"CYCLE VIEW",true,0);
            menuRow(ShipyardBuilderCommand::DccToggleGrid,"TOGGLE GRID",true,1);
            menuRow(ShipyardBuilderCommand::FrameShip,"FRAME ALL",true,2);
            menuRow(ShipyardBuilderCommand::DccResetLayout,"RESET LAYOUT",true,3);
        }else{
            menuRow(ShipyardBuilderCommand::DccToggleGuidedWorkflow,"TOGGLE GUIDANCE",true,0);
            menuRow(ShipyardBuilderCommand::DccToggleStatsOverlay,"TOGGLE STATS",true,1);
            menuRow(ShipyardBuilderCommand::DccRevealAssetBrowser,"RECOVER ASSETS",true,2);
        }
    }

    currentPanelId.clear();
    // Keep the complete historical command surface discoverable off-screen so
    // old automation and project-authored workflows remain compatible.
    const auto compatibility=LegacyBuildControls(model,w,h);constexpr float kCompatibilityStride=100000.0f;std::size_t compatibilityIndex=0;
    for(auto c:compatibility){
        switch(c.command){
        case ShipyardBuilderCommand::WorkspaceBuild:case ShipyardBuilderCommand::WorkspaceInterior:case ShipyardBuilderCommand::WorkspaceAppearance:case ShipyardBuilderCommand::WorkspaceSystems:case ShipyardBuilderCommand::WorkspaceModel:case ShipyardBuilderCommand::WorkspaceCharacter:case ShipyardBuilderCommand::WorkspacePcg:case ShipyardBuilderCommand::WorkspaceWorld:case ShipyardBuilderCommand::WorkspaceDevWorld:case ShipyardBuilderCommand::WorkspaceProjectTools:case ShipyardBuilderCommand::WorkspaceAuthoring:continue;
        default:break;
        }
        const bool visibleDuplicate=std::any_of(out.begin(),out.end(),[&](const auto& current){return current.command==c.command&&current.value==c.value&&current.width>0.0f&&current.height>0.0f;});if(visibleDuplicate)continue;const float offset=kCompatibilityStride*static_cast<float>(compatibilityIndex++);c.x=-kCompatibilityStride-offset;c.y=-kCompatibilityStride-offset;out.push_back(std::move(c));}
    // Render and hit-test the same z stack: global chrome, docked panels,
    // then floating panels in their workspace order. Stable ordering preserves
    // each panel's own control order; legacy offscreen commands stay offscreen.
    const auto layers=ShipyardPanelCompositorSystem::Snapshot(model.dockWorkspace,w,h,l.viewportTop);
    auto rank=[&](const ShipyardBuilderControl& control){
        if(control.panelId=="studio_menu")return 3000;
        if(control.panelId.empty())return 0;
        for(std::size_t i=0;i<layers.size();++i)
            if(layers[i].panelId==control.panelId)
                return layers[i].floating?1000+static_cast<int>(i):1+static_cast<int>(i);
        return 0;
    };
    std::stable_sort(out.begin(),out.end(),[&](const auto& a,const auto& b){return rank(a)<rank(b);});
    return out;
}

ShipyardBuilderControl ShipyardBuilderSystem::HitTest(const ShipyardBuilderRuntimeModel& model,int w,int h,float x,float y){
    const auto controls=BuildControls(model,w,h);
    const auto layout=Layout(model,w,h);
    const auto layers=ShipyardPanelCompositorSystem::Snapshot(model.dockWorkspace,w,h,layout.viewportTop);
    const auto* topFloating=ShipyardPanelCompositorSystem::TopFloatingAt(layers,x,y);
    for(auto it=controls.rbegin();it!=controls.rend();++it){
        // A floating window occludes all controls beneath its body, including
        // when that area has no control of its own.
        if(topFloating&&it->panelId!=topFloating->panelId&&it->panelId!="studio_menu")continue;
        if(it->Contains(x,y))return *it;
    }
    return {};
}

} // namespace subspace
