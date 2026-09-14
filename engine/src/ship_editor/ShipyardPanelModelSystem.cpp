#include "ship_editor/ShipyardPanelModelSystem.h"

#include "content/ShipyardPartTaxonomySystem.h"
#include "content/UniversalKitbashAuthority.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {

const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& c,const std::string& id){
    for(const auto& r:c){if(r.source.moduleId==id)return &r;}
    return nullptr;
}
std::string F(float v,int precision=2){std::ostringstream s;s.setf(std::ios::fixed);s<<std::setprecision(precision)<<v;return s.str();}
std::string Label(const ShipyardModuleRecord* r,const VisualModulePlacement& p){
    return r?ShipyardPartTaxonomySystem::DisplayName(*r):p.moduleId;
}
} // namespace

ShipyardPanelModel ShipyardPanelModelSystem::Build(const ShipyardDocument& d,
                                                    const std::vector<ShipyardModuleRecord>& c,
                                                    const ShipyardSelectionState& s){
    ShipyardPanelModel out;out.outliner=BuildOutliner(d,c);out.properties=BuildProperties(d,c,s);
    if(s.primary.Valid()){
        const auto i=ShipyardDocumentSystem::FindModuleIndex(d,s.primary);
        if(i&&*i<d.recipe.modules.size()){
            const auto& placement=d.recipe.modules[*i];
            out.selectedLabel=Label(FindRecord(c,placement.moduleId),placement);
            out.selectedInstanceId=ShipyardStableIdSystem::ToString(s.primary);
            out.selectedDefinitionId=placement.moduleId;
            out.definitionEditable=true;
            out.actions.push_back({"definition.open","Open Definition","","shipyard.definition",true,""});
        }
    }
    return out;
}

std::vector<ShipyardOutlinerNode> ShipyardPanelModelSystem::BuildOutliner(
    const ShipyardDocument& d,const std::vector<ShipyardModuleRecord>& c){
    std::vector<ShipyardOutlinerNode> out;
    out.push_back({d.documentId,{},d.recipe.recipeId.empty()?"Ship":d.recipe.recipeId,"Ship",0,true,true});
    std::vector<int> parent(d.recipe.modules.size(),-1);
    for(const auto& e:d.recipe.attachments)if(e.childModuleIndex<parent.size()&&e.parentModuleIndex<parent.size())
        parent[e.childModuleIndex]=static_cast<int>(e.parentModuleIndex);
    std::vector<int> depth(d.recipe.modules.size(),1);
    for(std::size_t i=0;i<depth.size();++i){int p=parent[i],guard=0;while(p>=0&&static_cast<std::size_t>(p)<parent.size()&&guard++<64){++depth[i];p=parent[static_cast<std::size_t>(p)];}}
    for(std::size_t i=0;i<d.recipe.modules.size();++i){
        const auto& p=d.recipe.modules[i];auto parentId=d.documentId;
        if(parent[i]>=0)parentId=ShipyardDocumentSystem::ModuleIdAt(d,static_cast<std::size_t>(parent[i]));
        const auto* r=FindRecord(c,p.moduleId);
        out.push_back({ShipyardDocumentSystem::ModuleIdAt(d,i),parentId,Label(r,p),r?ShipyardModuleSystem::ClassName(r->moduleClass):"Module",depth[i],true,true});
    }
    return out;
}

std::vector<EditorPropertySection> ShipyardPanelModelSystem::BuildProperties(
    const ShipyardDocument& d,const std::vector<ShipyardModuleRecord>& c,const ShipyardSelectionState& s){
    std::vector<EditorPropertySection> out;
    if(!s.primary.Valid())return out;
    const auto i=ShipyardDocumentSystem::FindModuleIndex(d,s.primary);
    if(!i||*i>=d.recipe.modules.size())return out;
    const auto& p=d.recipe.modules[*i];const auto* r=FindRecord(c,p.moduleId);

    EditorPropertySection identity{"identity","Identity",false,false,{}};
    identity.properties.push_back({"module.scope","Editing Scope",EditorPropertyType::ReadOnly,"INSTANCE","","shipyard.instance",false,false});
    identity.properties.push_back({"module.instance","Instance ID",EditorPropertyType::ReadOnly,ShipyardStableIdSystem::ToString(s.primary),"","",false,true});
    identity.properties.push_back({"module.definition","Definition",EditorPropertyType::AssetReference,p.moduleId,"","shipyard.definition",false,false});
    if(r){identity.properties.push_back({"module.class","Class",EditorPropertyType::ReadOnly,ShipyardModuleSystem::ClassName(r->moduleClass),"","",false,false});
          identity.properties.push_back({"module.semantic","Semantic",EditorPropertyType::ReadOnly,ShipyardModuleSystem::SemanticName(r->semantic),"","",false,true});}
    out.push_back(std::move(identity));

    EditorPropertySection t{"transform","Transform",false,false,{}};
    t.properties.push_back({"transform.x","X",EditorPropertyType::Float,F(p.x),"m","",true,false});
    t.properties.push_back({"transform.y","Y",EditorPropertyType::Float,F(p.y),"m","",true,false});
    t.properties.push_back({"transform.z","Z",EditorPropertyType::Float,F(p.z),"m","",true,false});
    t.properties.push_back({"rotation.pitch","Pitch",EditorPropertyType::Float,F(p.pitchDegrees,1),"deg","",true,false});
    t.properties.push_back({"rotation.yaw","Yaw",EditorPropertyType::Float,F(p.yawDegrees,1),"deg","",true,false});
    t.properties.push_back({"rotation.roll","Roll",EditorPropertyType::Float,F(p.rollDegrees,1),"deg","",true,false});
    t.properties.push_back({"scale.x","Scale X",EditorPropertyType::Float,F(p.scaleX),"x","",true,true});
    t.properties.push_back({"scale.y","Scale Y",EditorPropertyType::Float,F(p.scaleY),"x","",true,true});
    t.properties.push_back({"scale.z","Scale Z",EditorPropertyType::Float,F(p.scaleZ),"x","",true,true});
    out.push_back(std::move(t));

    if(r){
        const auto mount=ShipyardMountProfileSystem::Build(*r);
        EditorPropertySection a{"attachment","Attachment",false,false,{}};
        a.properties.push_back({"attachment.root","Primary Mount",EditorPropertyType::ReadOnly,mount.primaryRoot.valid?mount.primaryRoot.face:"AUTO / REVIEW","","",false,false});
        a.properties.push_back({"attachment.role","Placement Role",EditorPropertyType::ReadOnly,mount.placementRole,"","",false,false});
        a.properties.push_back({"attachment.clearance","Minimum Clearance",EditorPropertyType::ReadOnly,F(mount.minimumClearanceMeters),"m","",false,true});
        a.properties.push_back({"attachment.provenance","Mount Provenance",EditorPropertyType::ReadOnly,mount.provenance,"","",false,true});
        out.push_back(std::move(a));

        EditorPropertySection g{"generation","Generation",false,true,{}};
        g.properties.push_back({"generation.eligible","Generator Eligible",EditorPropertyType::Bool,r->generatorEligible?"true":"false","","",false,false});
        g.properties.push_back({"generation.paired","Paired Placement",EditorPropertyType::Bool,r->pairedPlacement?"true":"false","","",false,false});
        out.push_back(std::move(g));
    }
    return out;
}

} // namespace subspace
