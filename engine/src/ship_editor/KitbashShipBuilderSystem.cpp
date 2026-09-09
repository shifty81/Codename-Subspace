#include "ship_editor/KitbashShipBuilderSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

const ShipyardAssemblySocket* FindSocket(const ShipyardModuleRecord& record, const std::string& name) {
    for (const auto& socket : record.sockets) if (socket.name == name) return &socket;
    return nullptr;
}

bool SocketOccupied(const ProceduralShipVisualRecipe& recipe, std::size_t parentIndex, const std::string& socketName) {
    return std::any_of(recipe.attachments.begin(), recipe.attachments.end(), [&](const ShipVisualAttachment& edge) {
        return edge.parentModuleIndex == parentIndex && edge.parentSocket == socketName;
    });
}

SpaceMaterialKind MaterialFor(const ShipyardModuleRecord& record) {
    switch (record.partRole) {
    case ShipyardPartRole::Cockpit: return SpaceMaterialKind::Canopy;
    case ShipyardPartRole::EngineHousing:
    case ShipyardPartRole::EngineMount:
    case ShipyardPartRole::MainEngine:
    case ShipyardPartRole::RcsThruster: return SpaceMaterialKind::EngineHousing;
    default: return SpaceMaterialKind::ShipHull;
    }
}

float RootScale(const ShipyardModuleRecord& root) {
    switch (root.size) {
    case ShipyardModuleSize::XS: return 0.78f;
    case ShipyardModuleSize::S: return 0.74f;
    case ShipyardModuleSize::M: return 0.68f;
    case ShipyardModuleSize::L: return 0.62f;
    case ShipyardModuleSize::XL: return 0.56f;
    }
    return 0.68f;
}

KitbashShipBuilderSnapshot Snapshot(const KitbashShipBuilderState& state) {
    KitbashShipBuilderSnapshot s; s.blueprint=state.blueprint; s.equipmentSlots=state.equipmentSlots;
    s.appearance=state.appearance; s.selectedModuleIndex=state.selectedModuleIndex; return s;
}
void Restore(KitbashShipBuilderState& state,const KitbashShipBuilderSnapshot& s) {
    state.blueprint=s.blueprint; state.equipmentSlots=s.equipmentSlots; state.appearance=s.appearance;
    state.selectedModuleIndex=std::min(s.selectedModuleIndex,state.blueprint.modules.empty()?std::size_t{0}:state.blueprint.modules.size()-1);
    state.dragging=false; state.draggingModuleId.clear(); state.preview={}; state.dirty=true;
}

std::vector<ShipEquipmentSlot> RebuildSlotsPreserving(const ProceduralShipVisualRecipe& recipe,
                                                       const std::vector<ShipyardModuleRecord>& catalog,
                                                       const std::vector<ShipEquipmentSlot>& previous) {
    auto rebuilt=ShipyardEquipmentSystem::BuildSlots(recipe,catalog);
    for(auto& slot:rebuilt){
        const auto it=std::find_if(previous.begin(),previous.end(),[&](const ShipEquipmentSlot& old){return old.slotId==slot.slotId;});
        if(it!=previous.end()){slot.installedItemInstanceId=it->installedItemInstanceId;slot.installedDefinitionId=it->installedDefinitionId;}
    }
    return rebuilt;
}
void PushUndo(KitbashShipBuilderState& state) {
    state.undoStack.push_back(Snapshot(state)); if(state.undoStack.size()>100)state.undoStack.erase(state.undoStack.begin()); state.redoStack.clear();
}

} // namespace

std::vector<KitbashShipBuilderPaletteItem> KitbashShipBuilderSystem::BuildPalette(
    const std::vector<ShipyardModuleRecord>& catalog) {
    std::vector<KitbashShipBuilderPaletteItem> result;
    result.reserve(catalog.size());
    for (const auto& record : catalog) {
        result.push_back({record.source.moduleId,
                          ShipyardPartTaxonomySystem::DisplayName(record.source.moduleId),
                          record.builderCategory, record.partRole, record.size,
                          record.primaryHull, record.generatorEligible, record.surfaceOnly,
                          record.functional, record.mirrorPreferred});
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.category != b.category) return static_cast<int>(a.category) < static_cast<int>(b.category);
        if (a.role != b.role) return static_cast<int>(a.role) < static_cast<int>(b.role);
        return a.label < b.label;
    });
    return result;
}

