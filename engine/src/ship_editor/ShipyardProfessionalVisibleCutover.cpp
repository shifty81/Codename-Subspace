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

} // namespace

bool ShipyardBuilderSystem::Activate(ShipyardBuilderCommand command,int value){
    switch(command){
    case ShipyardBuilderCommand::WorkspaceDevWorld:
        if(value==-1268){
            model_.workspaceMode=ShipyardWorkspaceMode::Test;
            model_.testWorkspaceActive=true;
            model_.developerWorkspacesVisible=false;
            model_.status="Test workspace - validate, frame, save draft, and prepare embodied playtest";
            return true;
        }
        break;
    case ShipyardBuilderCommand::WorkspaceAuthoring:
        if(value==-1268){
            model_.developerWorkspacesVisible=!model_.developerWorkspacesVisible;
            model_.testWorkspaceActive=false;
            model_.status=model_.developerWorkspacesVisible?"Developer workspaces expanded":"Developer workspaces collapsed";
            return true;
        }
        break;
    case ShipyardBuilderCommand::PcgReroll:
        if(value==-1268){
            // One visible gesture = one authoring transaction. Compose the
            // existing internal Reroll and Generate actions directly, then
            // commit a single legacy history snapshot. Generation itself is
            // still the exact canonical GenerateVariant implementation.
            const auto before=model_;
            model_.testWorkspaceActive=false;
            if(!ActivateInternal(ShipyardBuilderCommand::PcgReroll,0))return false;
            const bool generated=ActivateInternal(ShipyardBuilderCommand::GenerateVariant,0);
            if(generated)PushAuthoringSnapshot(before);
            return generated;
        }
        break;
    default:
        break;
    }

    const bool result=LegacyActivate(command,value);
    if(result){
        switch(command){
        case ShipyardBuilderCommand::WorkspaceBuild:
        case ShipyardBuilderCommand::WorkspaceInterior:
        case ShipyardBuilderCommand::WorkspaceSystems:
        case ShipyardBuilderCommand::WorkspaceAppearance:
            model_.testWorkspaceActive=false;
            model_.developerWorkspacesVisible=false;
            break;
        case ShipyardBuilderCommand::WorkspaceModel:
        case ShipyardBuilderCommand::WorkspaceCharacter:
        case ShipyardBuilderCommand::WorkspacePcg:
        case ShipyardBuilderCommand::WorkspaceWorld:
        case ShipyardBuilderCommand::WorkspaceDevWorld:
        case ShipyardBuilderCommand::WorkspaceProjectTools:
        case ShipyardBuilderCommand::WorkspaceAuthoring:
            model_.testWorkspaceActive=false;
            model_.developerWorkspacesVisible=true;
            break;
        case ShipyardBuilderCommand::InspectorSockets:
            // Socket authoring is a developer surface. Keep the DEV strip
            // visible and let the certified legacy inspector own the details.
            model_.testWorkspaceActive=false;
            model_.developerWorkspacesVisible=true;
            break;
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

    // Professional shell: compact global workspace bar, GameMaker-like asset
    // browser left, Blender-like tool rail beside the viewport, split
    // Outliner/Properties authority on the right, and a reserved bottom dock.
    l.workspaceBarY=8.0f*s;
    l.workspaceBarHeight=36.0f*s;
    l.left=18.0f*s;
    l.top=58.0f*s;
    l.leftWidth=std::clamp(static_cast<float>(w)*.22f,330.0f*s,520.0f*s);
    l.rightWidth=std::clamp(static_cast<float>(w)*.27f,390.0f*s,640.0f*s);
    l.right=static_cast<float>(w)-l.rightWidth-14.0f*s;
    l.toolRailWidth=76.0f*s;
    l.toolRailX=l.left+l.leftWidth+10.0f*s;
    l.toolRailY=l.top+72.0f*s;
    l.rowHeight=(l.compact?30.0f:34.0f)*s;
    l.rowGap=6.0f*s;

    l.libraryListY=l.top+78.0f*s;
    l.moduleCardsY=l.libraryListY+2.0f*(l.rowHeight+l.rowGap)+48.0f*s;
    l.moduleCardHeight=(l.compact?62.0f:72.0f)*s;
    l.leftActionsY=l.moduleCardsY+5.0f*(l.moduleCardHeight+l.rowGap)+12.0f*s;
    l.leftInfoY=l.leftActionsY+82.0f*s;

    // Existing renderer consumes these coordinates for the right panel content.
    // They now describe a visible Outliner region followed by Properties.
    l.tabRowY=l.top+58.0f*s;
    l.tabHeight=34.0f*s;
    l.contentTopY=l.top+112.0f*s;
    l.selectedSummaryY=l.contentTopY;
    l.placedListY=l.contentTopY+34.0f*s;
    l.placedPageSize=l.compact?4u:5u;

    const float outlinerBottom=l.placedListY+static_cast<float>(l.placedPageSize)*(l.rowHeight+l.rowGap)+18.0f*s;
    l.editLabelY=outlinerBottom+26.0f*s;
    l.editRowY=l.editLabelY+24.0f*s;
    l.focusLabelY=l.editRowY+42.0f*s;
    l.focusRowY=l.focusLabelY+24.0f*s;
    l.moveLabelY=l.focusRowY+42.0f*s;
    l.moveRowY=l.moveLabelY+24.0f*s;
    l.moveRow2Y=l.moveRowY+42.0f*s;
    l.rotateLabelY=l.moveRow2Y+44.0f*s;
    l.rotateRowY=l.rotateLabelY+24.0f*s;
    l.yawRowY=l.rotateRowY+42.0f*s;
    l.rollRowY=l.yawRowY+42.0f*s;
    l.flipRowY=l.rollRowY+42.0f*s;

    l.blueprintLabelY=outlinerBottom+28.0f*s;
    l.classRowY=l.blueprintLabelY+24.0f*s;
    l.sizeModeRowY=l.classRowY+44.0f*s;
    l.generateRowY=l.sizeModeRowY+44.0f*s;
    l.saveRowY=l.generateRowY+44.0f*s;
    l.liveryLabelY=outlinerBottom+30.0f*s;
    l.liveryRowY=l.liveryLabelY+28.0f*s;
    l.liverySecondaryRowY=l.liveryRowY+46.0f*s;
    l.paintSecondaryRowY=l.liverySecondaryRowY+46.0f*s;
    l.paintTrimRowY=l.paintSecondaryRowY+46.0f*s;
    l.decalRowY=l.paintTrimRowY+46.0f*s;

    l.statusY=static_cast<float>(h)-42.0f*s;
    l.validationY=l.statusY-116.0f*s;
    return l;
}

std::vector<ShipyardBuilderControl> ShipyardBuilderSystem::BuildControls(const ShipyardBuilderRuntimeModel& model,int w,int h){
    std::vector<ShipyardBuilderControl> out;
    const auto l=Layout(w,h);if(!l.valid)return out;
    const float s=l.uiScale,gap=6.0f*s;
    auto add=[&](ShipyardBuilderCommand c,int value,float x,float y,float cw,float ch,std::string label,bool active=false,bool enabled=true){
        out.push_back({c,value,x,y,cw,ch,std::move(label),active,enabled});
    };

    // Primary workspace strip. It is deliberately screen-wide and shallow so
    // the center viewport remains the dominant surface.
    const float barX=l.left;
    const float barRight=l.right+l.rightWidth;
    const float barGap=6.0f*s;
    const float devW=82.0f*s;
    // Task workspaces are compact navigation tabs, not giant screen-wide
    // buttons. Preserve viewport/titlebar room for status and future search.
    const float primaryW=145.0f*s;
    float bx=barX;
    auto tab=[&](ShipyardBuilderCommand c,const char* label,bool active){add(c,0,bx,l.workspaceBarY,primaryW,l.workspaceBarHeight,label,active,true);bx+=primaryW+barGap;};
    tab(ShipyardBuilderCommand::WorkspaceBuild,"BUILD",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Build);
    tab(ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Interior);
    tab(ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Systems);
    tab(ShipyardBuilderCommand::WorkspaceAppearance,"APPEARANCE",!model.testWorkspaceActive&&model.workspaceMode==ShipyardWorkspaceMode::Appearance);
    add(ShipyardBuilderCommand::WorkspaceDevWorld,-1268,bx,l.workspaceBarY,primaryW,l.workspaceBarHeight,"TEST",model.testWorkspaceActive,true);bx+=primaryW+barGap;
    add(ShipyardBuilderCommand::WorkspaceAuthoring,-1268,barRight-devW,l.workspaceBarY,devW,l.workspaceBarHeight,"DEV",model.developerWorkspacesVisible||IsAdvancedWorkspace(model.workspaceMode),true);

    if(model.developerWorkspacesVisible){
        const float y=l.workspaceBarY+l.workspaceBarHeight+5.0f*s;
        const float x0=l.left+l.leftWidth+98.0f*s;
        const float x1=l.right-8.0f*s;
        std::vector<std::tuple<ShipyardBuilderCommand,const char*,bool,bool>> dev={
            {ShipyardBuilderCommand::WorkspaceModel,"MODEL",model.workspaceMode==ShipyardWorkspaceMode::Model,model.capabilities.model},
            {ShipyardBuilderCommand::InspectorSockets,"SOCKETS",model.inspectorTab==ShipyardInspectorTab::Sockets,model.capabilities.sockets},
            {ShipyardBuilderCommand::WorkspaceCharacter,"CHARACTER",model.workspaceMode==ShipyardWorkspaceMode::Character,model.capabilities.character},
            {ShipyardBuilderCommand::WorkspacePcg,"PCG LAB",model.workspaceMode==ShipyardWorkspaceMode::Pcg,model.capabilities.pcgStudio},
            {ShipyardBuilderCommand::WorkspaceWorld,"WORLD",model.workspaceMode==ShipyardWorkspaceMode::World,model.capabilities.world},
            {ShipyardBuilderCommand::WorkspaceDevWorld,"DEV WORLD",model.workspaceMode==ShipyardWorkspaceMode::DevWorld,model.capabilities.devWorld},
            {ShipyardBuilderCommand::WorkspaceProjectTools,"PROJECT",model.workspaceMode==ShipyardWorkspaceMode::ProjectTools,true},
            {ShipyardBuilderCommand::WorkspaceAuthoring,"AUTHOR",model.workspaceMode==ShipyardWorkspaceMode::Authoring,model.capabilities.rawAuthoring}
        };
        const float cw=(x1-x0-gap*static_cast<float>(dev.size()-1))/static_cast<float>(dev.size());
        float x=x0;for(const auto& d:dev){add(std::get<0>(d),0,x,y,cw,30.0f*s,std::get<1>(d),std::get<2>(d),std::get<3>(d));x+=cw+gap;}
    }

    // GameMaker-style Asset Browser: categories/tags plus real visual cards.
    const float left=l.left,libraryW=l.leftWidth,rowH=l.rowHeight;
    add(ShipyardBuilderCommand::SelectClass,-1,left+10*s,l.top+42*s,libraryW-20*s,28*s,"SEARCH ASSETS  [F3]",false,false);
    const float categoryGap=5.0f*s;
    const float categoryW=(libraryW-20.0f*s-categoryGap*3.0f)/4.0f;
    for(int ci=0;ci<8;++ci){
        const auto cls=static_cast<ShipyardModuleClass>(ci);
        const int col=ci%4,row=ci/4;
        std::size_t classCount=0;for(const auto& rec:model.catalog)if(rec.moduleClass==cls)++classCount;
        add(ShipyardBuilderCommand::SelectClass,ci,left+10*s+col*(categoryW+categoryGap),l.libraryListY+row*(rowH+gap),categoryW,rowH,
            std::string(ShipyardModuleSystem::ClassName(cls))+" ("+std::to_string(classCount)+")",static_cast<int>(model.selectedClass)==ci,true);
    }
    add(ShipyardBuilderCommand::SelectClass,-1,left+10*s,l.moduleCardsY-34*s,libraryW-20*s,26*s,"FAVORITES  |  COMPATIBLE  |  CERTIFIED",false,false);

    std::vector<std::size_t> filtered;
    for(std::size_t i=0;i<model.catalog.size();++i)if(model.catalog[i].moduleClass==model.selectedClass)filtered.push_back(i);
    constexpr std::size_t pageSize=5;
    const std::size_t selected=filtered.empty()?0:std::min(model.selectedFilteredModule,filtered.size()-1);
    const std::size_t maxStart=filtered.size()>pageSize?filtered.size()-pageSize:0;
    const std::size_t start=filtered.empty()?0:std::min(model.catalogScrollStart,maxStart);
    for(std::size_t i=0;i<pageSize&&start+i<filtered.size();++i){
        const auto fi=start+i;const auto& rec=model.catalog[filtered[fi]];
        add(ShipyardBuilderCommand::SelectModule,static_cast<int>(fi),left+10*s,l.moduleCardsY+i*(l.moduleCardHeight+gap),libraryW-20*s,l.moduleCardHeight,
            FriendlyModuleLabel(rec),fi==selected,true);
    }
    const float actionY=l.leftActionsY;
    const float inner=libraryW-20*s;
    add(ShipyardBuilderCommand::PreviousModule,0,left+10*s,actionY,52*s,32*s,"<");
    add(ShipyardBuilderCommand::NextModule,0,left+68*s,actionY,52*s,32*s,">");
    add(ShipyardBuilderCommand::AddModule,0,left+126*s,actionY,88*s,32*s,"PLACE",false,!filtered.empty());
    add(ShipyardBuilderCommand::ReplaceModule,0,left+220*s,actionY,inner-210*s,32*s,"REPLACE",false,!filtered.empty()&&HasPlaced(model));
    add(ShipyardBuilderCommand::RemoveModule,0,left+10*s,actionY+38*s,92*s,32*s,"REMOVE",false,HasPlaced(model));
    add(ShipyardBuilderCommand::ToggleLiveSymmetry,0,left+108*s,actionY+38*s,126*s,32*s,model.symmetryFrame.live?"SYMMETRY ON":"SYMMETRY OFF",model.symmetryFrame.live,true);
    add(ShipyardBuilderCommand::Validate,0,left+240*s,actionY+38*s,inner-230*s,32*s,"VALIDATE",false,true);

    // Blender-style tool rail: readable names, original hotkeys preserved.
    const float tx=l.toolRailX,tw=l.toolRailWidth,th=38.0f*s;
    add(ShipyardBuilderCommand::ToolSelect,0,tx,l.toolRailY,tw,th,"SELECT [Q]",model.transformTool==ShipyardTransformTool::Select,true);
    add(ShipyardBuilderCommand::ToolMove,0,tx,l.toolRailY+(th+gap),tw,th,"MOVE [W]",model.transformTool==ShipyardTransformTool::Move,HasPlaced(model));
    add(ShipyardBuilderCommand::ToolRotate,0,tx,l.toolRailY+2*(th+gap),tw,th,"ROTATE [E]",model.transformTool==ShipyardTransformTool::Rotate,HasPlaced(model));
    add(ShipyardBuilderCommand::ToolScale,0,tx,l.toolRailY+3*(th+gap),tw,th,"SCALE [R]",model.transformTool==ShipyardTransformTool::Scale,HasPlaced(model));
    add(ShipyardBuilderCommand::ToggleTransformSnap,0,tx,l.toolRailY+4*(th+gap),tw,th,model.transformSnap?"SNAP ON":"SNAP OFF",model.transformSnap,HasPlaced(model));
    add(ShipyardBuilderCommand::FrameSelected,0,tx,l.toolRailY+5*(th+gap),tw,th,"FRAME [F]",false,HasPlaced(model));

    // Right side: visible Outliner region first. Selection remains the same
    // authoritative recipe selection used by viewport transforms.
    const float rx=l.right+12*s,rw=l.rightWidth-24*s;
    add(ShipyardBuilderCommand::SelectPlaced,-1,rx,l.top+44*s,rw,28*s,"OUTLINER  /  SHIP HIERARCHY",false,false);
    const std::size_t psel=model.recipe.modules.empty()?0:std::min(model.selectedPlacedModule,model.recipe.modules.size()-1);
    const std::size_t placedPage=l.placedPageSize;
    const std::size_t maxPlaced=model.recipe.modules.size()>placedPage?model.recipe.modules.size()-placedPage:0;
    const std::size_t pstart=model.recipe.modules.empty()?0:std::min(model.placedScrollStart,maxPlaced);
    for(std::size_t r=0;r<placedPage&&pstart+r<model.recipe.modules.size();++r){
        const auto pi=pstart+r;
        std::string label=std::to_string(pi+1)+". "+model.recipe.modules[pi].moduleId;
        for(const auto& rec:model.catalog)if(rec.source.moduleId==model.recipe.modules[pi].moduleId){label=std::to_string(pi+1)+". "+FriendlyModuleLabel(rec);break;}
        add(ShipyardBuilderCommand::SelectPlaced,static_cast<int>(pi),rx,l.placedListY+r*(rowH+gap),rw,rowH,label,pi==psel,true);
    }

    const float propertiesY=l.editLabelY-6*s;
    add(ShipyardBuilderCommand::SelectPlaced,-1,rx,propertiesY,rw,28*s,"PROPERTIES  /  INSTANCE + DEFINITION",false,false);
    auto row=[&](float y,const std::vector<std::tuple<ShipyardBuilderCommand,std::string,bool,bool>>& items){
        const float g=6*s;const float cw=(rw-g*static_cast<float>(items.size()-1))/static_cast<float>(items.size());float x=rx;
        for(const auto& item:items){add(std::get<0>(item),0,x,y,cw,34*s,std::get<1>(item),std::get<2>(item),std::get<3>(item));x+=cw+g;}
    };

    if(model.testWorkspaceActive){
        row(l.editRowY,{{ShipyardBuilderCommand::Validate,"VALIDATE SHIP",false,true},{ShipyardBuilderCommand::FrameShip,"FRAME SHIP",false,HasPlaced(model)}});
        row(l.focusRowY,{{ShipyardBuilderCommand::SaveBlueprint,model.validation.valid?"SAVE BLUEPRINT":"SAVE DRAFT",false,HasPlaced(model)},{ShipyardBuilderCommand::Apply,model.liveApplyEnabled?"APPLY REFIT":"APPLY WHEN DOCKED",false,model.liveApplyEnabled&&model.validation.valid}});
        row(l.moveRowY,{{ShipyardBuilderCommand::WorkspaceInterior,"INTERIOR FPS",false,model.capabilities.interior},{ShipyardBuilderCommand::WorkspaceSystems,"SYSTEM CHECK",false,true}});
    }else if(model.workspaceMode==ShipyardWorkspaceMode::Systems){
        row(l.editRowY,{{ShipyardBuilderCommand::PreviousShipClass,"< CLASS",false,true},{ShipyardBuilderCommand::NextShipClass,std::string(ShipClassRoleSystem::ClassName(model.shipClass))+" >",true,true}});
        row(l.focusRowY,{{ShipyardBuilderCommand::PreviousTargetSize,"< SIZE",false,true},{ShipyardBuilderCommand::NextTargetSize,std::string(UniversalKitbashAuthority::SizeName(model.targetModuleSize))+" >",true,true}});
        {
            const float g=6*s; const float cw=(rw-g*2.0f)/3.0f;
            add(ShipyardBuilderCommand::GenerateVariant,0,rx,l.moveRowY,cw,34*s,"GENERATE",false,true);
            add(ShipyardBuilderCommand::PcgReroll,-1268,rx+cw+g,l.moveRowY,cw,34*s,"NEW SEED + GENERATE",false,true);
            add(ShipyardBuilderCommand::PcgAudit,0,rx+2*(cw+g),l.moveRowY,cw,34*s,"EXPLAIN",false,model.capabilities.pcgStudio);
        }
        row(l.moveRow2Y,{{ShipyardBuilderCommand::Validate,"VALIDATE",false,true},{ShipyardBuilderCommand::SaveBlueprint,model.validation.valid?"SAVE BLUEPRINT":"SAVE DRAFT",false,HasPlaced(model)}});
    }else if(model.workspaceMode==ShipyardWorkspaceMode::Appearance){
        row(l.editRowY,{{ShipyardBuilderCommand::PreviousLiveryPreset,"< PRESET",false,true},{ShipyardBuilderCommand::NextLiveryPreset,"PRESET >",false,true}});
        row(l.focusRowY,{{ShipyardBuilderCommand::PreviousPrimaryPaint,"< PRIMARY",false,true},{ShipyardBuilderCommand::NextPrimaryPaint,"PRIMARY >",false,true}});
        row(l.moveRowY,{{ShipyardBuilderCommand::PreviousSecondaryPaint,"< SECONDARY",false,true},{ShipyardBuilderCommand::NextSecondaryPaint,"SECONDARY >",false,true}});
        row(l.moveRow2Y,{{ShipyardBuilderCommand::PreviousTrimPaint,"< ACCENT",false,true},{ShipyardBuilderCommand::NextTrimPaint,"ACCENT >",false,true}});
    }else if(model.workspaceMode==ShipyardWorkspaceMode::Interior){
        row(l.editRowY,{{ShipyardBuilderCommand::Validate,"REBUILD INTERIOR PLAN",false,true},{ShipyardBuilderCommand::FrameShip,"FRAME SHIP",false,HasPlaced(model)}});
        row(l.focusRowY,{{ShipyardBuilderCommand::InspectorSockets,"APERTURES / SOCKETS",false,model.capabilities.sockets},{ShipyardBuilderCommand::WorkspaceSystems,"SYSTEMS",false,true}});
    }else if(model.workspaceMode==ShipyardWorkspaceMode::Build){
        row(l.editRowY,{{ShipyardBuilderCommand::ToggleTransformSpace,model.transformSpace==ShipyardTransformSpace::View?"VIEW SPACE":(model.transformSpace==ShipyardTransformSpace::Ship?"SHIP SPACE":"LOCAL SPACE"),false,HasPlaced(model)},{ShipyardBuilderCommand::ToggleLiveSymmetry,model.symmetryFrame.live?"SYMMETRY ON":"SYMMETRY OFF",model.symmetryFrame.live,true}});
        row(l.focusRowY,{{ShipyardBuilderCommand::FrameSelected,"FRAME PART",false,HasPlaced(model)},{ShipyardBuilderCommand::FrameShip,"FRAME SHIP",false,HasPlaced(model)}});
        if(model.transformTool==ShipyardTransformTool::Move)row(l.moveRowY,{{ShipyardBuilderCommand::NudgePort,"PORT",false,HasPlaced(model)},{ShipyardBuilderCommand::NudgeStarboard,"STARBOARD",false,HasPlaced(model)},{ShipyardBuilderCommand::NudgeForward,"FORWARD",false,HasPlaced(model)}});
        else if(model.transformTool==ShipyardTransformTool::Rotate)row(l.moveRowY,{{ShipyardBuilderCommand::RotatePitchNegative,"PITCH -",false,HasPlaced(model)},{ShipyardBuilderCommand::RotatePitchPositive,"PITCH +",false,HasPlaced(model)},{ShipyardBuilderCommand::CycleRotationStep,"ROTATION STEP",false,HasPlaced(model)}});
        else if(model.transformTool==ShipyardTransformTool::Scale)row(l.moveRowY,{{ShipyardBuilderCommand::ScaleUniformNegative,"PART -5%",false,HasPlaced(model)},{ShipyardBuilderCommand::ScaleUniformPositive,"PART +5%",false,HasPlaced(model)},{ShipyardBuilderCommand::ResetScale,"RESET SCALE",false,HasPlaced(model)}});
        else row(l.moveRowY,{{ShipyardBuilderCommand::MirrorSelectedAcrossSymmetry,"MIRROR COPY",false,HasPlaced(model)},{ShipyardBuilderCommand::BreakSymmetryPair,"BREAK PAIR",false,HasPlaced(model)}});
    }else{
        // Advanced workspaces keep the already-certified legacy controls
        // reachable while the permanent shell stays compact.
        const auto legacy=LegacyBuildControls(model,w,h);
        for(const auto& c:legacy){
            if(c.command==ShipyardBuilderCommand::WorkspaceBuild||c.command==ShipyardBuilderCommand::WorkspaceInterior||c.command==ShipyardBuilderCommand::WorkspaceSystems||c.command==ShipyardBuilderCommand::WorkspaceAppearance)continue;
            if(c.x<l.right-1.0f)continue;
            out.push_back(c);
        }
    }

    // Migration compatibility projection. Historical native tests and older
    // automation discover commands through BuildControls. Preserve that API
    // without re-rendering the old shell by moving the certified legacy page
    // controls far outside the viewport. Workspace-strip duplicates are
    // excluded because the new primary/DEV hierarchy is now authoritative.
    // This block is deleted when historical tests are migrated to command IDs.
    const auto compatibility=LegacyBuildControls(model,w,h);
    // Historical controls remain discoverable to older tests/automation, but
    // they must never participate in visible layout geometry. A large diagonal
    // stride guarantees each compatibility rectangle is isolated even when a
    // later legacy control is substantially wider/taller than the previous one.
    constexpr float kCompatibilityStride=100000.0f;
    std::size_t compatibilityIndex=0;
    for(auto c:compatibility){
        switch(c.command){
        case ShipyardBuilderCommand::WorkspaceBuild:
        case ShipyardBuilderCommand::WorkspaceInterior:
        case ShipyardBuilderCommand::WorkspaceAppearance:
        case ShipyardBuilderCommand::WorkspaceSystems:
        case ShipyardBuilderCommand::WorkspaceModel:
        case ShipyardBuilderCommand::WorkspaceCharacter:
        case ShipyardBuilderCommand::WorkspacePcg:
        case ShipyardBuilderCommand::WorkspaceWorld:
        case ShipyardBuilderCommand::WorkspaceDevWorld:
        case ShipyardBuilderCommand::WorkspaceProjectTools:
        case ShipyardBuilderCommand::WorkspaceAuthoring:
            continue;
        default:break;
        }
        // Do not duplicate commands already present in the professional visible
        // shell unless the historical page needs a different command family.
        const bool visibleDuplicate=std::any_of(out.begin(),out.end(),[&](const auto& current){
            return current.command==c.command && current.value==c.value && current.width>0.0f && current.height>0.0f;
        });
        if(visibleDuplicate)continue;
        const float offset=kCompatibilityStride*static_cast<float>(compatibilityIndex++);
        c.x=-kCompatibilityStride-offset;
        c.y=-kCompatibilityStride-offset;
        out.push_back(std::move(c));
    }
    return out;
}

ShipyardBuilderControl ShipyardBuilderSystem::HitTest(const ShipyardBuilderRuntimeModel& model,int w,int h,float x,float y){
    const auto controls=BuildControls(model,w,h);
    for(auto it=controls.rbegin();it!=controls.rend();++it)if(it->Contains(x,y))return *it;
    return {};
}

} // namespace subspace
