#include "ship_editor/ShipyardBuilderMigrationSystem.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {

bool SamePlacement(const VisualModulePlacement& a, const VisualModulePlacement& b) {
    constexpr float e=.0001f;
    return a.moduleId==b.moduleId &&
           std::fabs(a.x-b.x)<=e && std::fabs(a.y-b.y)<=e && std::fabs(a.z-b.z)<=e &&
           std::fabs(a.scaleX-b.scaleX)<=e && std::fabs(a.scaleY-b.scaleY)<=e && std::fabs(a.scaleZ-b.scaleZ)<=e &&
           std::fabs(a.yawDegrees-b.yawDegrees)<=e && std::fabs(a.pitchDegrees-b.pitchDegrees)<=e &&
           std::fabs(a.rollDegrees-b.rollDegrees)<=e &&
           a.mirrorX==b.mirrorX && a.mirrorY==b.mirrorY && a.mirrorZ==b.mirrorZ;
}

bool SameAttachment(const VisualAssemblyAttachment& a, const VisualAssemblyAttachment& b) {
    return a.parentModuleIndex==b.parentModuleIndex &&
           a.childModuleIndex==b.childModuleIndex &&
           a.parentSocket==b.parentSocket &&
           a.childSocket==b.childSocket;
}

ShipyardObjectId Allocate(ShipyardDocument& d, ShipyardObjectKind kind) {
    return ShipyardStableIdSystem::Local(kind,d.documentId,d.nextLocalOrdinal++);
}

std::uint64_t AuthoredFingerprint(const ShipyardBuilderRuntimeModel& legacy) {
    std::ostringstream ss;
    const auto& r=legacy.recipe;
    ss<<r.recipeId<<'|'<<r.role<<'|'<<r.seed<<'|'<<r.sourceFamily<<'|'<<r.factionId<<'|'<<r.shipClassId<<'|'<<r.hullFamilyId<<'|'<<r.roleVariantId<<'|';
    for(const auto& p:r.modules)ss<<p.moduleId<<':'<<p.x<<','<<p.y<<','<<p.z<<','<<p.scaleX<<','<<p.scaleY<<','<<p.scaleZ<<','<<p.yawDegrees<<','<<p.pitchDegrees<<','<<p.rollDegrees<<','<<p.mirrorX<<p.mirrorY<<p.mirrorZ<<';';
    ss<<'|';
    for(const auto& a:r.attachments)ss<<a.parentModuleIndex<<','<<a.childModuleIndex<<','<<a.parentSocket<<','<<a.childSocket<<';';
    const auto paint=[&](const ShipPaintLayer& p){ss<<p.id<<','<<p.r<<','<<p.g<<','<<p.b<<','<<p.a<<','<<p.metallic<<','<<p.roughness<<';';};
    paint(legacy.appearance.primary);paint(legacy.appearance.secondary);paint(legacy.appearance.trim);ss<<legacy.appearance.factoryWear<<'|';
    for(const auto& d:legacy.appearance.decals)ss<<d.id<<','<<d.decalAsset<<','<<d.moduleIndex<<','<<d.u<<','<<d.v<<','<<d.scale<<','<<d.rotationDegrees<<','<<d.opacity<<','<<d.mirror<<';';
    return ShipyardStableIdSystem::Hash64(ss.str());
}

} // namespace

ShipyardBuilderMigrationState ShipyardBuilderMigrationSystem::Bind(const ShipyardBuilderRuntimeModel& legacy,
                                                                   std::string persistentShipId) {
    ShipyardBuilderMigrationState out;
    out.document=ShipyardDocumentSystem::Create(legacy.recipe,legacy.appearance,std::move(persistentShipId));
    out.session=ShipyardSessionSystem::Create(out.document.documentId);
    out.authoredFingerprint=AuthoredFingerprint(legacy);
    out.bound=true;
    SyncSessionFromLegacy(legacy,out);
    SyncSelectionFromLegacy(legacy,out);
    return out;
}

