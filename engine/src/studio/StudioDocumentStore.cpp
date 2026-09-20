#include "studio/StudioDocumentStore.h"
#include "ship_editor/ShipyardDocumentStartupSystem.h"
#include "studio/StudioProjectPaths.h"
#include "studio/StudioRecoveryPathPolicy.h"
#include "studio/StudioUnsavedWorkPolicy.h"
#include "studio/StudioModelDocumentCodec.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace subspace {
namespace {
std::filesystem::path NewDraftPath() {
    const auto base=StudioProjectPaths::Blueprints();
    if(base.empty())return {}; // never write into an unrecognized directory
    const auto tick=std::chrono::system_clock::now().time_since_epoch().count();
    for (unsigned n=0;n<1000;++n) {
        const auto candidate=base/("studio_draft_"+std::to_string(tick)+"_"+std::to_string(n)+".subspace_ship");
        std::error_code ec;
        if (!std::filesystem::exists(candidate,ec) && !ec) return candidate;
    }
    return {}; // never silently overwrite a collision
}
}
bool StudioDocumentStore::Open(const std::filesystem::path& source,ShipyardBuilderSystem& builder,std::string& error) {
    const auto& state=builder.Model();
    if(StudioUnsavedWorkPolicy::HasUnsaved({state.dirty,state.socketOverridesDirty,
            state.definitionOverridesDirty,builder.HasUnsavedModeling(),
            state.interiorStructure.dirty})){
        error="Unsaved changes in blueprint or authoring overrides: save before Open";return false;
    }
    if(StudioModelDocumentCodec::IsStudioModelPath(source)){
        ShipyardModelingState modeling;if(!StudioModelDocumentCodec::Load(source,modeling,error))return false;
        const auto catalog=builder.Model().catalog;const auto layout=builder.Model().dockWorkspace;
        builder.Initialize(catalog,ShipyardDocumentStartupSystem::EmptyDocument());builder.MutableDockWorkspace()=layout;
        builder.SetLiveApplyEnabled(false,true);builder.SetModelingState(modeling);path_=source;builder.MarkModelingSaved(path_.filename().string());return true;
    }
    ShipBlueprintDocument document;
    if(!ShipBlueprintLibrarySystem::Load(source.string(),document,&error))return false;
    for(const auto& placed:document.recipe.modules) {
            bool present=false;
            for(const auto& record:builder.Model().catalog)
                if(record.source.moduleId==placed.moduleId){present=true;break;}
            if(!present){error="Blueprint references an unavailable module: "+placed.moduleId;return false;}
    }
    const auto catalog=builder.Model().catalog;
    const auto layout=builder.Model().dockWorkspace;
    builder.Initialize(catalog,document.recipe);
    builder.MutableDockWorkspace()=layout;
    builder.SetLiveApplyEnabled(false,true);
    builder.SetAppearance(document.appearance);
    path_=source;
    builder.MarkSaved(path_.filename().string());
    return true;
}
bool StudioDocumentStore::SaveAs(const std::filesystem::path& destination,ShipyardBuilderSystem& builder,std::string& error){
    if(destination.empty()){error="Choose a Studio document path";return false;}
    if(StudioModelDocumentCodec::IsStudioModelPath(destination)){
        if(builder.Model().modeling.recipe.primitives.empty()){error="Model document has no geometry";return false;}
        const bool overwrite=destination==path_;
        if(!StudioModelDocumentCodec::Save(destination,builder.Model().modeling,overwrite,error))return false;
        path_=destination;builder.MarkModelingSaved(path_.filename().string());return true;
    }
    if(destination.extension()!=".subspace_ship") {error="Studio saves .subspace_ship or .subspace_studio documents";return false;}
    if(builder.Recipe().modules.empty()){error="Empty ship document: add a module before saving";return false;}
    std::error_code ec;
    const auto parent=destination.parent_path();
    if(!parent.empty())std::filesystem::create_directories(parent,ec);
    if(ec){error="Cannot create blueprint directory: "+ec.message();return false;}
    // Explicit Save As is never allowed to overwrite a different document.
    if(destination!=path_ && std::filesystem::exists(destination,ec)){
        error="Save As target exists: choose a different name";return false;
    }
    if(ec){error="Cannot inspect target: "+ec.message();return false;}
    ShipBlueprintDocument doc;
    doc.recipe=builder.Recipe();doc.appearance=builder.Appearance();
    doc.name=destination.stem().string();doc.author="PLAYER";
    doc.equipmentSlots=ShipyardEquipmentSystem::BuildSlots(doc.recipe,builder.Model().catalog);
    doc.tags={"STUDIO_AUTHORED","DRAFT_REVIEWABLE"};
    doc.blueprintId=ShipBlueprintLibrarySystem::CanonicalId(doc);
    doc.recipe.recipeId=doc.blueprintId;
    const auto pending=std::filesystem::path(destination.string()+".studio_pending");
    const auto recovery=std::filesystem::path(destination.string()+".studio_recovery");
    if(std::filesystem::exists(pending,ec)||std::filesystem::exists(recovery,ec)||ec){
        error="Pending/recovery file exists; review before saving again";return false;
    }
    if(!ShipBlueprintLibrarySystem::Save(doc,pending.string(),&error)){
        std::filesystem::remove(pending,ec);return false;
    }
    // Verify actual persisted bytes through the canonical loader before moving.
    ShipBlueprintDocument verified;
    if(!ShipBlueprintLibrarySystem::Load(pending.string(),verified,&error) ||
       verified.blueprintId!=doc.blueprintId ||
       verified.recipe.modules.size()!=doc.recipe.modules.size() ||
       verified.recipe.attachments.size()!=doc.recipe.attachments.size() ||
       verified.appearance.decals.size()!=doc.appearance.decals.size()){
        if(error.empty())error="Blueprint round-trip validation failed";
        std::filesystem::remove(pending,ec);return false;
    }
    const bool replacing=std::filesystem::exists(destination,ec);
    if(ec){error="Target inspection failed: "+ec.message();return false;}
    if(replacing){
        std::filesystem::rename(destination,recovery,ec);
        if(ec){error="Cannot stage existing blueprint: "+ec.message();return false;}
    }
    std::filesystem::rename(pending,destination,ec);
    if(ec){
        error="Cannot finalize blueprint: "+ec.message();
        if(replacing){std::error_code rollback;std::filesystem::rename(recovery,destination,rollback);
            if(rollback)error+="; recovery preserved at "+recovery.string();}
        return false;
    }
    if(replacing){std::filesystem::remove(recovery,ec);if(ec)error="Saved; previous version remains for review at "+recovery.string();}
    path_=destination;builder.MarkSaved(path_.filename().string());
    return true;
}
bool StudioDocumentStore::Save(ShipyardBuilderSystem& builder,std::string& error){
    std::filesystem::path destination=path_;
    if(destination.empty()&&!builder.Model().modeling.recipe.primitives.empty()&&builder.Recipe().modules.empty()){
        const auto base=StudioProjectPaths::Blueprints();if(base.empty()){error="Unable to locate Studio document directory";return false;}
        const auto tick=std::chrono::system_clock::now().time_since_epoch().count();destination=base/("studio_model_"+std::to_string(tick)+".subspace_studio");
    }else if(destination.empty())destination=NewDraftPath();
    if(destination.empty()){error="Unable to reserve unique draft filename";return false;}
    return SaveAs(destination,builder,error);
}
bool StudioDocumentStore::SaveModelExitRecovery(ShipyardBuilderSystem& builder,
                                                 std::filesystem::path& recovered,std::string& error){
    recovered.clear();error.clear();
    const auto& modeling=builder.Model().modeling;
    if(modeling.recipe.primitives.empty()&&modeling.recipe.sourceAssetId.empty()){
        error="No model geometry to recover";return false;
    }
    const auto base=StudioProjectPaths::Blueprints();
    if(base.empty()){error="Project root missing: model recovery cannot choose a safe destination";return false;}
    const auto tick=std::chrono::system_clock::now().time_since_epoch().count();
    for(unsigned i=0;i<1000;++i){
        const auto candidate=base/"recovery"/("studio_model_recovery_"+std::to_string(tick)+"_"+std::to_string(i)+".subspace_studio");
        std::error_code ec;
        if(std::filesystem::exists(candidate,ec)||ec)continue;
        if(!StudioModelDocumentCodec::Save(candidate,modeling,false,error))return false;
        recovered=candidate;return true; // codec verifies round-trip before returning
    }
    error="Could not reserve a unique model recovery filename";return false;
}
bool StudioDocumentStore::SaveExitRecovery(ShipyardBuilderSystem& builder,
                                            std::filesystem::path& recovered,std::string& error){
    recovered.clear();error.clear();
    if(builder.Recipe().modules.empty())return SaveModelExitRecovery(builder,recovered,error);
    const auto base=StudioProjectPaths::Blueprints();
    if(base.empty()){error="Project root missing: recovery cannot choose a safe destination";return false;}
    const auto tick=std::chrono::system_clock::now().time_since_epoch().count();
    for(unsigned i=0;i<1000;++i){
        const auto candidate=StudioRecoveryPathPolicy::Candidate(base,tick,i);
        std::error_code ec;
        if(std::filesystem::exists(candidate,ec)||ec)continue;
        const auto original=path_;
        const bool saved=SaveAs(candidate,builder,error);
        path_=original; // recovery must NEVER switch or replace the current document
        if(saved){recovered=candidate;return true;}
        return false; // failed writes are not retried over ambiguous pending files
    }
    error="Could not reserve a unique recovery filename";
    return false;
}
bool StudioDocumentStore::New(ShipyardBuilderSystem& builder,std::string& error){
    const auto& state=builder.Model();
    if(StudioUnsavedWorkPolicy::HasUnsaved({state.dirty,state.socketOverridesDirty,
            state.definitionOverridesDirty,builder.HasUnsavedModeling(),
            state.interiorStructure.dirty})){
        error="Unsaved changes in blueprint or authoring overrides: save before New";return false;
    }
    const auto catalog=builder.Model().catalog;
    const auto layout=builder.Model().dockWorkspace;
    builder.Initialize(catalog,ShipyardDocumentStartupSystem::EmptyDocument());
    builder.MutableDockWorkspace()=layout;
    builder.SetLiveApplyEnabled(false,true);
    path_.clear();return true;
}
} // namespace subspace
