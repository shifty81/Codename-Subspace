#include "modeling/ShipyardModelingSystem.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;

// Keep vertex construction explicit so the canonical baker remains portable C++17.
assets::StaticVertex MakeVertex(float x,float y,float z,float nx,float ny,float nz,float u=0,float vv=0){
    assets::StaticVertex out;out.position={x,y,z};out.normal={nx,ny,nz};out.uv0={u,vv};return out;
}

void AppendQuad(assets::MeshPrimitive& p,
                const Vector3& a,const Vector3& b,const Vector3& c,const Vector3& d,
                const Vector3& n){
    const auto base=static_cast<std::uint32_t>(p.vertices.size());
    p.vertices.push_back(MakeVertex(a.x,a.y,a.z,n.x,n.y,n.z,0,0));
    p.vertices.push_back(MakeVertex(b.x,b.y,b.z,n.x,n.y,n.z,1,0));
    p.vertices.push_back(MakeVertex(c.x,c.y,c.z,n.x,n.y,n.z,1,1));
    p.vertices.push_back(MakeVertex(d.x,d.y,d.z,n.x,n.y,n.z,0,1));
    p.indices.insert(p.indices.end(),{base,base+1,base+2,base,base+2,base+3});
}

assets::CanonicalMesh MakeBox(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;mesh.name=d.id;
    assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const float x=std::max(.005f,d.size.x)*.5f,y=std::max(.005f,d.size.y)*.5f,z=std::max(.005f,d.size.z)*.5f;
    AppendQuad(p,{-x,-y,-z},{ x,-y,-z},{ x, y,-z},{-x, y,-z},{0,0,-1});
    AppendQuad(p,{-x, y, z},{ x, y, z},{ x,-y, z},{-x,-y, z},{0,0,1});
    AppendQuad(p,{-x,-y, z},{ x,-y, z},{ x,-y,-z},{-x,-y,-z},{0,-1,0});
    AppendQuad(p,{ x,-y, z},{ x, y, z},{ x, y,-z},{ x,-y,-z},{1,0,0});
    AppendQuad(p,{ x, y, z},{-x, y, z},{-x, y,-z},{ x, y,-z},{0,1,0});
    AppendQuad(p,{-x, y, z},{-x,-y, z},{-x,-y,-z},{-x, y,-z},{-1,0,0});
    mesh.primitives.push_back(std::move(p));return mesh;
}

assets::CanonicalMesh MakeWedge(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;mesh.name=d.id;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const float x=std::max(.005f,d.size.x)*.5f,y=std::max(.005f,d.size.y)*.5f,z=std::max(.005f,d.size.z)*.5f;
    // A useful ship-authoring wedge: full-height aft edge (+Y), tapered to the
    // centerline at the forward edge (-Y).
    const Vector3 a{-x,-y,0},b{x,-y,0},c{x,y,-z},dd{-x,y,-z},e{x,y,z},f{-x,y,z};
    AppendQuad(p,a,b,c,dd,{0,0,-1});
    AppendQuad(p,f,e,b,a,{0,0,1});
    AppendQuad(p,dd,c,e,f,{0,1,0});
    // Side triangles.
    auto tri=[&](Vector3 q0,Vector3 q1,Vector3 q2,Vector3 n){const auto base=static_cast<std::uint32_t>(p.vertices.size());for(auto q:{q0,q1,q2})p.vertices.push_back(MakeVertex(q.x,q.y,q.z,n.x,n.y,n.z));p.indices.insert(p.indices.end(),{base,base+1,base+2});};
    tri(a,dd,f,{-1,0,0});tri(b,e,c,{1,0,0});
    mesh.primitives.push_back(std::move(p));return mesh;
}

assets::CanonicalMesh MakeCylinder(const ModelingPrimitiveDefinition& d,bool cone=false,bool tube=false){
    assets::CanonicalMesh mesh;mesh.name=d.id;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const std::uint32_t seg=std::clamp<std::uint32_t>(d.radialSegments,6,96);
    const float rx=std::max(.005f,d.size.x)*.5f,rz=std::max(.005f,d.size.z)*.5f,hy=std::max(.005f,d.size.y)*.5f;
    const float topScale=cone?0.0f:1.0f;
    for(std::uint32_t i=0;i<seg;++i){
        const float a0=2*kPi*float(i)/float(seg),a1=2*kPi*float(i+1)/float(seg);
        Vector3 p0{std::cos(a0)*rx,-hy,std::sin(a0)*rz},p1{std::cos(a1)*rx,-hy,std::sin(a1)*rz};
        Vector3 p2{std::cos(a1)*rx*topScale,hy,std::sin(a1)*rz*topScale},p3{std::cos(a0)*rx*topScale,hy,std::sin(a0)*rz*topScale};
        Vector3 n0{std::cos((a0+a1)*.5f),0,std::sin((a0+a1)*.5f)};
        AppendQuad(p,p0,p1,p2,p3,n0);
    }
    if(!tube){
        auto cap=[&](float y,float normalY,float scale){
            const auto center=static_cast<std::uint32_t>(p.vertices.size());p.vertices.push_back(MakeVertex(0,y,0,0,normalY,0,.5f,.5f));
            for(std::uint32_t i=0;i<=seg;++i){const float a=2*kPi*float(i)/float(seg);p.vertices.push_back(MakeVertex(std::cos(a)*rx*scale,y,std::sin(a)*rz*scale,0,normalY,0));}
            for(std::uint32_t i=0;i<seg;++i){if(normalY<0)p.indices.insert(p.indices.end(),{center,center+i+2,center+i+1});else p.indices.insert(p.indices.end(),{center,center+i+1,center+i+2});}
        };
        cap(-hy,-1,1);if(topScale>0)cap(hy,1,topScale);
    }
    mesh.primitives.push_back(std::move(p));return mesh;
}