void ShipyardBuilderMigrationSystem::PullFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                                                     ShipyardBuilderMigrationState& state) {
    if(!state.bound || !state.document.documentId.Valid()){
        state=Bind(legacy,state.document.persistentShipId);
        return;
    }

    const auto oldRecipe=state.document.recipe;
    const auto oldModuleIds=state.document.moduleIds;
    const auto oldAttachmentIds=state.document.attachmentIds;

    const auto nextFingerprint=AuthoredFingerprint(legacy);
    const bool authoredChanged=nextFingerprint!=state.authoredFingerprint;
    state.document.recipe=legacy.recipe;
    state.document.appearance=legacy.appearance;

    state.document.moduleIds.clear();
    state.document.moduleIds.reserve(state.document.recipe.modules.size());
    for(std::size_t i=0;i<state.document.recipe.modules.size();++i){
        ShipyardObjectId id{};
        if(i<oldRecipe.modules.size() && i<oldModuleIds.size() &&
           SamePlacement(oldRecipe.modules[i],state.document.recipe.modules[i])) id=oldModuleIds[i];
        if(!id.Valid()) id=Allocate(state.document,ShipyardObjectKind::Module);
        state.document.moduleIds.push_back(id);
    }

    state.document.attachmentIds.clear();
    state.document.attachmentIds.reserve(state.document.recipe.attachments.size());
    for(std::size_t i=0;i<state.document.recipe.attachments.size();++i){
        ShipyardObjectId id{};
        if(i<oldRecipe.attachments.size() && i<oldAttachmentIds.size() &&
           SameAttachment(oldRecipe.attachments[i],state.document.recipe.attachments[i])) id=oldAttachmentIds[i];
        if(!id.Valid()) id=Allocate(state.document,ShipyardObjectKind::Attachment);
        state.document.attachmentIds.push_back(id);
    }

    if(authoredChanged)ShipyardDocumentSystem::MarkModified(state.document);
    state.authoredFingerprint=nextFingerprint;
    SyncSessionFromLegacy(legacy,state);
    SyncSelectionFromLegacy(legacy,state);
}

void ShipyardBuilderMigrationSystem::SyncSessionFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                                                            ShipyardBuilderMigrationState& state) {
    state.session.documentId=state.document.documentId;
    state.session.workspace=legacy.workspaceMode;
    state.session.activeTool=legacy.transformTool;
    state.session.transformSpace=legacy.transformSpace;
    state.session.transformSnap=legacy.transformSnap;
    state.session.status=legacy.status;
}

void ShipyardBuilderMigrationSystem::SyncSelectionFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                                                              ShipyardBuilderMigrationState& state) {
    if(legacy.recipe.modules.empty() || state.document.moduleIds.empty()){
        ShipyardSelectionSystem::Clear(state.session.selection);
        return;
    }
    const auto index=std::min(legacy.selectedPlacedModule,state.document.moduleIds.size()-1);
    ShipyardSelectionSystem::SelectSingle(state.session.selection,state.document.moduleIds[index]);
}

bool ShipyardBuilderMigrationSystem::Validate(const ShipyardBuilderRuntimeModel& legacy,
                                               const ShipyardBuilderMigrationState& state,
                                               std::string* error) {
    if(!state.bound){if(error)*error="Migration state is not bound";return false;}
    const auto identity=ShipyardDocumentSystem::ValidateIdentity(state.document);
    if(!identity.valid){if(error)*error=identity.errors.empty()?"Document identity invalid":identity.errors.front();return false;}
    if(legacy.recipe.modules.size()!=state.document.recipe.modules.size() ||
       legacy.recipe.attachments.size()!=state.document.recipe.attachments.size()){
        if(error)*error="Legacy/document topology mismatch";
        return false;
    }
    if(state.session.documentId!=state.document.documentId){if(error)*error="Session/document identity mismatch";return false;}
    return true;
}

} // namespace subspace
