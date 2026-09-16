#include "interior/ShipInteriorStructureAuthoringSystem.h"
#include <algorithm>

namespace subspace {
const char* ShipInteriorStructureAuthoringSystem::KindName(ShipInteriorElementKind k){
    switch(k){
        case ShipInteriorElementKind::Floor:return "FLOOR";
        case ShipInteriorElementKind::Wall:return "WALL";
        case ShipInteriorElementKind::Ceiling:return "CEILING";
        case ShipInteriorElementKind::Door:return "DOOR";
        case ShipInteriorElementKind::Hatch:return "HATCH";
        case ShipInteriorElementKind::Airlock:return "AIRLOCK";
        case ShipInteriorElementKind::Window:return "WINDOW";
        case ShipInteriorElementKind::Ramp:return "RAMP";
        case ShipInteriorElementKind::Stair:return "STAIR";
        case ShipInteriorElementKind::Ladder:return "LADDER";
        case ShipInteriorElementKind::Elevator:return "ELEVATOR";
        case ShipInteriorElementKind::Corridor:return "CORRIDOR";
        case ShipInteriorElementKind::Bulkhead:return "BULKHEAD";
        default:return "CUSTOM";
    }
}
ShipInteriorStructuralElement ShipInteriorStructureAuthoringSystem::DefaultElement(ShipInteriorElementKind k,std::size_t m,int deck,std::size_t ordinal){
    ShipInteriorStructuralElement e;e.kind=k;e.moduleIndex=m;e.deck=deck;e.id=std::string("interior.")+KindName(k)+"."+std::to_string(ordinal+1);
    switch(k){
        case ShipInteriorElementKind::Floor:e.sizeMeters={2,2,.12f};e.materialId="INTERIOR_FLOOR_METAL";break;
        case ShipInteriorElementKind::Ceiling:e.sizeMeters={2,2,.10f};e.position.z=2.65f;e.materialId="INTERIOR_CEILING";break;
        case ShipInteriorElementKind::Wall:case ShipInteriorElementKind::Bulkhead:e.sizeMeters={2,.12f,2.7f};e.materialId="INTERIOR_BULKHEAD";e.pressureBoundary=k==ShipInteriorElementKind::Bulkhead;break;
        case ShipInteriorElementKind::Door:e.sizeMeters={1.15f,.18f,2.25f};e.materialId="INTERIOR_DOOR";e.openable=true;break;
        case ShipInteriorElementKind::Hatch:e.sizeMeters={.95f,.18f,1.75f};e.materialId="INTERIOR_HATCH";e.openable=true;e.pressureBoundary=true;break;
        case ShipInteriorElementKind::Airlock:e.sizeMeters={1.45f,1.8f,2.55f};e.materialId="INTERIOR_AIRLOCK";e.openable=true;e.pressureBoundary=true;break;
        case ShipInteriorElementKind::Window:e.sizeMeters={1.4f,.08f,.8f};e.materialId="INTERIOR_WINDOW";e.pressureBoundary=true;break;
        case ShipInteriorElementKind::Ramp:e.shape=ShipInteriorElementShape::Wedge;e.sizeMeters={1.8f,3.0f,.75f};break;
        case ShipInteriorElementKind::Stair:e.shape=ShipInteriorElementShape::Wedge;e.sizeMeters={1.3f,2.8f,1.5f};break;
        case ShipInteriorElementKind::Ladder:e.sizeMeters={.7f,.15f,2.6f};break;
        case ShipInteriorElementKind::Elevator:e.sizeMeters={1.8f,1.8f,2.5f};e.pressureBoundary=true;break;
        case ShipInteriorElementKind::Corridor:e.sizeMeters={2.2f,4.0f,2.6f};break;
        case ShipInteriorElementKind::Custom:e.shape=ShipInteriorElementShape::CustomModel;e.sizeMeters={1,1,1};break;
    }
    return e;
}
ShipInteriorStructuralModel ShipInteriorStructureAuthoringSystem::GenerateDefaults(const GeneratedShipInteriorProgram& p){
    ShipInteriorStructuralModel out;out.status=p.valid?"Generated module interiors":"Generated draft interior structure";
    std::size_t ordinal=0;
    for(const auto& r:p.rooms){
        auto floor=DefaultElement(ShipInteriorElementKind::Floor,r.moduleIndex,r.deck,ordinal++);
        floor.sizeMeters={std::max(1.0f,r.widthCells*r.cellSizeMeters),std::max(1.0f,r.lengthCells*r.cellSizeMeters),.12f};
        floor.position.z=r.deck*r.deckHeightMeters;out.elements.push_back(floor);
        auto ceil=DefaultElement(ShipInteriorElementKind::Ceiling,r.moduleIndex,r.deck,ordinal++);
        ceil.sizeMeters={floor.sizeMeters.x,floor.sizeMeters.y,.10f};ceil.position.z=floor.position.z+r.deckHeightMeters-.10f;out.elements.push_back(ceil);
    }
    for(const auto& ptl:p.portals){
        const auto kind=ptl.kind==InteriorPortalKind::Airlock?ShipInteriorElementKind::Airlock:
                        ptl.kind==InteriorPortalKind::Hatch?ShipInteriorElementKind::Hatch:ShipInteriorElementKind::Door;
        auto e=DefaultElement(kind,ptl.moduleA,0,ordinal++);e.portalId=ptl.portalA+"->"+ptl.portalB;out.elements.push_back(e);
    }
    return out;
}
std::size_t ShipInteriorStructureAuthoringSystem::Add(ShipInteriorStructuralModel& m,ShipInteriorElementKind k,std::size_t module,int deck){m.elements.push_back(DefaultElement(k,module,deck,m.elements.size()));m.selected=m.elements.size()-1;m.revision++;m.dirty=true;m.status=std::string("Added ")+KindName(k);return m.selected;}
bool ShipInteriorStructureAuthoringSystem::RemoveSelected(ShipInteriorStructuralModel& m){if(m.elements.empty()||m.selected>=m.elements.size())return false;if(!m.elements[m.selected].removable)return false;m.elements.erase(m.elements.begin()+static_cast<std::ptrdiff_t>(m.selected));if(m.selected>=m.elements.size()&&m.selected>0)--m.selected;m.revision++;m.dirty=true;m.status="Interior element removed";return true;}
bool ShipInteriorStructureAuthoringSystem::MoveSelected(ShipInteriorStructuralModel&m,const Vector3&d){if(m.selected>=m.elements.size())return false;m.elements[m.selected].position=m.elements[m.selected].position+d;m.revision++;m.dirty=true;return true;}
bool ShipInteriorStructureAuthoringSystem::RotateSelected(ShipInteriorStructuralModel&m,const Vector3&d){if(m.selected>=m.elements.size())return false;m.elements[m.selected].rotationDegrees=m.elements[m.selected].rotationDegrees+d;m.revision++;m.dirty=true;return true;}
bool ShipInteriorStructureAuthoringSystem::ResizeSelected(ShipInteriorStructuralModel&m,const Vector3&d){if(m.selected>=m.elements.size())return false;auto& s=m.elements[m.selected].sizeMeters;s.x=std::max(.03f,s.x+d.x);s.y=std::max(.03f,s.y+d.y);s.z=std::max(.03f,s.z+d.z);m.revision++;m.dirty=true;return true;}
bool ShipInteriorStructureAuthoringSystem::AssignMaterial(ShipInteriorStructuralModel&m,const std::string&id){if(m.selected>=m.elements.size())return false;m.elements[m.selected].materialId=id;m.revision++;m.dirty=true;return true;}
ShipInteriorStructureValidation ShipInteriorStructureAuthoringSystem::Validate(const ShipInteriorStructuralModel&m,const WorldScaleProfile&scale){
    ShipInteriorStructureValidation v;
    for(const auto&e:m.elements){
        if(e.id.empty())v.errors.push_back("Interior element missing id");
        if(e.sizeMeters.x<=0||e.sizeMeters.y<=0||e.sizeMeters.z<=0)v.errors.push_back(e.id+": invalid dimensions");
        if(e.kind==ShipInteriorElementKind::Door||e.kind==ShipInteriorElementKind::Hatch||e.kind==ShipInteriorElementKind::Airlock){
            const auto purpose=e.kind==ShipInteriorElementKind::Airlock?SemanticObjectPurpose::Airlock:(e.kind==ShipInteriorElementKind::Hatch?SemanticObjectPurpose::Hatch:SemanticObjectPurpose::Door);
            auto obj=AuthoringStandardsSystem::DefaultObject(purpose,scale);obj.id=e.id;obj.sizeMeters=e.sizeMeters;
            const auto check=AuthoringStandardsSystem::ValidateObject(obj,scale);v.errors.insert(v.errors.end(),check.errors.begin(),check.errors.end());v.warnings.insert(v.warnings.end(),check.warnings.begin(),check.warnings.end());
        }
        if(e.materialId.empty())v.warnings.push_back(e.id+": material unassigned; semantic fallback will be used");
    }
    v.valid=v.errors.empty();return v;
}
}
