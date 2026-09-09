#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardBuilderSystem.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace fs=std::filesystem;

static int passed=0,failed=0;
#define CHECK(name,expr) do{if(expr){++passed;std::cout<<"[PASS] "<<name<<"\n";}else{++failed;std::cout<<"[FAIL] "<<name<<"\n";}}while(0)

static bool Near(float a,float b,float e=1.0e-4f){return std::fabs(a-b)<=e;}

static std::vector<ShipyardModuleRecord> Catalog(){
    ShipyardModuleRecord parent;
    parent.source={"test_hull_parent",1.5f,2.0f,.75f};
    parent.moduleClass=ShipyardModuleClass::Hull;
    parent.semantic=ShipyardModuleSemantic::HullMid;
    parent.size=ShipyardModuleSize::M;
    parent.primaryHull=true;
    parent.generatorEligible=true;
    parent.sockets={
        {"forward","hull_forward",0.0f,2.0f,0.0f,0.0f,1.0f,0.0f,.05f,0.0f,0.0f,1.0f,false},
        {"utility","detail_mount",1.0f,0.0f,0.25f,1.0f,0.0f,0.0f,.05f,0.0f,0.0f,1.0f,false}
    };

    ShipyardModuleRecord child;
    child.source={"test_hull_child",1.0f,1.0f,.50f};
    child.moduleClass=ShipyardModuleClass::Hull;
    child.semantic=ShipyardModuleSemantic::HullMid;
    child.size=ShipyardModuleSize::M;
    child.generatorEligible=true;
    child.sockets={
        {"aft","hull_aft",0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,.05f,0.0f,0.0f,1.0f,false}
    };
    return {parent,child};
}

static ProceduralShipVisualRecipe Recipe(const std::vector<ShipyardModuleRecord>& catalog){
    ProceduralShipVisualRecipe recipe;
    VisualModulePlacement root;root.moduleId=catalog[0].source.moduleId;root.scaleX=root.scaleY=root.scaleZ=1.0f;
    recipe.modules.push_back(root);
    using AttachFn=VisualModulePlacement (*)(const VisualModulePlacement&,const ShipyardAssemblySocket&,const ShipyardModuleRecord&,const ShipyardAssemblySocket&,float);
    const auto attach=static_cast<AttachFn>(&ShipyardModuleSystem::BuildAttachmentPlacement);
    auto child=attach(root,catalog[0].sockets[0],catalog[1],catalog[1].sockets[0],1.0f);
    recipe.modules.push_back(child);
    recipe.attachments.push_back({0,1,"forward","aft",0.0f,true});
    recipe.role="INDUSTRIAL";
    recipe.seed=0x533Bu;
    return recipe;
}

static bool SelectSocketByName(ShipyardBuilderSystem& builder,const std::string& name){
    const auto* record=[&]()->const ShipyardModuleRecord*{
        if(builder.Recipe().modules.empty())return nullptr;
        const auto idx=std::min(builder.Model().selectedPlacedModule,builder.Recipe().modules.size()-1);
        const auto& id=builder.Recipe().modules[idx].moduleId;
        for(const auto& rec:builder.Model().catalog)if(rec.source.moduleId==id)return &rec;
        return nullptr;
    }();
    if(!record||record->sockets.empty())return false;
    for(std::size_t i=0;i<record->sockets.size()+1;++i){
        const auto* socket=builder.SelectedSocket();
        if(socket&&socket->name==name)return true;
        builder.Activate(ShipyardBuilderCommand::NextSocket);
    }
    return false;
}

