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

assets::CanonicalMesh MakeSphere(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const std::uint32_t slices=std::max<std::uint32_t>(8,d.radialSegments);
    const std::uint32_t stacks=std::max<std::uint32_t>(4,slices/2);
    const float rx=d.size.x*.5f,ry=d.size.y*.5f,rz=d.size.z*.5f;
    for(std::uint32_t j=0;j<=stacks;++j){
        const float v=float(j)/float(stacks);const float lat=-kPi*.5f+v*kPi;const float cl=std::cos(lat),sl=std::sin(lat);
        for(std::uint32_t i=0;i<=slices;++i){
            const float u=float(i)/float(slices);const float lon=u*2*kPi;const float x=std::cos(lon)*cl,y=std::sin(lon)*cl,z=sl;
            p.vertices.push_back(MakeVertex(x*rx,y*ry,z*rz,x,y,z,u,1.0f-v));
        }
    }
    const std::uint32_t row=slices+1;
    for(std::uint32_t j=0;j<stacks;++j)for(std::uint32_t i=0;i<slices;++i){
        const auto a=j*row+i,b=a+1,c=a+row+1,dv=a+row;
        p.indices.insert(p.indices.end(),{a,b,c,a,c,dv});
    }
    mesh.primitives.push_back(std::move(p));return mesh;
}

// Ship-specialized shapes must not silently collapse into generic boxes.
assets::CanonicalMesh MakeHullSegment(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;mesh.name=d.id;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const float x=d.size.x*.5f,y=d.size.y*.5f,z=d.size.z*.5f;
    // Narrow forward section, wider aft section, enclosed and author-scale.
    const Vector3 a{-x,-y,-z},b{x,-y,-z},c{x,-y,z},e{-x,-y,z};
    const Vector3 f{-x*.68f,y,-z*.85f},g{x*.68f,y,-z*.85f},h{x*.68f,y,z*.85f},i{-x*.68f,y,z*.85f};
    AppendQuad(p,a,b,c,e,{0,-1,0});AppendQuad(p,f,i,h,g,{0,1,0});
    AppendQuad(p,a,f,g,b,{0,0,-1});AppendQuad(p,e,c,h,i,{0,0,1});
    AppendQuad(p,a,e,i,f,{-1,0,0});AppendQuad(p,b,g,h,c,{1,0,0});
    mesh.primitives.push_back(std::move(p));return mesh;
}
assets::CanonicalMesh MakeWing(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;mesh.name=d.id;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const float x=d.size.x*.5f,y=d.size.y*.5f,z=d.size.z*.5f;
    // Five-point planform, tapered leading/trailing edges and finite thickness.
    const Vector3 top[5]={{-x,-y,z},{-x,y,z},{x*.78f,y*.40f,z},{x,y*.10f,z},{x,-y*.52f,z}};
    Vector3 low[5];for(int j=0;j<5;++j)low[j]={top[j].x,top[j].y,-z};
    for(int j=1;j<4;++j){
        const auto base=static_cast<std::uint32_t>(p.vertices.size());
        for(const auto v:{top[0],top[j],top[j+1]})p.vertices.push_back(MakeVertex(v.x,v.y,v.z,0,0,1));
        p.indices.insert(p.indices.end(),{base,base+2,base+1});
        const auto bottom=static_cast<std::uint32_t>(p.vertices.size());
        for(const auto v:{low[0],low[j+1],low[j]})p.vertices.push_back(MakeVertex(v.x,v.y,v.z,0,0,-1));
        p.indices.insert(p.indices.end(),{bottom,bottom+2,bottom+1});
    }
    for(int j=0;j<5;++j){
        const int next=(j+1)%5;
        const auto ex=top[next].x-top[j].x,ey=top[next].y-top[j].y;
        const auto inv=1.0f/std::max(.0001f,std::sqrt(ex*ex+ey*ey));
        AppendQuad(p,low[next],low[j],top[j],top[next],{-ey*inv,ex*inv,0});
    }
    mesh.primitives.push_back(std::move(p));return mesh;
}
assets::CanonicalMesh MakeHollowTube(const ModelingPrimitiveDefinition& d){
    assets::CanonicalMesh mesh;mesh.name=d.id;assets::MeshPrimitive p;p.hasNormals=true;p.hasUv0=true;
    const auto seg=std::clamp<std::uint32_t>(d.radialSegments,6,96);
    const float rx=d.size.x*.5f,rz=d.size.z*.5f,hy=d.size.y*.5f;
    const float irx=std::max(.005f,rx-d.wallThickness),irz=std::max(.005f,rz-d.wallThickness);
    for(std::uint32_t j=0;j<seg;++j){
        const float a=2*kPi*float(j)/float(seg),b=2*kPi*float(j+1)/float(seg);
        const Vector3 lo{std::cos(a)*rx,-hy,std::sin(a)*rz},ro{std::cos(b)*rx,-hy,std::sin(b)*rz};
        const Vector3 hi{lo.x,hy,lo.z},hr{ro.x,hy,ro.z};
        const Vector3 li{std::cos(a)*irx,-hy,std::sin(a)*irz},ri{std::cos(b)*irx,-hy,std::sin(b)*irz};
        const Vector3 lh{li.x,hy,li.z},rh{ri.x,hy,ri.z};
        const float mid=(a+b)*.5f;
        const Vector3 normal{std::cos(mid),0,std::sin(mid)};
        AppendQuad(p,ro,lo,hi,hr,normal);
        AppendQuad(p,li,ri,rh,lh,normal*-1.0f);
        AppendQuad(p,hr,hi,lh,rh,{0,1,0});
        AppendQuad(p,lo,ro,ri,li,{0,-1,0});
    }
    mesh.primitives.push_back(std::move(p));return mesh;
}

