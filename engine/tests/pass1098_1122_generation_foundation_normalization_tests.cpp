#include "generator/GeneratorParitySystem.h"
#include "ships/ShipClassGenerationAuthoritySystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;std::cout<<(ok?"[PASS] ":"[FAIL] ")<<name<<"\n";if(!ok)++failures;}
bool Near(float a,float b,float e=.05f){return std::fabs(a-b)<=e;}

ShipyardModuleRecord Module(const char* id,float halfWidth=1.5f,float halfLength=2.0f,float halfHeight=1.0f){
    ShipyardModuleRecord r;r.source.moduleId=id;r.source.halfWidth=halfWidth;r.source.halfLength=halfLength;r.source.halfHeight=halfHeight;
    r.moduleClass=ShipyardModuleClass::Hull;r.semantic=ShipyardModuleSemantic::HullMid;r.size=ShipyardModuleSize::M;
    r.builderCategory=ShipyardPartCategory::Hull;r.partRole=ShipyardPartRole::PrimaryHull;r.primaryHull=true;r.generatorEligible=true;r.placementRole="STRUCTURAL";
    return r;
}

ProceduralShipVisualRecipe SpanRecipe(const ShipyardModuleRecord& module,int count,float minCenter,float maxCenter){
    ProceduralShipVisualRecipe r;r.recipeId="integrity";
    for(int i=0;i<count;++i){VisualModulePlacement p;p.moduleId=module.source.moduleId;const float t=count<=1?0.5f:static_cast<float>(i)/static_cast<float>(count-1);p.y=minCenter+(maxCenter-minCenter)*t;r.modules.push_back(p);}
    return r;
}
}