int main(){
    const auto catalog=Catalog();
    const auto recipe=Recipe(catalog);
    ShipyardBuilderSystem builder;
    builder.Initialize(catalog,recipe);
    builder.Activate(ShipyardBuilderCommand::SelectPlaced,0);
    builder.Activate(ShipyardBuilderCommand::InspectorSockets);
    CHECK("Pass533B socket authoring inspector becomes active",builder.Model().inspectorTab==ShipyardInspectorTab::Sockets);
    CHECK("Pass533B reusable socket definition is selectable",SelectSocketByName(builder,"forward"));

    const float originalSocketX=builder.SelectedSocket()?builder.SelectedSocket()->x:999.0f;
    const float originalChildX=builder.Recipe().modules[1].x;
    builder.Activate(ShipyardBuilderCommand::ToolMove);
    CHECK("Pass533B socket move transaction begins",builder.BeginSelectedSocketTransform());
    CHECK("Pass533B socket origin translates in module-local space",builder.TranslateSelectedSocket({.35f,0.0f,0.0f}));
    CHECK("Pass533B socket move commits",builder.CommitSocketTransform());
    CHECK("Pass533B socket position changed",builder.SelectedSocket()&&Near(builder.SelectedSocket()->x,originalSocketX+.35f));
    CHECK("Pass533B attached child reflows after parent socket move",!Near(builder.Recipe().modules[1].x,originalChildX));
    CHECK("Pass533B socket edits mark override document dirty",builder.Model().socketOverridesDirty);

    CHECK("Pass533B socket edit supports undo",builder.UndoSocketEdit());
    CHECK("Pass533B undo restores socket origin",builder.SelectedSocket()&&Near(builder.SelectedSocket()->x,originalSocketX));
    CHECK("Pass533B undo reflows child back",Near(builder.Recipe().modules[1].x,originalChildX));
    CHECK("Pass533B socket edit supports redo",builder.RedoSocketEdit());
    CHECK("Pass533B redo restores edited socket origin",builder.SelectedSocket()&&Near(builder.SelectedSocket()->x,originalSocketX+.35f));

    builder.Activate(ShipyardBuilderCommand::ToolRotate);
    const Vector3 beforeDir{builder.SelectedSocket()->dirX,builder.SelectedSocket()->dirY,builder.SelectedSocket()->dirZ};
    CHECK("Pass533B socket rotation transaction begins",builder.BeginSelectedSocketTransform());
    CHECK("Pass533B socket frame rotates",builder.RotateSelectedSocket({0.0f,90.0f,0.0f}));
    CHECK("Pass533B socket frame rotation commits",builder.CommitSocketTransform());
    const Vector3 afterDir{builder.SelectedSocket()->dirX,builder.SelectedSocket()->dirY,builder.SelectedSocket()->dirZ};
    CHECK("Pass533B socket direction visibly changes after rotation",(afterDir-beforeDir).length()>.5f);

    const std::size_t beforeProtectedRemove=builder.Model().catalog[0].sockets.size();
    CHECK("Pass533B an actively used mating socket cannot be deleted",!builder.Activate(ShipyardBuilderCommand::RemoveSocket));
    CHECK("Pass533B protected socket remains present",builder.Model().catalog[0].sockets.size()==beforeProtectedRemove);

    CHECK("Pass533B utility socket selectable",SelectSocketByName(builder,"utility"));
    const std::size_t beforeMirror=builder.Model().catalog[0].sockets.size();
    CHECK("Pass533B socket can be mirrored across module X",builder.Activate(ShipyardBuilderCommand::MirrorSocketX));
    CHECK("Pass533B mirror creates a reusable paired socket",builder.Model().catalog[0].sockets.size()==beforeMirror+1&&builder.SelectedSocket()&&builder.SelectedSocket()->x<0.0f);
    const std::string typeBefore=builder.SelectedSocket()->type;
    CHECK("Pass533B manual socket type can be changed",builder.Activate(ShipyardBuilderCommand::CycleSocketType));
    CHECK("Pass533B socket type change persists in working catalog",builder.SelectedSocket()&&builder.SelectedSocket()->type!=typeBefore);

    const std::size_t beforeAdd=builder.Model().catalog[0].sockets.size();
    CHECK("Pass533B editor can add a socket",builder.Activate(ShipyardBuilderCommand::AddSocket));
    CHECK("Pass533B added socket is selected and manual",builder.Model().catalog[0].sockets.size()==beforeAdd+1&&builder.SelectedSocket()&&builder.SelectedSocket()->manualOverride);
    CHECK("Pass533B unused manual socket can be removed",builder.Activate(ShipyardBuilderCommand::RemoveSocket));
    CHECK("Pass533B remove returns socket count",builder.Model().catalog[0].sockets.size()==beforeAdd);

    const fs::path temp=fs::temp_directory_path()/"subspace_pass533b_socket_overrides.subspace_socket_overrides";
    std::error_code ec;fs::remove(temp,ec);
    std::string error;std::size_t changed=0;
    CHECK("Pass533B socket override sidecar saves non-destructively",builder.SaveSocketOverrides(temp.string(),&error,&changed));
    CHECK("Pass533B sidecar contains only changed module definitions",changed==1&&fs::exists(temp));

    ShipyardBuilderSystem reloaded;
    reloaded.Initialize(catalog,recipe);
    std::size_t applied=0;error.clear();
    CHECK("Pass533B socket override sidecar reloads",reloaded.LoadSocketOverrides(temp.string(),&error,&applied));
    reloaded.Activate(ShipyardBuilderCommand::SelectPlaced,0);
    reloaded.Activate(ShipyardBuilderCommand::InspectorSockets);
    CHECK("Pass533B exactly one module override reapplies",applied==1&&SelectSocketByName(reloaded,"forward"));
    CHECK("Pass533B reloaded socket preserves edited origin",reloaded.SelectedSocket()&&Near(reloaded.SelectedSocket()->x,originalSocketX+.35f));
    CHECK("Pass533B reset restores certified/inferred baseline",reloaded.Activate(ShipyardBuilderCommand::ResetSocket));
    CHECK("Pass533B baseline reset restores original socket origin",reloaded.SelectedSocket()&&Near(reloaded.SelectedSocket()->x,originalSocketX));

    const auto controls=ShipyardBuilderSystem::BuildControls(builder.Model(),1440,900);
    bool hasAdd=false,hasRemove=false,hasMirror=false,hasType=false,hasUndo=false,hasRedo=false,hasSave=false;
    for(const auto& c:controls){
        hasAdd|=c.command==ShipyardBuilderCommand::AddSocket;
        hasRemove|=c.command==ShipyardBuilderCommand::RemoveSocket;
        hasMirror|=c.command==ShipyardBuilderCommand::MirrorSocketX;
        hasType|=c.command==ShipyardBuilderCommand::CycleSocketType;
        hasUndo|=c.command==ShipyardBuilderCommand::UndoSocketEdit;
        hasRedo|=c.command==ShipyardBuilderCommand::RedoSocketEdit;
        hasSave|=c.command==ShipyardBuilderCommand::SaveSocketOverrides;
    }
    CHECK("Pass533B sockets inspector exposes add/remove/mirror/type/undo/redo/save controls",hasAdd&&hasRemove&&hasMirror&&hasType&&hasUndo&&hasRedo&&hasSave);

    fs::remove(temp,ec);
    std::cout<<"Pass533B assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed?1:0;
}