assets::CanonicalMesh MeshFor(const ModelingPrimitiveDefinition& d){
    switch(d.type){
        case ModelingPrimitiveType::Wedge: return MakeWedge(d);
        case ModelingPrimitiveType::Cylinder:
        case ModelingPrimitiveType::Barrel:
        case ModelingPrimitiveType::Nozzle:
        return MakeCylinder(d,false,false);
        case ModelingPrimitiveType::Pipe: return MakeHollowTube(d);
        case ModelingPrimitiveType::Cone: return MakeCylinder(d,true,false);
        case ModelingPrimitiveType::Sphere: return MakeSphere(d);
        case ModelingPrimitiveType::Tube:
        case ModelingPrimitiveType::Ring:
        case ModelingPrimitiveType::TurretRing: return MakeHollowTube(d);
        case ModelingPrimitiveType::Wing: return MakeWing(d);
        case ModelingPrimitiveType::HullSegment: return MakeHullSegment(d);
        default:return MakeBox(d);
    }
}


// Evaluated geometry never mutates the non-destructive authored recipe.
// Mirror reflects vertices and face winding and conjugates the local node
// matrix, preserving authored rotations without negative-scale runtime nodes.
struct EvaluatedShape {
    assets::CanonicalMesh mesh;
    assets::Matrix4 transform;
    std::string name;
};
int MirrorAxis(const Vector3& direction){
    if(std::fabs(direction.x)>.5f)return 0;
    if(std::fabs(direction.y)>.5f)return 1;
    if(std::fabs(direction.z)>.5f)return 2;
    return -1;
}
EvaluatedShape MirrorShape(EvaluatedShape shape,int axis){
    for(auto& part:shape.mesh.primitives){
        for(auto& vertex:part.vertices){
            if(axis==0){vertex.position.x=-vertex.position.x;vertex.normal.x=-vertex.normal.x;vertex.tangent.x=-vertex.tangent.x;}
            else if(axis==1){vertex.position.y=-vertex.position.y;vertex.normal.y=-vertex.normal.y;vertex.tangent.y=-vertex.tangent.y;}
            else {vertex.position.z=-vertex.position.z;vertex.normal.z=-vertex.normal.z;vertex.tangent.z=-vertex.tangent.z;}
            // Reflected tangents have reversed handedness.
            vertex.tangent.w=-vertex.tangent.w;
        }
        for(std::size_t i=0;i+2<part.indices.size();i+=3)
            std::swap(part.indices[i+1],part.indices[i+2]);
    }
    auto& matrix=shape.transform.value;
    // Column-major M * T * M; M reflects the chosen axis.
    for(int col=0;col<4;++col)for(int row=0;row<4;++row){
        const bool one=(col==axis),two=(row==axis);
        if(one!=two)matrix[col*4+row]=-matrix[col*4+row];
    }
    shape.name+=".mirror"+std::to_string(axis);
    shape.mesh.name=shape.name;
    return shape;
}