int main(){
    std::cout<<"[Pass1098-1122 Generation Foundation Emergency Normalization]\n";
    using U=UniversalSizeClass;

    GeneratorParityRequest request;request.domain=GeneratorDomain::Ship;request.seed=424242;request.profileId="PLAYER_HULL_FAMILY_1";request.role="COMBAT";request.shipClass=ShipClass::Frigate;request.size=U::S;
    const auto idA=GeneratorParitySystem::StableRequestIdentity(request);
    const auto idB=GeneratorParitySystem::StableRequestIdentity(request);
    Check(idA==idB&&!idA.empty(),"1098 identical canonical generator requests have stable identity");
    auto changed=request;changed.seed++;
    Check(GeneratorParitySystem::StableRequestIdentity(changed)!=idA,"1099 seed change is explicit and changes generator identity");
    changed=request;changed.domain=GeneratorDomain::Station;
    Check(GeneratorParitySystem::StableRequestIdentity(changed)!=idA,"1100 generator domain participates in stable request identity");
    Check(GeneratorParitySystem::Validate(request).valid,"1101 canonical ship generator request validates independently of UI workspace");

    Check(ShipClassRoleSystem::Envelope(ShipClass::Frigate).maximumLengthMeters<ShipClassRoleSystem::Envelope(ShipClass::Battlecruiser).minimumLengthMeters,
          "1102 large combat classes occupy distinct physical envelopes rather than labels");

    auto hull=Module("hull");std::vector<ShipyardModuleRecord> catalog{hull};
    auto tiny=SpanRecipe(hull,2,-2.0f,2.0f);const auto tinyBefore=ShipClassGenerationAuthoritySystem::MeasureLengthMeters(tiny,catalog);
    auto tinyReport=ShipClassGenerationAuthoritySystem::Resolve(tiny,catalog,ShipClass::Battlecruiser,U::L);
    Check(!tinyReport.valid&&tinyReport.topologyRegenerationRequired,"1103 tiny draft cannot satisfy Battlecruiser class by global inflation");
    auto tinyStamped=tiny;const auto stampedReport=ShipClassGenerationAuthoritySystem::ApplyAndStamp(tinyStamped,catalog,ShipClass::Battlecruiser,U::L);
    Check(!stampedReport.valid&&Near(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(tinyStamped,catalog),tinyBefore),"1104 rejected class correction leaves source geometry unchanged");

    auto oversized=SpanRecipe(hull,6,-30.0f,30.0f);auto lowCorrection=ShipClassGenerationAuthoritySystem::Resolve(oversized,catalog,ShipClass::Frigate,U::XS);
    Check(!lowCorrection.valid&&lowCorrection.appliedScale<ShipClassGenerationAuthoritySystem::MinimumFinalCorrectionScale,"1105 destructive down-scaling is rejected instead of crushing detail");
    auto undersized=SpanRecipe(hull,6,-13.0f,13.0f);auto highCorrection=ShipClassGenerationAuthoritySystem::Resolve(undersized,catalog,ShipClass::Frigate,U::XS);
    Check(!highCorrection.valid&&highCorrection.appliedScale>ShipClassGenerationAuthoritySystem::MaximumFinalCorrectionScale,"1106 destructive up-scaling is rejected instead of stretching detail");

    auto valid=SpanRecipe(hull,6,-18.0f,18.0f);auto validReport=ShipClassGenerationAuthoritySystem::Resolve(valid,catalog,ShipClass::Frigate,U::XS);
    Check(validReport.valid&&Near(validReport.measuredLengthMeters,40.0f),"1107 class-correct topology certifies without large geometry scaling");
    auto sparse=SpanRecipe(hull,5,-18.0f,18.0f);auto sparseReport=ShipClassGenerationAuthoritySystem::Resolve(sparse,catalog,ShipClass::Frigate,U::XS);
    Check(!sparseReport.valid,"1108 class minimum module topology is a hard generation constraint");
    auto dense=SpanRecipe(hull,23,-18.0f,18.0f);auto denseReport=ShipClassGenerationAuthoritySystem::Resolve(dense,catalog,ShipClass::Frigate,U::XS);
    Check(!denseReport.valid,"1109 class maximum module topology is a hard generation constraint");

    auto hugeInstance=valid;hugeInstance.modules.front().scaleX=1.6f;auto hugeReport=ShipClassGenerationAuthoritySystem::Resolve(hugeInstance,catalog,ShipClass::Frigate,U::XS);
    Check(!hugeReport.valid&&hugeReport.maximumInstanceScale>ShipClassGenerationAuthoritySystem::MaximumGeneratedInstanceScale,"1110 generated module instances cannot exceed certified scale range");

    auto rotated=SpanRecipe(hull,1,0,0);rotated.modules.front().yawDegrees=90.0f;auto rb=ShipClassGenerationAuthoritySystem::MeasureBoundsMeters(rotated,catalog);
    Check(rb.valid&&Near(rb.size.x,4.0f)&&Near(rb.size.y,3.0f),"1111 physical measurement honors transformed module orientation");

    auto needleModule=Module("needle",.2f,2.0f,1.0f);std::vector<ShipyardModuleRecord> needleCatalog{needleModule};auto needle=SpanRecipe(needleModule,6,-18,18);
    Check(!ShipClassGenerationAuthoritySystem::Resolve(needle,needleCatalog,ShipClass::Frigate,U::XS).valid,"1112 collapsed width/length plane is rejected");
    auto flatModule=Module("flat",1.5f,2.0f,.10f);std::vector<ShipyardModuleRecord> flatCatalog{flatModule};auto flat=SpanRecipe(flatModule,6,-18,18);
    Check(!ShipClassGenerationAuthoritySystem::Resolve(flat,flatCatalog,ShipClass::Frigate,U::XS).valid,"1113 collapsed height/length plane is rejected");

    auto clampReport=ShipClassGenerationAuthoritySystem::Resolve(valid,catalog,ShipClass::Frigate,U::XL);
    Check(clampReport.resolvedSize==U::M&&!clampReport.warnings.empty(),"1114 out-of-class XS-XL request is visibly clamped and reported");
    Check(Near(ShipClassGenerationAuthoritySystem::CombinedScale(tiny,catalog,ShipClass::Battlecruiser,U::L),1.0f),"1115 invalid class correction cannot leak a giant scale through CombinedScale");

    auto blockedScale=valid;const auto blockedBefore=ShipClassGenerationAuthoritySystem::MeasureLengthMeters(blockedScale,catalog);ShipClassGenerationAuthoritySystem::ApplyScale(blockedScale,2.0f);
    Check(Near(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(blockedScale,catalog),blockedBefore),"1116 ApplyScale refuses out-of-policy correction factors");
    auto minorScale=valid;ShipClassGenerationAuthoritySystem::ApplyScale(minorScale,1.05f);
    Check(ShipClassGenerationAuthoritySystem::MeasureLengthMeters(minorScale,catalog)>40.0f,"1117 small final metric correction remains available inside policy bounds");

    auto fullBounds=ShipClassGenerationAuthoritySystem::MeasureBoundsMeters(valid,catalog);
    Check(fullBounds.valid&&fullBounds.size.x>0&&fullBounds.size.y>0&&fullBounds.size.z>0,"1118 generator diagnostics expose full 3D measured bounds");
    auto validStamped=valid;auto validStampedReport=ShipClassGenerationAuthoritySystem::ApplyAndStamp(validStamped,catalog,ShipClass::Frigate,U::XS);
    Check(validStampedReport.valid&&validStamped.runtimeCertificationMessage=="CLASS_TOPOLOGY_ENVELOPE_CERTIFIED","1119 accepted generation is stamped as topology-certified");
    Check(tinyStamped.runtimeCertificationMessage=="CLASS_TOPOLOGY_REGEN_REQUIRED","1120 rejected generation explicitly requests topology regeneration");

    const auto json=GeneratorParitySystem::RequestJson(request);
    Check(json.find("subspace.generator-request.v1")!=std::string::npos&&json.find("\"domain\": \"ship\"")!=std::string::npos,"1121 generator request remains the shared runtime/Blender parity contract");
    Check(json.find("424242")!=std::string::npos&&json.find("FRIGATE")!=std::string::npos,"1122 request serialization preserves deterministic seed and class inputs");

    std::cout<<"Pass1098-1122 assertions: "<<(assertions-failures)<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
