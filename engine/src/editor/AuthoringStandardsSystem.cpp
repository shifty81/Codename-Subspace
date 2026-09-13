#include "editor/AuthoringStandardsSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
void AddSurface(std::vector<FlatSurfaceSnapCandidate>& out,const char* id,const VisualModuleSurfaceContact& c,
                float spanA,float spanB,Vector3 tangentU,Vector3 tangentV,
                float minArea,float minConfidence,float pitch,float maxPen){
    if(!c.valid||c.supportingArea<minArea||c.confidence<minConfidence)return;
    FlatSurfaceSnapCandidate s;s.id=id;s.point=c.point;s.normal=c.normal;s.tangentU=tangentU;s.tangentV=tangentV;
    s.spanUMeters=spanA;s.spanVMeters=spanB;s.supportingArea=c.supportingArea;s.confidence=c.confidence;
    s.gridPitchMeters=pitch;s.maximumInsertionMeters=maxPen;
    s.gridColumns=std::clamp(static_cast<int>(std::floor(std::max(pitch,spanA)/pitch))+1,1,9);
    s.gridRows=std::clamp(static_cast<int>(std::floor(std::max(pitch,spanB)/pitch))+1,1,9);
    out.push_back(s);
}
}

std::vector<FlatSurfaceSnapCandidate> AuthoringStandardsSystem::DiscoverFlatSnapSurfaces(const ShipyardModuleRecord& r,float minArea,float minConfidence,const WorldScaleProfile& scale){
    std::vector<FlatSurfaceSnapCandidate> out;const float width=r.source.halfWidth*2.0f,length=r.source.halfLength*2.0f,height=r.source.halfHeight*2.0f;const float pitch=std::max(scale.detailSnapMeters,.25f);const float maxPen=std::min(.25f,std::max(.04f,scale.fineSnapMeters*.72f));
    AddSurface(out,"forward",r.source.forwardSurface,width,height,{1,0,0},{0,0,1},minArea,minConfidence,pitch,maxPen);
    AddSurface(out,"aft",r.source.aftSurface,width,height,{1,0,0},{0,0,1},minArea,minConfidence,pitch,maxPen);
    AddSurface(out,"port",r.source.portSurface,length,height,{0,1,0},{0,0,1},minArea,minConfidence,pitch,maxPen);
    AddSurface(out,"starboard",r.source.starboardSurface,length,height,{0,1,0},{0,0,1},minArea,minConfidence,pitch,maxPen);
    AddSurface(out,"dorsal",r.source.dorsalSurface,width,length,{1,0,0},{0,1,0},minArea,minConfidence,pitch,maxPen);
    AddSurface(out,"ventral",r.source.ventralSurface,width,length,{1,0,0},{0,1,0},minArea,minConfidence,pitch,maxPen);
    std::sort(out.begin(),out.end(),[](const auto& a,const auto& b){if(a.confidence!=b.confidence)return a.confidence>b.confidence;return a.supportingArea>b.supportingArea;});return out;
}

SemanticObjectDefinition AuthoringStandardsSystem::DefaultObject(SemanticObjectPurpose p,const WorldScaleProfile& s){
    SemanticObjectDefinition o;o.purpose=p;o.interactionReachMeters=s.referenceReachMeters;o.snapStepMeters=s.fineSnapMeters;o.approachClearanceMeters=std::max(.55f,s.referenceShoulderWidthMeters*1.45f);o.minimumHeadClearanceMeters=s.referenceDoorHeightMeters;
    switch(p){
        case SemanticObjectPurpose::Seat:o.sizeMeters={.55f,.62f,.95f};o.interactionPoint={0,-.38f,.48f};o.sitInteraction=true;break;
        case SemanticObjectPurpose::Table:o.sizeMeters={1.20f,.70f,.76f};o.interactionPoint={0,-.55f,.78f};break;
        case SemanticObjectPurpose::Storage:o.sizeMeters={.90f,.62f,1.10f};o.interactionPoint={0,-.46f,.95f};o.storageInteraction=true;break;
        case SemanticObjectPurpose::Console:o.sizeMeters={.92f,.55f,1.25f};o.interactionPoint={0,-.48f,1.12f};o.consoleInteraction=true;break;
        case SemanticObjectPurpose::Door:o.sizeMeters={s.referenceDoorWidthMeters,.12f,s.referenceDoorHeightMeters};o.interactionPoint={0,-.45f,s.referenceEyeHeightMeters};o.minimumHeadClearanceMeters=s.referenceDoorHeightMeters;break;
        case SemanticObjectPurpose::Hatch:o.sizeMeters={.90f,.90f,.10f};o.interactionPoint={0,0,.25f};break;
        case SemanticObjectPurpose::Airlock:o.sizeMeters={std::max(1.15f,s.referenceDoorWidthMeters),1.25f,std::max(2.30f,s.referenceDoorHeightMeters)};o.interactionPoint={0,-.62f,s.referenceEyeHeightMeters};break;
        case SemanticObjectPurpose::Bed:o.sizeMeters={.90f,2.05f,.58f};o.interactionPoint={0,-.55f,.58f};break;
        case SemanticObjectPurpose::Workbench:o.sizeMeters={1.40f,.72f,.98f};o.interactionPoint={0,-.55f,1.02f};break;
        case SemanticObjectPurpose::Fixture:o.sizeMeters={.50f,.30f,.50f};o.interactionPoint={0,-.35f,.80f};break;
        default:o.sizeMeters={1,1,1};o.interactionPoint={0,-.50f,s.referenceEyeHeightMeters};break;
    }
    return o;
}