assets::CanonicalMesh MeshFor(const ModelingPrimitiveDefinition& d){
    switch(d.type){
        case ModelingPrimitiveType::Wedge: return MakeWedge(d);
        case ModelingPrimitiveType::Cylinder:
        case ModelingPrimitiveType::Barrel:
        case ModelingPrimitiveType::Nozzle:
        case ModelingPrimitiveType::Pipe: return MakeCylinder(d,false,d.type==ModelingPrimitiveType::Pipe);
        case ModelingPrimitiveType::Cone: return MakeCylinder(d,true,false);
        case ModelingPrimitiveType::Tube:
        case ModelingPrimitiveType::Ring:
        case ModelingPrimitiveType::TurretRing: return MakeCylinder(d,false,true);
        default:return MakeBox(d);
    }
}

assets::Matrix4 TranslationMatrix(const Vector3& p){auto m=assets::Matrix4::Identity();m.value[12]=p.x;m.value[13]=p.y;m.value[14]=p.z;return m;}
}

const char* ShipyardModelingSystem::PrimitiveName(ModelingPrimitiveType t){
    switch(t){case ModelingPrimitiveType::Box:return "BOX";case ModelingPrimitiveType::Wedge:return "WEDGE";case ModelingPrimitiveType::Cylinder:return "CYLINDER";case ModelingPrimitiveType::Cone:return "CONE";case ModelingPrimitiveType::Sphere:return "SPHERE";case ModelingPrimitiveType::Tube:return "TUBE";case ModelingPrimitiveType::Ring:return "RING";case ModelingPrimitiveType::Beam:return "BEAM";case ModelingPrimitiveType::Plate:return "PLATE";case ModelingPrimitiveType::HullSegment:return "HULL SEGMENT";case ModelingPrimitiveType::Wing:return "WING";case ModelingPrimitiveType::EngineHousing:return "ENGINE HOUSING";case ModelingPrimitiveType::Nozzle:return "NOZZLE";case ModelingPrimitiveType::TurretRing:return "TURRET RING";case ModelingPrimitiveType::Barrel:return "BARREL";case ModelingPrimitiveType::Pipe:return "PIPE";}return "UNKNOWN";
}
const char* ShipyardModelingSystem::SelectionModeName(ModelingSelectionMode m){switch(m){case ModelingSelectionMode::Object:return "OBJECT";case ModelingSelectionMode::Vertex:return "VERTEX";case ModelingSelectionMode::Edge:return "EDGE";case ModelingSelectionMode::Face:return "FACE";}return "OBJECT";}
const char* ShipyardModelingSystem::ModifierName(ModelingModifierType t){switch(t){case ModelingModifierType::Transform:return "TRANSFORM";case ModelingModifierType::Stretch:return "STRETCH";case ModelingModifierType::Taper:return "TAPER";case ModelingModifierType::Bend:return "BEND";case ModelingModifierType::Twist:return "TWIST";case ModelingModifierType::Bevel:return "BEVEL";case ModelingModifierType::Inset:return "INSET";case ModelingModifierType::Extrude:return "EXTRUDE";case ModelingModifierType::Mirror:return "MIRROR";case ModelingModifierType::LinearArray:return "LINEAR ARRAY";case ModelingModifierType::RadialArray:return "RADIAL ARRAY";case ModelingModifierType::BooleanUnion:return "BOOLEAN UNION";case ModelingModifierType::BooleanSubtract:return "BOOLEAN SUBTRACT";case ModelingModifierType::BooleanIntersect:return "BOOLEAN INTERSECT";}return "MODIFIER";}

