#include "ship_editor/ShipyardProfessionalController.h"

namespace subspace {

void ShipyardProfessionalController::Bind(ShipyardBuilderSystem& builder,std::string persistentShipId){
    builder_=&builder;migration_=ShipyardBuilderMigrationSystem::Bind(builder.Model(),std::move(persistentShipId));
}
void ShipyardProfessionalController::Pull(){if(builder_)ShipyardBuilderMigrationSystem::PullFromLegacy(builder_->Model(),migration_);}
bool ShipyardProfessionalController::ActivateLegacy(ShipyardBuilderCommand c,int value){
    if(!builder_)return false;
    const bool ok=builder_->Activate(c,value);
    Pull();
    return ok;
}

ShipyardCommandResult ShipyardProfessionalController::ExecuteRuntime(std::string_view id,int value){
    if(!builder_)return {false,false,"Shipyard runtime is not bound"};
    if(id=="generator.generate"){const bool ok=ActivateLegacy(ShipyardBuilderCommand::GenerateVariant,value);return {ok,ok,builder_->Model().status};}
    if(id=="generator.new-seed-generate"){
        if(!ActivateLegacy(ShipyardBuilderCommand::PcgReroll,0))return {false,false,builder_->Model().status};
        const bool ok=ActivateLegacy(ShipyardBuilderCommand::GenerateVariant,value);return {ok,ok,builder_->Model().status};
    }
    if(id=="generator.explain"){const bool ok=ActivateLegacy(ShipyardBuilderCommand::PcgAudit,value);return {ok,false,builder_->Model().status};}
    if(id=="validation.run"){const bool ok=ActivateLegacy(ShipyardBuilderCommand::Validate,value);return {ok,false,builder_->Model().status};}
    return {false,false,"No runtime adapter for command"};
}

bool ShipyardProfessionalController::ApplySessionToLegacy(std::string_view id){
    if(!builder_)return false;
    if(id=="workspace.build")return ActivateLegacy(ShipyardBuilderCommand::WorkspaceBuild);
    if(id=="workspace.interior")return ActivateLegacy(ShipyardBuilderCommand::WorkspaceInterior);
    if(id=="workspace.systems")return ActivateLegacy(ShipyardBuilderCommand::WorkspaceSystems);
    if(id=="workspace.appearance")return ActivateLegacy(ShipyardBuilderCommand::WorkspaceAppearance);
    // Test intentionally remains a real session workspace until Play/Test owns
    // the same persistent ship. Never fake TEST by routing to Dev World or by
    // pulling the legacy workspace back over the session-only TEST selection.
    if(id=="tool.select")return ActivateLegacy(ShipyardBuilderCommand::ToolSelect);
    if(id=="tool.move")return ActivateLegacy(ShipyardBuilderCommand::ToolMove);
    if(id=="tool.rotate")return ActivateLegacy(ShipyardBuilderCommand::ToolRotate);
    if(id=="tool.scale")return ActivateLegacy(ShipyardBuilderCommand::ToolScale);
    return false;
}

ShipyardCommandResult ShipyardProfessionalController::Execute(std::string_view id,int value){
    if(!builder_)return {false,false,"Shipyard runtime is not bound"};
    ShipyardCommandContext c;c.document=&migration_.document;c.session=&migration_.session;c.history=&migration_.history;c.value=value;
    c.runtimeExecutor=[this](std::string_view rid,int v){return ExecuteRuntime(rid,v);};
    auto result=commands_.Execute(id,c);
    if(result.handled)ApplySessionToLegacy(id);
    return result;
}

} // namespace subspace
