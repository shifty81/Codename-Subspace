#include "content/ShipyardMaterialAuditSystem.h"

namespace subspace {
namespace {
void Add(ShipyardMaterialAuditReport& r,ShipyardMaterialAuditIssueKind k,const std::string& material,const std::string& message,bool blocking=false){
    r.issues.push_back({k,material,message,blocking});
}
}

const char* ShipyardMaterialAuditSystem::IssueName(ShipyardMaterialAuditIssueKind k){
    switch(k){
    case ShipyardMaterialAuditIssueKind::NoMaterialSlots:return "NO_MATERIAL_SLOTS";
    case ShipyardMaterialAuditIssueKind::ReferencedMtlUnresolved:return "REFERENCED_MTL_UNRESOLVED";
    case ShipyardMaterialAuditIssueKind::MaterialSlotUnresolved:return "MATERIAL_SLOT_UNRESOLVED";
    case ShipyardMaterialAuditIssueKind::MissingBaseColorTexture:return "MISSING_BASE_COLOR_TEXTURE";
    case ShipyardMaterialAuditIssueKind::MissingNormalTexture:return "MISSING_NORMAL_TEXTURE";
    case ShipyardMaterialAuditIssueKind::MissingMetallicTexture:return "MISSING_METALLIC_TEXTURE";
    case ShipyardMaterialAuditIssueKind::MissingRoughnessTexture:return "MISSING_ROUGHNESS_TEXTURE";
    case ShipyardMaterialAuditIssueKind::MissingTexcoords:return "MISSING_TEXCOORDS";
    case ShipyardMaterialAuditIssueKind::MissingNormals:return "MISSING_NORMALS";
    }
    return "UNKNOWN";
}

ShipyardMaterialAuditReport ShipyardMaterialAuditSystem::Audit(const ShipyardObjCertification& m){
    ShipyardMaterialAuditReport r;r.sourceName=m.sourceName;r.materialSlots=m.materials.size();
    if(!m.hasNormals)Add(r,ShipyardMaterialAuditIssueKind::MissingNormals,{},"Source mesh has no normals; regenerate/import normals before production use.",true);
    if(!m.hasTexcoords)Add(r,ShipyardMaterialAuditIssueKind::MissingTexcoords,{},"Source mesh has no texture coordinates; use UV generation or governed triplanar fallback.",false);

    if(m.materials.empty()){
        r.semanticFallbackRecommended=true;
        if(!m.materialLibraries.empty()){
            Add(r,ShipyardMaterialAuditIssueKind::ReferencedMtlUnresolved,{},"OBJ references material libraries but no material slots resolved.",true);
            r.certification=KitbashMaterialCertification::BrokenDependency;
        }else{
            Add(r,ShipyardMaterialAuditIssueKind::NoMaterialSlots,{},"No authored material slots; semantic material fallback is required.",false);
            r.certification=KitbashMaterialCertification::NormalizedFallback;
        }
        return r;
    }

    bool anyBlocking=false,anyReview=false,anyFallback=false;
    for(const auto& mat:m.materials){
        if(mat.resolvedFromMtl)++r.resolvedSlots;
        else {Add(r,ShipyardMaterialAuditIssueKind::MaterialSlotUnresolved,mat.name,"Material slot did not resolve through its MTL authority.",true);anyBlocking=true;}
        if(mat.hasBaseColorTexture)++r.texturedSlots;
        else {Add(r,ShipyardMaterialAuditIssueKind::MissingBaseColorTexture,mat.name,"No base-color texture; use semantic/faction paint fallback or assign a governed texture.");anyFallback=true;}
        if(!mat.hasNormalTexture){Add(r,ShipyardMaterialAuditIssueKind::MissingNormalTexture,mat.name,"No normal texture; detail-normal fallback may be used.");anyReview=true;}
        if(!mat.hasMetallicTexture){Add(r,ShipyardMaterialAuditIssueKind::MissingMetallicTexture,mat.name,"No metallic map; material-channel scalar fallback will be used.");anyReview=true;}
        if(!mat.hasRoughnessTexture){Add(r,ShipyardMaterialAuditIssueKind::MissingRoughnessTexture,mat.name,"No roughness map; finish preset roughness will be used.");anyReview=true;}
    }
    if(!m.hasNormals)anyBlocking=true;
    if(!m.hasTexcoords)anyReview=true;
    r.semanticFallbackRecommended=anyFallback||r.texturedSlots<r.materialSlots;
    if(anyBlocking)r.certification=KitbashMaterialCertification::BrokenDependency;
    else if(anyReview)r.certification=KitbashMaterialCertification::ReviewRequired;
    else if(anyFallback)r.certification=KitbashMaterialCertification::NormalizedFallback;
    else r.certification=KitbashMaterialCertification::Complete;
    return r;
}

ShipyardMaterialCorpusAudit ShipyardMaterialAuditSystem::AuditCorpus(const std::vector<ShipyardObjCertification>& modules){
    ShipyardMaterialCorpusAudit out;out.modules=modules.size();out.reports.reserve(modules.size());
    for(const auto& m:modules){auto r=Audit(m);switch(r.certification){case KitbashMaterialCertification::Complete:++out.complete;break;case KitbashMaterialCertification::NormalizedFallback:++out.normalizedFallback;break;case KitbashMaterialCertification::ReviewRequired:++out.reviewRequired;break;case KitbashMaterialCertification::BrokenDependency:++out.brokenDependency;break;}out.reports.push_back(std::move(r));}
    return out;
}

} // namespace subspace