ModelingPrimitiveDefinition ShipyardModelingSystem::DefaultPrimitive(ModelingPrimitiveType t,std::size_t ordinal){
    ModelingPrimitiveDefinition d;d.type=t;d.id="shape."+std::to_string(ordinal+1);d.size={1,1,1};
    switch(t){case ModelingPrimitiveType::Plate:d.size={1.5f,1.5f,.12f};break;case ModelingPrimitiveType::Beam:d.size={.35f,2.0f,.35f};break;case ModelingPrimitiveType::HullSegment:d.size={3.0f,5.0f,1.8f};break;case ModelingPrimitiveType::Wing:d.size={4.0f,2.5f,.3f};break;case ModelingPrimitiveType::EngineHousing:d.size={1.8f,3.2f,1.8f};d.surfaceSemantic="EngineHousing";break;case ModelingPrimitiveType::Nozzle:d.size={1.2f,1.3f,1.2f};d.surfaceSemantic="Nozzle";break;case ModelingPrimitiveType::TurretRing:d.size={1.6f,.35f,1.6f};d.surfaceSemantic="WeaponMetal";break;case ModelingPrimitiveType::Barrel:d.size={.28f,2.5f,.28f};d.surfaceSemantic="WeaponMetal";break;case ModelingPrimitiveType::Pipe:d.size={.22f,2.0f,.22f};d.wallThickness=.03f;d.surfaceSemantic="StructuralMetal";break;default:break;}return d;
}
std::size_t ShipyardModelingSystem::AddPrimitive(ShipyardModelRecipe& recipe,ModelingPrimitiveType t){recipe.primitives.push_back(DefaultPrimitive(t,recipe.primitives.size()));recipe.revision++;recipe.draft=true;recipe.collisionDirty=true;return recipe.primitives.size()-1;}
bool ShipyardModelingSystem::StretchPrimitive(ShipyardModelRecipe& r,std::size_t i,const Vector3& d,bool symmetric){if(i>=r.primitives.size())return false;auto& p=r.primitives[i];const Vector3 old=p.size;p.size.x=std::max(.01f,p.size.x+d.x);p.size.y=std::max(.01f,p.size.y+d.y);p.size.z=std::max(.01f,p.size.z+d.z);if(!symmetric){p.position.x+=d.x*.5f;p.position.y+=d.y*.5f;p.position.z+=d.z*.5f;}if(old.x==p.size.x&&old.y==p.size.y&&old.z==p.size.z)return false;r.revision++;r.collisionDirty=true;return true;}
bool ShipyardModelingSystem::AddModifier(ShipyardModelRecipe& r,ModelingModifier m){if(m.id.empty())m.id="modifier."+std::to_string(r.modifiers.size()+1);r.modifiers.push_back(std::move(m));r.revision++;r.draft=true;r.collisionDirty=true;return true;}
ShipyardModelingValidation ShipyardModelingSystem::Validate(const ShipyardModelRecipe& r){ShipyardModelingValidation v;if(r.primitives.empty()&&r.sourceAssetId.empty())v.errors.push_back("Model has no source geometry or authored shapes");for(const auto&p:r.primitives){if(p.id.empty())v.errors.push_back("Shape is missing an id");if(p.size.x<=0||p.size.y<=0||p.size.z<=0)v.errors.push_back("Shape has non-positive dimensions: "+p.id);if((p.type==ModelingPrimitiveType::Cylinder||p.type==ModelingPrimitiveType::Cone||p.type==ModelingPrimitiveType::Pipe)&&p.radialSegments<6)v.errors.push_back("Radial shape has too few segments: "+p.id);}if(r.collisionDirty)v.warnings.push_back("Collision needs regeneration before certification");if(r.socketsDirty)v.warnings.push_back("Socket frames need review after modeling changes");if(r.surfacesDirty)v.warnings.push_back("Surface semantics need review after topology changes");v.valid=v.errors.empty();return v;}
std::string ShipyardModelingSystem::DerivedAssetId(const ShipyardModelRecipe&r,const std::string& requested){if(!requested.empty())return requested;std::ostringstream o;o<<"subspace.modeled."<<(r.recipeId.empty()?"module":r.recipeId)<<".r"<<r.revision;std::string s=o.str();for(char&c:s)if(!(std::isalnum(static_cast<unsigned char>(c))||c=='.'||c=='_'||c=='-'))c='_';return s;}
assets::CanonicalAsset ShipyardModelingSystem::BakeCanonicalAsset(const ShipyardModelRecipe&r,const std::string& id){assets::CanonicalAsset a;a.assetId=DerivedAssetId(r,id);a.provenance.sourcePath="shipyard://model/"+r.recipeId;a.provenance.sourceFormat="SUBSPACE_MODEL_RECIPE";a.provenance.importer="Subspace Shipyard Model Workspace";a.provenance.importerVersion="1";a.provenance.importPolicy="NON_DESTRUCTIVE_RECIPE_BAKE";for(const auto&p:r.primitives){const auto mi=static_cast<assets::AssetIndex>(a.meshes.size());a.meshes.push_back(MeshFor(p));assets::CanonicalNode n;n.name=p.id;n.meshIndex=mi;n.localTransform=TranslationMatrix(p.position);a.nodes.push_back(std::move(n));}assets::ModuleDefinition m;m.moduleId=a.assetId;m.role=assets::ModuleRole::Unknown;m.size=assets::SocketSize::M;m.rootNodeIndex=a.nodes.empty()?assets::kInvalidAssetIndex:0;a.modules.push_back(std::move(m));return a;}

} // namespace subspace