KitbashShipBuilderState KitbashShipBuilderSystem::CreateStarter(
    const std::vector<ShipyardModuleRecord>& catalog, const std::string& role) {
    KitbashShipBuilderState state;
    state.palette = BuildPalette(catalog);
    state.blueprint.recipeId = "player_shipyard_blueprint";
    state.blueprint.role = role;
    state.blueprint.sourceFamily = "SHIPYARD_V07_CC0";
    state.blueprint.manufacturerFamily = "PLAYER_SHIPYARD";
    state.blueprint.cockpitFamily = "SHIPYARD_AUTHORED";
    state.blueprint.widthScale = state.blueprint.lengthScale = 1.0f;

    const ShipyardModuleRecord* root = nullptr;
    for (const auto& record : catalog) {
        if (!record.primaryHull) continue;
        if (!root || static_cast<int>(record.size) < static_cast<int>(root->size)) root = &record;
    }
    if (root) {
        VisualModulePlacement placement;
        placement.moduleId = root->source.moduleId;
        placement.scaleX = placement.scaleY = placement.scaleZ = RootScale(*root);
        placement.material = SpaceMaterialKind::ShipHull;
        state.blueprint.modules.push_back(placement);
        state.blueprint.anchors.push_back({"PRIMARY_HULL", root->source.moduleId, {0,0,0}, false});
        state.status = "PRIMARY HULL READY - DRAG A PART TO A SOCKET";
        state.equipmentSlots = ShipyardEquipmentSystem::BuildSlots(state.blueprint, catalog);
    } else {
        state.status = "NO CERTIFIED PRIMARY HULLS";
    }
    return state;
}

const ShipyardModuleRecord* KitbashShipBuilderSystem::FindRecord(
    const std::vector<ShipyardModuleRecord>& catalog, const std::string& moduleId) {
    const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const auto& r) { return r.source.moduleId == moduleId; });
    return it == catalog.end() ? nullptr : &*it;
}

bool KitbashShipBuilderSystem::BeginDrag(KitbashShipBuilderState& state,
                                         const std::vector<ShipyardModuleRecord>& catalog,
                                         const std::string& moduleId,
                                         std::string* error) {
    const auto* record = FindRecord(catalog, moduleId);
    if (!record) { if (error) *error = "part is not in certified Shipyard catalog"; return false; }
    if (record->primaryHull && !state.blueprint.modules.empty()) {
        if (error) *error = "normal blueprints permit exactly one primary hull";
        state.status = "SECOND PRIMARY HULL REJECTED - USE AN ADAPTER/STRUCTURAL PART";
        return false;
    }
    state.dragging = true;
    state.draggingModuleId = moduleId;
    state.preview = {};
    state.status = "DRAGGING " + ShipyardPartTaxonomySystem::DisplayName(moduleId);
    return true;
}

KitbashShipBuilderPreview KitbashShipBuilderSystem::PreviewDrop(
    const KitbashShipBuilderState& state,
    const std::vector<ShipyardModuleRecord>& catalog,
    std::size_t parentModuleIndex,
    const std::string& parentSocketName) {
    KitbashShipBuilderPreview preview;
    preview.parentModuleIndex = parentModuleIndex;
    preview.parentSocket = parentSocketName;
    if (!state.dragging || state.draggingModuleId.empty()) { preview.reason = "no active dragged part"; return preview; }
    if (parentModuleIndex >= state.blueprint.modules.size()) { preview.reason = "target module is outside blueprint"; return preview; }
    if (SocketOccupied(state.blueprint, parentModuleIndex, parentSocketName)) { preview.reason = "target socket is occupied"; return preview; }

    const auto* parentRecord = FindRecord(catalog, state.blueprint.modules[parentModuleIndex].moduleId);
    const auto* childRecord = FindRecord(catalog, state.draggingModuleId);
    if (!parentRecord || !childRecord) { preview.reason = "missing catalog record"; return preview; }
    if (childRecord->primaryHull && !state.blueprint.modules.empty()) { preview.reason = "second primary hull is forbidden"; return preview; }
    const auto* parentSocket = FindSocket(*parentRecord, parentSocketName);
    if (!parentSocket) { preview.reason = "target socket no longer exists"; return preview; }

    const ShipyardAssemblySocket* childSocket = nullptr;
    for (const auto& socket : childRecord->sockets) {
        if (ShipyardModuleSystem::CanMate(parentSocket->type, socket.type)) { childSocket = &socket; break; }
    }
    if (!childSocket) { preview.reason = "part has no compatible mating socket"; return preview; }
    if (!ShipyardModuleSystem::SizeCompatible(parentRecord->size, childRecord->size) && childRecord->partRole != ShipyardPartRole::HullAdapter) {
        preview.reason = "part size is incompatible - use an adapter"; return preview;
    }

    const float childScale = state.blueprint.modules.front().scaleX;
    if (!ShipyardModuleSystem::BuildAttachmentPlacement(*parentRecord, state.blueprint.modules[parentModuleIndex],
                                                        parentSocketName, *childRecord, childSocket->name,
                                                        childScale, MaterialFor(*childRecord),
                                                        preview.placement, preview.attachment, 0.75f)) {
        preview.reason = "mating surfaces failed contact certification";
        return preview;
    }
    preview.attachment.parentModuleIndex = parentModuleIndex;
    preview.attachment.childModuleIndex = state.blueprint.modules.size();
    preview.childSocket = childSocket->name;
    preview.valid = true;
    preview.reason = "VALID SNAP";
    return preview;
}

