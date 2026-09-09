#include "content/KitbashReviewCatalogSystem.h"

#include <algorithm>

namespace subspace {
namespace {
void AddUnique(std::vector<std::string>&v,const std::string&s){if(std::find(v.begin(),v.end(),s)==v.end())v.push_back(s);}
}

std::vector<KitbashReviewItem> KitbashReviewCatalogSystem::BuildReviewQueue(const std::vector<ShipyardModuleRecord>&catalog,KitbashMaterialCertification material){
    std::vector<KitbashReviewItem> out;
    const auto prop=ShipPcgRuntimeClosureSystem::AuditPropulsionCatalog(catalog);
    for(const auto& rec:catalog){
        const auto profile=UniversalKitbashAuthority::BuildProfile(rec,material);
        if(rec.semantic==ShipyardModuleSemantic::Component||rec.partRole==ShipyardPartRole::ReviewRequired)out.push_back({rec.source.moduleId,KitbashReviewKind::Classification,"Semantic classification requires review.",.35f,true});
        if(rec.sockets.empty()&&!rec.surfaceOnly)out.push_back({rec.source.moduleId,KitbashReviewKind::Socket,"No certified mating socket is available.",.20f,true});
        if(!profile.hasKnownOrientation)out.push_back({rec.source.moduleId,KitbashReviewKind::Orientation,"Forward/mount orientation is not confidently known.",rec.mountFaceConfidence,true});
        if(material==KitbashMaterialCertification::ReviewRequired||material==KitbashMaterialCertification::BrokenDependency)out.push_back({rec.source.moduleId,KitbashReviewKind::Material,"Material dependency or semantic surface mapping requires review.",material==KitbashMaterialCertification::BrokenDependency?0.0f:.5f,true});
        if(profile.morph.policy==KitbashScalingPolicy::FixedReference&&rec.surfaceOnly)out.push_back({rec.source.moduleId,KitbashReviewKind::Morph,"Surface/detail asset has no safe morph profile.",.55f,false});
        if(!profile.pcgCertified&&rec.generatorEligible)out.push_back({rec.source.moduleId,KitbashReviewKind::PcgCertification,"Generator-eligible asset does not meet canonical PCG certification.",.40f,true});
    }
    for(const auto& p:prop)if(p.requiresReview)out.push_back({p.moduleId,KitbashReviewKind::Propulsion,p.reason,p.confidence,true});
    return out;
}

RuntimeKitbashCatalogs KitbashReviewCatalogSystem::Materialize(const std::vector<ShipyardModuleRecord>&catalog,KitbashMaterialCertification material){
    RuntimeKitbashCatalogs out;
    for(const auto& rec:catalog){
        const auto profile=UniversalKitbashAuthority::BuildProfile(rec,material);
        if(material==KitbashMaterialCertification::BrokenDependency){AddUnique(out.quarantined,rec.source.moduleId);continue;}
        if(!UniversalKitbashAuthority::IsPcgEligible(profile)){AddUnique(out.manualOnly,rec.source.moduleId);continue;}
        for(const auto& role:profile.domainRoles){if(!role.generatorEligible)continue;switch(role.domain){case ConstructionDomain::Ship:AddUnique(out.ship,rec.source.moduleId);break;case ConstructionDomain::Station:AddUnique(out.station,rec.source.moduleId);break;case ConstructionDomain::Planetary:AddUnique(out.planetary,rec.source.moduleId);break;case ConstructionDomain::Weapon:AddUnique(out.weapon,rec.source.moduleId);break;default:break;}}
    }
    return out;
}

KitbashContentCertificationSummary KitbashReviewCatalogSystem::Certify(const std::vector<ShipyardModuleRecord>&catalog,KitbashMaterialCertification material){
    KitbashContentCertificationSummary s;s.total=catalog.size();s.queue=BuildReviewQueue(catalog,material);s.catalogs=Materialize(catalog,material);if(material==KitbashMaterialCertification::NormalizedFallback)s.materialFallback=catalog.size();
    for(const auto& rec:catalog){const auto profile=UniversalKitbashAuthority::BuildProfile(rec,material);if(material==KitbashMaterialCertification::BrokenDependency){++s.quarantined;continue;}if(UniversalKitbashAuthority::IsPcgEligible(profile))++s.certified;else ++s.manualOnly;}
    for(const auto&i:s.queue){if(i.blockingPcg)++s.review;if(i.kind==KitbashReviewKind::Propulsion)++s.propulsionReview;}
    s.fullGateReady=s.quarantined==0&&s.total>0&&!s.catalogs.ship.empty();return s;
}

} // namespace subspace