assets::Matrix4 TransformMatrix(const Vector3& p,const Vector3& degrees){
    const float x=degrees.x*kPi/180.0f,y=degrees.y*kPi/180.0f,z=degrees.z*kPi/180.0f;
    const float cx=std::cos(x),sx=std::sin(x),cy=std::cos(y),sy=std::sin(y),cz=std::cos(z),sz=std::sin(z);
    // Column-major Rz * Ry * Rx, matching glTF/OpenGL node transforms.
    assets::Matrix4 m=assets::Matrix4::Identity();
    m.value[0]=cz*cy; m.value[1]=sz*cy; m.value[2]=-sy;
    m.value[4]=cz*sy*sx-sz*cx; m.value[5]=sz*sy*sx+cz*cx; m.value[6]=cy*sx;
    m.value[8]=cz*sy*cx+sz*sx; m.value[9]=sz*sy*cx-cz*sx; m.value[10]=cy*cx;
    m.value[12]=p.x;m.value[13]=p.y;m.value[14]=p.z;return m;
}
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
std::size_t ShipyardModelingSystem::AddPrimitive(ShipyardModelRecipe& recipe,ModelingPrimitiveType t){recipe.primitives.push_back(DefaultPrimitive(t,recipe.primitives.size()));recipe.revision++;recipe.draft=true;recipe.collisionDirty=true;recipe.socketsDirty=true;recipe.surfacesDirty=true;return recipe.primitives.size()-1;}
bool ShipyardModelingSystem::DuplicatePrimitive(ShipyardModelRecipe& r,std::size_t i){if(i>=r.primitives.size())return false;auto copy=r.primitives[i];copy.id="shape."+std::to_string(r.primitives.size()+1);copy.position.x+=std::max(.1f,copy.size.x*.15f);copy.position.y+=std::max(.1f,copy.size.y*.10f);r.primitives.push_back(std::move(copy));r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;return true;}
bool ShipyardModelingSystem::RemovePrimitive(ShipyardModelRecipe& r,std::size_t i){if(i>=r.primitives.size())return false;r.primitives.erase(r.primitives.begin()+static_cast<std::ptrdiff_t>(i));r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;r.surfacesDirty=true;return true;}
bool ShipyardModelingSystem::TranslatePrimitive(ShipyardModelRecipe& r,std::size_t i,const Vector3& d){if(i>=r.primitives.size())return false;auto& p=r.primitives[i];p.position=p.position+d;r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;return true;}
bool ShipyardModelingSystem::RotatePrimitive(ShipyardModelRecipe& r,std::size_t i,const Vector3& d){if(i>=r.primitives.size())return false;auto& p=r.primitives[i];p.rotationDegrees=p.rotationDegrees+d;auto wrap=[](float v){while(v>180)v-=360;while(v<-180)v+=360;return v;};p.rotationDegrees.x=wrap(p.rotationDegrees.x);p.rotationDegrees.y=wrap(p.rotationDegrees.y);p.rotationDegrees.z=wrap(p.rotationDegrees.z);r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;return true;}
bool ShipyardModelingSystem::ScalePrimitive(ShipyardModelRecipe& r,std::size_t i,const Vector3& d){if(i>=r.primitives.size())return false;auto& p=r.primitives[i];p.size.x=std::max(.01f,p.size.x*(1.0f+d.x));p.size.y=std::max(.01f,p.size.y*(1.0f+d.y));p.size.z=std::max(.01f,p.size.z*(1.0f+d.z));r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;r.surfacesDirty=true;return true;}
bool ShipyardModelingSystem::StretchPrimitive(ShipyardModelRecipe& r,std::size_t i,const Vector3& d,bool symmetric){if(i>=r.primitives.size())return false;auto& p=r.primitives[i];const Vector3 old=p.size;p.size.x=std::max(.01f,p.size.x+d.x);p.size.y=std::max(.01f,p.size.y+d.y);p.size.z=std::max(.01f,p.size.z+d.z);if(!symmetric){p.position.x+=d.x*.5f;p.position.y+=d.y*.5f;p.position.z+=d.z*.5f;}if(old.x==p.size.x&&old.y==p.size.y&&old.z==p.size.z)return false;r.revision++;r.draft=true;r.collisionDirty=true;r.socketsDirty=true;r.surfacesDirty=true;return true;}
bool ShipyardModelingSystem::AddModifier(ShipyardModelRecipe& r,ModelingModifier m){if(m.id.empty())m.id="modifier."+std::to_string(r.modifiers.size()+1);r.modifiers.push_back(std::move(m));r.revision++;r.draft=true;r.collisionDirty=true;return true;}
bool ShipyardModelingSystem::AssignSemanticPurpose(ShipyardModelRecipe& r,SemanticObjectPurpose purpose,const WorldScaleProfile& scale){
    auto object=AuthoringStandardsSystem::DefaultObject(purpose,scale);
    object.id=r.recipeId.empty()?std::string("modeled.object"):r.recipeId;
    if(!r.primitives.empty()){
        Vector3 mn{1.0e9f,1.0e9f,1.0e9f},mx{-1.0e9f,-1.0e9f,-1.0e9f};
        for(const auto& p:r.primitives){const Vector3 half=p.size*.5f;mn.x=std::min(mn.x,p.position.x-half.x);mn.y=std::min(mn.y,p.position.y-half.y);mn.z=std::min(mn.z,p.position.z-half.z);mx.x=std::max(mx.x,p.position.x+half.x);mx.y=std::max(mx.y,p.position.y+half.y);mx.z=std::max(mx.z,p.position.z+half.z);}
        object.sizeMeters={std::max(.01f,mx.x-mn.x),std::max(.01f,mx.y-mn.y),std::max(.01f,mx.z-mn.z)};
    }
    r.semanticObject=std::move(object);r.semanticPurposeAssigned=true;r.revision++;r.draft=true;return true;
}
ShipyardModelingValidation ShipyardModelingSystem::Validate(const ShipyardModelRecipe& r){ShipyardModelingValidation v;if(r.primitives.empty()&&r.sourceAssetId.empty())v.errors.push_back("Model has no source geometry or authored shapes");for(const auto&p:r.primitives){if(p.id.empty())v.errors.push_back("Shape is missing an id");if(p.size.x<=0||p.size.y<=0||p.size.z<=0)v.errors.push_back("Shape has non-positive dimensions: "+p.id);if((p.type==ModelingPrimitiveType::Cylinder||p.type==ModelingPrimitiveType::Cone||p.type==ModelingPrimitiveType::Pipe)&&p.radialSegments<6)v.errors.push_back("Radial shape has too few segments: "+p.id);}if(r.semanticPurposeAssigned){const auto semantic=AuthoringStandardsSystem::ValidateObject(r.semanticObject);v.errors.insert(v.errors.end(),semantic.errors.begin(),semantic.errors.end());v.warnings.insert(v.warnings.end(),semantic.warnings.begin(),semantic.warnings.end());}else v.warnings.push_back("Modeled object has no semantic gameplay purpose; assign seat/table/storage/console/door/etc. before production certification");std::size_t evaluatedShapes=r.primitives.size();
for(const auto& modifier:r.modifiers)if(modifier.enabled){
    if(modifier.type==ModelingModifierType::Mirror){
        if(MirrorAxis(modifier.vector)<0)v.errors.push_back("Mirror modifier needs an explicit axis");
        if(evaluatedShapes>128/2)v.errors.push_back("Modifier stack exceeds 128 evaluated shapes");
        else evaluatedShapes*=2;
    }else if(modifier.type==ModelingModifierType::LinearArray){
        if(modifier.count<1||modifier.count>32)v.errors.push_back("Linear array count must be 1..32");
        if(modifier.count && evaluatedShapes>128/modifier.count)v.errors.push_back("Modifier stack exceeds 128 evaluated shapes");
        else evaluatedShapes*=modifier.count;
    }else{
        // No UI may claim a bevel/Boolean exists merely because its data was stored.
        v.errors.push_back(std::string("Modifier not evaluated by canonical baker: ")+ModifierName(modifier.type));
    }
}
for(const auto& p:r.primitives){
    if(!std::isfinite(p.size.x)||!std::isfinite(p.size.y)||!std::isfinite(p.size.z))
        v.errors.push_back("Non-finite dimensions: "+p.id);
    const bool hollow=p.type==ModelingPrimitiveType::Tube||p.type==ModelingPrimitiveType::Ring||p.type==ModelingPrimitiveType::TurretRing||p.type==ModelingPrimitiveType::Pipe;
    if(hollow && (!(p.wallThickness>0)||p.wallThickness>=std::min(p.size.x,p.size.z)*.5f))
        v.errors.push_back("Hollow shape wall thickness invalid: "+p.id);
}
if(r.collisionDirty)v.warnings.push_back("Collision needs regeneration before certification");
if(r.socketsDirty)v.warnings.push_back("Socket frames need review after modeling changes");
if(r.surfacesDirty)v.warnings.push_back("Surface semantics need review after topology changes");
v.valid=v.errors.empty();return v;}
std::string ShipyardModelingSystem::DerivedAssetId(const ShipyardModelRecipe&r,const std::string& requested){if(!requested.empty())return requested;std::ostringstream o;o<<"subspace.modeled."<<(r.recipeId.empty()?"module":r.recipeId)<<".r"<<r.revision;std::string s=o.str();for(char&c:s)if(!(std::isalnum(static_cast<unsigned char>(c))||c=='.'||c=='_'||c=='-'))c='_';return s;}
assets::CanonicalAsset ShipyardModelingSystem::BakeCanonicalAsset(const ShipyardModelRecipe&r,const std::string& id){assets::CanonicalAsset a;a.assetId=DerivedAssetId(r,id);a.provenance.sourcePath="shipyard://model/"+r.recipeId;a.provenance.sourceFormat="SUBSPACE_MODEL_RECIPE";a.provenance.importer="Subspace Shipyard Model Workspace";a.provenance.importerVersion="2";a.provenance.importPolicy=r.semanticPurposeAssigned?std::string("NON_DESTRUCTIVE_RECIPE_BAKE;PURPOSE=")+AuthoringStandardsSystem::PurposeName(r.semanticObject.purpose):"NON_DESTRUCTIVE_RECIPE_BAKE;PURPOSE=UNASSIGNED";if(!Validate(r).valid){a.provenance.importPolicy="REJECTED_INVALID_MODEL";return a;}std::vector<EvaluatedShape> shapes;
for(const auto& p:r.primitives)shapes.push_back({MeshFor(p),TransformMatrix(p.position,p.rotationDegrees),p.id});
for(const auto& modifier:r.modifiers){
    if(!modifier.enabled)continue;
    if(modifier.type==ModelingModifierType::Mirror){
        const auto original=shapes.size();
        for(std::size_t i=0;i<original;++i)shapes.push_back(MirrorShape(shapes[i],MirrorAxis(modifier.vector)));
    }else if(modifier.type==ModelingModifierType::LinearArray){
        const auto original=shapes.size();
        for(std::uint32_t count=1;count<modifier.count;++count){
            for(std::size_t i=0;i<original;++i){
                auto shape=shapes[i];
                shape.transform.value[12]+=modifier.vector.x*count;
                shape.transform.value[13]+=modifier.vector.y*count;
                shape.transform.value[14]+=modifier.vector.z*count;
                shape.name+=".array"+std::to_string(count);
                shape.mesh.name=shape.name;
                shapes.push_back(std::move(shape));
            }
        }
    }
}
for(auto& shape:shapes){
    const auto mi=static_cast<assets::AssetIndex>(a.meshes.size());
    a.meshes.push_back(std::move(shape.mesh));
    assets::CanonicalNode node;node.name=shape.name;node.meshIndex=mi;node.localTransform=shape.transform;
    if(r.semanticPurposeAssigned&&a.nodes.empty())node.extras["subspace.semanticPurpose"]=AuthoringStandardsSystem::PurposeName(r.semanticObject.purpose);
    a.nodes.push_back(std::move(node));
}assets::ModuleDefinition m;m.moduleId=a.assetId;m.role=assets::ModuleRole::Unknown;m.size=assets::SocketSize::M;m.rootNodeIndex=a.nodes.empty()?assets::kInvalidAssetIndex:0;a.modules.push_back(std::move(m));return a;}

} // namespace subspace