AuthoringValidationResult AuthoringStandardsSystem::ValidateObject(const SemanticObjectDefinition& o,const WorldScaleProfile& s){
    AuthoringValidationResult r;if(o.id.empty())r.warnings.push_back("semantic object has no stable authored id");if(o.sizeMeters.x<=0||o.sizeMeters.y<=0||o.sizeMeters.z<=0)r.errors.push_back("semantic object dimensions must be positive");if(o.interactionReachMeters<=0||o.interactionReachMeters>s.referenceReachMeters*1.65f)r.errors.push_back("interaction point is outside canonical player reach envelope");if(o.requiresHumanClearance&&o.minimumHeadClearanceMeters>0&&o.minimumHeadClearanceMeters<s.referencePlayerHeightMeters*1.08f)r.errors.push_back("interactive object head clearance is below canonical player envelope");if(o.purpose==SemanticObjectPurpose::Door&&o.sizeMeters.x<s.referenceDoorWidthMeters*.92f)r.errors.push_back("door is too narrow for canonical player");if(o.purpose==SemanticObjectPurpose::Door&&o.sizeMeters.z<s.referenceDoorHeightMeters*.95f)r.errors.push_back("door is too short for canonical player");if(o.purpose==SemanticObjectPurpose::Seat&&o.interactionPoint.z>s.referencePlayerHeightMeters*.55f)r.warnings.push_back("seat interaction height is high relative to canonical player");r.valid=r.errors.empty();return r;
}

CohesiveAssemblyBakePolicy AuthoringStandardsSystem::DefaultBakePolicy(const WorldScaleProfile& s){CohesiveAssemblyBakePolicy p;p.maximumAttachmentPenetrationMeters=std::min(.25f,std::max(.08f,s.fineSnapMeters*.72f));p.minimumWalkableHeadroomMeters=std::max(2.05f,s.referencePlayerHeightMeters*1.15f);p.minimumWalkableWidthMeters=std::max(s.referenceDoorWidthMeters,s.referenceShoulderWidthMeters*2.1f);return p;}
float AuthoringStandardsSystem::AllowedAttachmentPenetration(const ShipyardAssemblySocket& socket,const CohesiveAssemblyBakePolicy& p){return std::clamp(socket.insertionDepth,0.0f,p.maximumAttachmentPenetrationMeters);}
const char* AuthoringStandardsSystem::PurposeName(SemanticObjectPurpose p){switch(p){case SemanticObjectPurpose::Seat:return"SEAT";case SemanticObjectPurpose::Table:return"TABLE";case SemanticObjectPurpose::Storage:return"STORAGE";case SemanticObjectPurpose::Console:return"CONSOLE";case SemanticObjectPurpose::Door:return"DOOR";case SemanticObjectPurpose::Hatch:return"HATCH";case SemanticObjectPurpose::Airlock:return"AIRLOCK";case SemanticObjectPurpose::Bed:return"BED";case SemanticObjectPurpose::Workbench:return"WORKBENCH";case SemanticObjectPurpose::Fixture:return"FIXTURE";default:return"GENERIC";}}

} // namespace subspace
