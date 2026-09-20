#include "studio/StudioModelDocumentCodec.h"
#include "modeling/ShipyardModelingSystem.h"

#include <filesystem>
#include <iostream>

using namespace subspace;

int main(){
    int fail=0;auto check=[&](bool ok,const char* label){if(!ok){std::cerr<<"FAIL "<<label<<"\n";++fail;}};
    ShipyardModelingState state;
    const auto box=ShipyardModelingSystem::AddPrimitive(state.recipe,ModelingPrimitiveType::Box);
    state.recipe.primitives[box].position={1.25f,-2.0f,.5f};
    state.recipe.primitives[box].rotationDegrees={15.0f,25.0f,35.0f};
    state.recipe.primitives[box].size={2.0f,3.0f,4.0f};
    const auto wing=ShipyardModelingSystem::AddPrimitive(state.recipe,ModelingPrimitiveType::Wing);
    state.selectedPrimitiveIndex=wing;state.selectedPrimitive=ModelingPrimitiveType::Wing;
    ShipyardModelingSystem::AssignSemanticPurpose(state.recipe,SemanticObjectPurpose::Fixture);
    ModelingModifier mirror;mirror.type=ModelingModifierType::Mirror;mirror.vector={1,0,0};
    ShipyardModelingSystem::AddModifier(state.recipe,mirror);
    state.savedRevision=1;

    const auto root=std::filesystem::temp_directory_path()/"subspace_studio_model_codec_test";
    std::error_code ec;std::filesystem::remove_all(root,ec);std::filesystem::create_directories(root,ec);
    const auto path=root/"fixture.subspace_studio";std::string error;
    check(StudioModelDocumentCodec::Save(path,state,false,error),"initial save");
    ShipyardModelingState loaded;
    check(StudioModelDocumentCodec::Load(path,loaded,error),"load");
    check(loaded.recipe.primitives.size()==2,"primitive count");
    check(loaded.recipe.modifiers.size()==1,"modifier count");
    check(loaded.recipe.primitives[0].position.x==1.25f,"position roundtrip");
    check(loaded.recipe.primitives[0].rotationDegrees.z==35.0f,"rotation roundtrip");
    check(loaded.recipe.semanticPurposeAssigned,"semantic roundtrip");
    check(loaded.savedRevision==loaded.recipe.revision,"loaded document clean revision");
    check(!StudioModelDocumentCodec::Save(path,state,false,error),"save-as overwrite blocked");
    loaded.recipe.primitives[0].position.x=9.0f;loaded.recipe.revision++;
    check(StudioModelDocumentCodec::Save(path,loaded,true,error),"ordinary overwrite save");
    ShipyardModelingState reopened;check(StudioModelDocumentCodec::Load(path,reopened,error),"reopen overwrite");
    check(reopened.recipe.primitives[0].position.x==9.0f,"overwrite value survived");
    std::filesystem::remove_all(root,ec);
    std::cout<<"Studio model document codec: "<<(fail?"FAIL":"PASS")<<"\n";return fail?1:0;
}