bool KitbashShipBuilderSystem::CommitDrop(KitbashShipBuilderState& state,
                                          const std::vector<ShipyardModuleRecord>& catalog,
                                          const KitbashShipBuilderPreview& preview,
                                          std::string* error) {
    if (!preview.valid || !state.dragging) { if (error) *error = preview.reason.empty()?"invalid drop preview":preview.reason; return false; }
    const auto old = state.blueprint;
    const auto oldSlots = state.equipmentSlots;
    PushUndo(state);
    state.blueprint.modules.push_back(preview.placement);
    auto edge = preview.attachment;
    edge.childModuleIndex = state.blueprint.modules.size() - 1;
    state.blueprint.attachments.push_back(edge);
    state.blueprint.anchors.push_back({"PLAYER_PART_" + std::to_string(edge.childModuleIndex), preview.placement.moduleId,
                                       {preview.placement.x, preview.placement.y, preview.placement.z}, true});
    std::string validation;
    if (!ValidateBlueprint(state, catalog, &validation)) {
        state.blueprint = old;
        state.equipmentSlots = oldSlots;
        if (!state.undoStack.empty()) state.undoStack.pop_back();
        if (error) *error = validation;
        state.status = "DROP REJECTED - " + validation;
        return false;
    }
    state.dragging = false;
    state.draggingModuleId.clear();
    state.preview = {};
    state.selectedModuleIndex = state.blueprint.modules.size()-1;
    state.equipmentSlots = RebuildSlotsPreserving(state.blueprint, catalog, oldSlots);
    state.dirty = true;
    state.status = "PART ATTACHED - BLUEPRINT VALID";
    return true;
}

void KitbashShipBuilderSystem::CancelDrag(KitbashShipBuilderState& state) {
    state.dragging = false;
    state.draggingModuleId.clear();
    state.preview = {};
    state.status = "DRAG CANCELLED";
}

void KitbashShipBuilderSystem::SelectModule(KitbashShipBuilderState& state, std::size_t moduleIndex) {
    if(moduleIndex<state.blueprint.modules.size()){state.selectedModuleIndex=moduleIndex;state.status="MODULE SELECTED";}
}

bool KitbashShipBuilderSystem::Undo(KitbashShipBuilderState& state) {
    if(state.undoStack.empty())return false;state.redoStack.push_back(Snapshot(state));const auto snap=state.undoStack.back();state.undoStack.pop_back();Restore(state,snap);state.status="UNDO";return true;
}

bool KitbashShipBuilderSystem::Redo(KitbashShipBuilderState& state) {
    if(state.redoStack.empty())return false;state.undoStack.push_back(Snapshot(state));const auto snap=state.redoStack.back();state.redoStack.pop_back();Restore(state,snap);state.status="REDO";return true;
}

bool KitbashShipBuilderSystem::ValidateBlueprint(const KitbashShipBuilderState& state,
                                                 const std::vector<ShipyardModuleRecord>& catalog,
                                                 std::string* error) {
    if (state.blueprint.modules.empty()) { if (error) *error = "blueprint has no primary hull"; return false; }
    std::size_t hulls = 0;
    for (const auto& placement : state.blueprint.modules) {
        const auto* record = FindRecord(catalog, placement.moduleId);
        if (!record) { if (error) *error = "blueprint references unknown module"; return false; }
        if (record->primaryHull) ++hulls;
    }
    if (hulls != 1) { if (error) *error = "blueprint must contain exactly one primary hull"; return false; }
    return ShipyardModuleSystem::ValidateAssemblyGraph(state.blueprint, error);
}

std::vector<std::size_t> KitbashShipBuilderSystem::FilterPalette(const KitbashShipBuilderState& state) {
    std::vector<std::size_t> indices;
    for (std::size_t i=0;i<state.palette.size();++i) if (state.palette[i].category==state.category) indices.push_back(i);
    return indices;
}

} // namespace subspace
