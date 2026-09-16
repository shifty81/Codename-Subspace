#include "ship_editor/ShipyardWorkflowSystem.h"

namespace subspace {

const char* ShipyardWorkflowSystem::StageName(ShipyardWorkflowStage stage){
    switch(stage){
    case ShipyardWorkflowStage::Start:return "START";
    case ShipyardWorkflowStage::Assemble:return "ASSEMBLE";
    case ShipyardWorkflowStage::Attachments:return "ATTACHMENTS";
    case ShipyardWorkflowStage::Interior:return "INTERIOR";
    case ShipyardWorkflowStage::Systems:return "SYSTEMS";
    case ShipyardWorkflowStage::Appearance:return "APPEARANCE";
    case ShipyardWorkflowStage::Validate:return "VALIDATE";
    case ShipyardWorkflowStage::TestAndSave:return "TEST / SAVE";
    }
    return "START";
}

std::vector<ShipyardWorkflowStep> ShipyardWorkflowSystem::Build(const ShipyardBuilderRuntimeModel& model){
    const bool hasModules=!model.recipe.modules.empty();
    const bool hasAttachments=!model.recipe.attachments.empty()||model.recipe.modules.size()<=1;
    const bool hasInterior=model.interiorPlan.valid&&!model.interiorPlan.bindings.empty();
    const bool hasPaint=model.appearance.primary.a>0.0f;
    return {
        {ShipyardWorkflowStage::Start,"1. Choose class + role","Pick the intended ship class/role first; generation and module filtering use this envelope.","LAYOUT",hasModules,false},
        {ShipyardWorkflowStage::Assemble,"2. Assemble hull + modules","Drag certified assets into the viewport, place, move, rotate, scale and use symmetry as needed.","G / R / S",hasModules,false},
        {ShipyardWorkflowStage::Attachments,"3. Author attachments","Select a module, edit sockets, detach/reattach, then configure pivots/articulation for moving equipment.","SOCKETS",hasAttachments,!hasModules},
        {ShipyardWorkflowStage::Interior,"4. Build interior","Generate module interior volumes, connect portals, then refine rooms/corridors in the Interior workspace.","INTERIOR",hasInterior,!hasModules},
        {ShipyardWorkflowStage::Systems,"5. Configure ship systems","Verify engines, sensors, equipment, power/heat/cargo/crew and role-specific functional requirements.","SYSTEMS",false,!hasModules},
        {ShipyardWorkflowStage::Appearance,"6. Finish appearance","Resolve materials/textures, paint zones, livery, decals and material warnings.","PAINT",hasPaint,!hasModules},
        {ShipyardWorkflowStage::Validate,"7. Validate","Run structural, class, socket, interior and runtime certification before apply/test.","CHECK",model.validation.valid,!hasModules},
        {ShipyardWorkflowStage::TestAndSave,"8. Test + save blueprint","Exercise moving parts/interiors, save blueprint or draft, then apply only when the authoritative docked workflow permits it.","TEST",false,!hasModules}
    };
}

ShipyardWorkflowStage ShipyardWorkflowSystem::Recommended(const ShipyardBuilderRuntimeModel& model){
    const auto steps=Build(model);for(const auto& step:steps)if(!step.complete&&!step.blocked)return step.stage;return ShipyardWorkflowStage::TestAndSave;
}

} // namespace subspace
