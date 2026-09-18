#include "modeling/ShipyardModelingSystem.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {int checks=0,fail=0;void Check(bool ok,const char* name){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<name<<'\n';}}}
int main(){
    ShipyardModelRecipe wing;wing.recipeId="test.wing";
    ShipyardModelingSystem::AddPrimitive(wing,ModelingPrimitiveType::Wing);
    const auto wingAsset=ShipyardModelingSystem::BakeCanonicalAsset(wing,{});
    Check(wingAsset.meshes.size()==1,"wing produces canonical mesh");
    Check(wingAsset.meshes[0].primitives[0].vertices.size()>24,"wing is not a six-face box");
    Check(!wingAsset.meshes[0].primitives[0].indices.empty(),"wing is triangulated");
    ShipyardModelRecipe hull;hull.recipeId="test.hull";
    ShipyardModelingSystem::AddPrimitive(hull,ModelingPrimitiveType::HullSegment);
    const auto hullAsset=ShipyardModelingSystem::BakeCanonicalAsset(hull,{});
    Check(hullAsset.meshes.size()==1,"hull mesh produced");
    bool tapered=false;
    for(const auto& v:hullAsset.meshes[0].primitives[0].vertices){
       if(v.position.y>2.0f&&std::fabs(v.position.x)<1.1f)tapered=true;
    }
    Check(tapered,"hull has tapered front section");
    ShipyardModelRecipe tube;tube.recipeId="test.pipe";
    ShipyardModelingSystem::AddPrimitive(tube,ModelingPrimitiveType::Pipe);
    const auto tubeAsset=ShipyardModelingSystem::BakeCanonicalAsset(tube,{});
    Check(tubeAsset.meshes.size()==1,"pipe mesh produced");
    float minRad=1000.0f,maxRad=0.0f;
    for(const auto& v:tubeAsset.meshes[0].primitives[0].vertices){
       const float radius=std::sqrt(v.position.x*v.position.x+v.position.z*v.position.z);
       minRad=std::min(minRad,radius);maxRad=std::max(maxRad,radius);
    }
    Check(minRad>0 && minRad<maxRad,"pipe has a finite hole and wall");
    auto outward=[&](const assets::CanonicalMesh& mesh){
        for(const auto& primitive:mesh.primitives)for(std::size_t k=0;k+2<primitive.indices.size();k+=3){
            const auto& a=primitive.vertices[primitive.indices[k]];
            const auto& b=primitive.vertices[primitive.indices[k+1]];
            const auto& c=primitive.vertices[primitive.indices[k+2]];
            const auto u=b.position-a.position,v=c.position-a.position;
            const Vector3 cross{u.y*v.z-u.z*v.y,u.z*v.x-u.x*v.z,u.x*v.y-u.y*v.x};
            const float dot=cross.x*a.normal.x+cross.y*a.normal.y+cross.z*a.normal.z;
            if(dot<-0.00001f)return false;
        }
        return true;
    };
    Check(outward(wingAsset.meshes[0]),"wing winding matches authored normals");
    Check(outward(tubeAsset.meshes[0]),"tube winding matches authored normals");
    tube.primitives[0].wallThickness=100.0f;
    Check(!ShipyardModelingSystem::Validate(tube).valid,"oversized wall rejected");
    const auto rejected=ShipyardModelingSystem::BakeCanonicalAsset(tube,{});
    Check(rejected.meshes.empty(),"invalid pipe cannot bake silently");
    ShipyardModelRecipe mirrored=wing;mirrored.recipeId="test.mirrored";
    mirrored.primitives[0].position.x=2.0f;
    ModelingModifier mirror;mirror.type=ModelingModifierType::Mirror;mirror.vector={1,0,0};
    ShipyardModelingSystem::AddModifier(mirrored,mirror);
    Check(ShipyardModelingSystem::Validate(mirrored).valid,"mirror is evaluated and valid");
    const auto reflected=ShipyardModelingSystem::BakeCanonicalAsset(mirrored,{});
    Check(reflected.meshes.size()==2&&reflected.nodes.size()==2,"mirror emits source and reflected nodes");
    Check(reflected.nodes[0].localTransform.value[12]==2.0f&&reflected.nodes[1].localTransform.value[12]==-2.0f,"mirror reflects placement");
    Check(outward(reflected.meshes[1]),"mirror reverses triangle winding correctly");
    ModelingModifier array;array.type=ModelingModifierType::LinearArray;array.vector={3,0,0};array.count=3;
    ShipyardModelingSystem::AddModifier(mirrored,array);
    const auto repeated=ShipyardModelingSystem::BakeCanonicalAsset(mirrored,{});
    Check(repeated.meshes.size()==6&&repeated.nodes.size()==6,"ordered mirror and array evaluate six nodes");
    Check(repeated.nodes[2].localTransform.value[12]==5.0f,"array offsets source copy");
    Check(repeated.nodes[3].localTransform.value[12]==1.0f,"array offsets mirror copy");
    mirrored.modifiers.back().count=33;
    Check(!ShipyardModelingSystem::Validate(mirrored).valid,"unbounded array rejected");
    ModelingModifier unsupported;unsupported.type=ModelingModifierType::Bevel;
    ShipyardModelingSystem::AddModifier(wing,unsupported);
    Check(!ShipyardModelingSystem::Validate(wing).valid,"unevaluated bevel blocked");
    Check(ShipyardModelingSystem::BakeCanonicalAsset(wing,{}).meshes.empty(),"unevaluated modifier is not silently ignored");
    std::cout<<"Studio geometry: "<<checks-fail<<"/"<<checks<<" assertions passed\n";
    return fail?1:0;
}
