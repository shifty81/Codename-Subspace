#include "studio/StudioModelDocumentCodec.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace subspace {
namespace {

bool ReadBool(std::istringstream& in,bool& value){
    int v=0;if(!(in>>v)||v<0||v>1)return false;value=v!=0;return true;
}

template <typename Enum>
bool ReadEnum(std::istringstream& in,Enum& value,int maximum){
    int raw=0;if(!(in>>raw)||raw<0||raw>maximum)return false;
    value=static_cast<Enum>(raw);return true;
}

bool Equivalent(const ShipyardModelingState& a,const ShipyardModelingState& b){
    const auto& x=a.recipe;const auto& y=b.recipe;
    if(x.recipeId!=y.recipeId||x.displayName!=y.displayName||x.sourceAssetId!=y.sourceAssetId||
       x.selectionMode!=y.selectionMode||x.primitives.size()!=y.primitives.size()||
       x.modifiers.size()!=y.modifiers.size()||x.semanticPurposeAssigned!=y.semanticPurposeAssigned||
       x.revision!=y.revision)return false;
    for(std::size_t i=0;i<x.primitives.size();++i){
        const auto& p=x.primitives[i];const auto& q=y.primitives[i];
        if(p.id!=q.id||p.type!=q.type||p.size.x!=q.size.x||p.size.y!=q.size.y||p.size.z!=q.size.z||
           p.position.x!=q.position.x||p.position.y!=q.position.y||p.position.z!=q.position.z||
           p.rotationDegrees.x!=q.rotationDegrees.x||p.rotationDegrees.y!=q.rotationDegrees.y||p.rotationDegrees.z!=q.rotationDegrees.z||
           p.radialSegments!=q.radialSegments||p.bevel!=q.bevel||p.wallThickness!=q.wallThickness||
           p.surfaceSemantic!=q.surfaceSemantic)return false;
    }
    for(std::size_t i=0;i<x.modifiers.size();++i){
        const auto& p=x.modifiers[i];const auto& q=y.modifiers[i];
        if(p.id!=q.id||p.type!=q.type||p.vector.x!=q.vector.x||p.vector.y!=q.vector.y||p.vector.z!=q.vector.z||
           p.amount!=q.amount||p.count!=q.count||p.enabled!=q.enabled||
           p.preserveSockets!=q.preserveSockets||p.preserveFunctionalRegions!=q.preserveFunctionalRegions)return false;
    }
    if(x.semanticPurposeAssigned){
        const auto& p=x.semanticObject;const auto& q=y.semanticObject;
        if(p.id!=q.id||p.purpose!=q.purpose||p.sizeMeters.x!=q.sizeMeters.x||p.sizeMeters.y!=q.sizeMeters.y||p.sizeMeters.z!=q.sizeMeters.z||
           p.interactionPoint.x!=q.interactionPoint.x||p.interactionPoint.y!=q.interactionPoint.y||p.interactionPoint.z!=q.interactionPoint.z||
           p.facingDirection.x!=q.facingDirection.x||p.facingDirection.y!=q.facingDirection.y||p.facingDirection.z!=q.facingDirection.z||
           p.interactionReachMeters!=q.interactionReachMeters||p.approachClearanceMeters!=q.approachClearanceMeters||
           p.minimumHeadClearanceMeters!=q.minimumHeadClearanceMeters||p.snapStepMeters!=q.snapStepMeters||
           p.requiresFloorContact!=q.requiresFloorContact||p.requiresHumanClearance!=q.requiresHumanClearance||
           p.sitInteraction!=q.sitInteraction||p.storageInteraction!=q.storageInteraction||
           p.consoleInteraction!=q.consoleInteraction)return false;
    }
    return a.selectedPrimitive==b.selectedPrimitive&&a.selectionMode==b.selectionMode&&
           a.selectedPurpose==b.selectedPurpose&&a.selectedPrimitiveIndex==b.selectedPrimitiveIndex&&
           a.stretchStep==b.stretchStep&&a.symmetricStretch==b.symmetricStretch&&
           a.autoCollision==b.autoCollision&&a.liveCanonicalPreview==b.liveCanonicalPreview;
}

std::string Encode(const ShipyardModelingState& state){
    const auto& r=state.recipe;
    std::ostringstream out;out.setf(std::ios::scientific);out.precision(9);
    out<<"SUBSPACE_STUDIO_DOCUMENT "<<StudioModelDocumentCodec::kSchemaVersion<<"\n";
    out<<"KIND MODEL_ASSET\n";
    out<<"RECIPE "<<std::quoted(r.recipeId)<<' '<<std::quoted(r.displayName)<<' '<<std::quoted(r.sourceAssetId)<<' '
       <<static_cast<int>(r.selectionMode)<<' '<<r.semanticPurposeAssigned<<' '<<r.draft<<' '
       <<r.collisionDirty<<' '<<r.socketsDirty<<' '<<r.surfacesDirty<<' '<<r.revision<<"\n";
    out<<"STATE "<<static_cast<int>(state.selectedPrimitive)<<' '<<static_cast<int>(state.selectionMode)<<' '
       <<static_cast<int>(state.selectedPurpose)<<' '<<state.selectedPrimitiveIndex<<' '<<state.stretchStep<<' '
       <<state.symmetricStretch<<' '<<state.autoCollision<<' '<<state.liveCanonicalPreview<<"\n";
    for(const auto& p:r.primitives){
        out<<"PRIMITIVE "<<std::quoted(p.id)<<' '<<static_cast<int>(p.type)<<' '
           <<p.size.x<<' '<<p.size.y<<' '<<p.size.z<<' '
           <<p.position.x<<' '<<p.position.y<<' '<<p.position.z<<' '
           <<p.rotationDegrees.x<<' '<<p.rotationDegrees.y<<' '<<p.rotationDegrees.z<<' '
           <<p.radialSegments<<' '<<p.bevel<<' '<<p.wallThickness<<' '<<std::quoted(p.surfaceSemantic)<<"\n";
    }
    for(const auto& m:r.modifiers){
        out<<"MODIFIER "<<std::quoted(m.id)<<' '<<static_cast<int>(m.type)<<' '
           <<m.vector.x<<' '<<m.vector.y<<' '<<m.vector.z<<' '<<m.amount<<' '<<m.count<<' '
           <<m.enabled<<' '<<m.preserveSockets<<' '<<m.preserveFunctionalRegions<<"\n";
    }
    if(r.semanticPurposeAssigned){
        const auto& s=r.semanticObject;
        out<<"SEMANTIC "<<std::quoted(s.id)<<' '<<static_cast<int>(s.purpose)<<' '
           <<s.sizeMeters.x<<' '<<s.sizeMeters.y<<' '<<s.sizeMeters.z<<' '
           <<s.interactionPoint.x<<' '<<s.interactionPoint.y<<' '<<s.interactionPoint.z<<' '
           <<s.facingDirection.x<<' '<<s.facingDirection.y<<' '<<s.facingDirection.z<<' '
           <<s.interactionReachMeters<<' '<<s.approachClearanceMeters<<' '<<s.minimumHeadClearanceMeters<<' '
           <<s.snapStepMeters<<' '<<s.requiresFloorContact<<' '<<s.requiresHumanClearance<<' '
           <<s.sitInteraction<<' '<<s.storageInteraction<<' '<<s.consoleInteraction<<"\n";
    }
    out<<"END\n";return out.str();
}

bool Decode(std::istream& stream,ShipyardModelingState& state,std::string& error){
    state=ShipyardModelingState{};
    std::string line;bool header=false,kind=false,recipe=false,ended=false;
    while(std::getline(stream,line)){
        if(line.empty()||line[0]=='#')continue;
        std::istringstream in(line);std::string tag;if(!(in>>tag))continue;
        if(tag=="SUBSPACE_STUDIO_DOCUMENT"){
            int version=0;if(!(in>>version)||version!=StudioModelDocumentCodec::kSchemaVersion){error="Unsupported Studio document schema";return false;}header=true;
        }else if(tag=="KIND"){
            std::string value;if(!(in>>value)||value!="MODEL_ASSET"){error="Studio document kind is not MODEL_ASSET";return false;}kind=true;
        }else if(tag=="RECIPE"){
            auto& r=state.recipe;int selection=0;int semantic=0,draft=0,collision=0,sockets=0,surfaces=0;
            if(!(in>>std::quoted(r.recipeId)>>std::quoted(r.displayName)>>std::quoted(r.sourceAssetId)>>selection>>semantic>>draft>>collision>>sockets>>surfaces>>r.revision)){
                error="Malformed RECIPE record";return false;
            }
            if(selection<0||selection>static_cast<int>(ModelingSelectionMode::Face)){error="Invalid model selection mode";return false;}
            r.selectionMode=static_cast<ModelingSelectionMode>(selection);r.semanticPurposeAssigned=semantic!=0;r.draft=draft!=0;
            r.collisionDirty=collision!=0;r.socketsDirty=sockets!=0;r.surfacesDirty=surfaces!=0;recipe=true;
        }else if(tag=="STATE"){
            int primitive=0,selection=0,purpose=0;int symmetric=0,collision=0,preview=0;
            if(!(in>>primitive>>selection>>purpose>>state.selectedPrimitiveIndex>>state.stretchStep>>symmetric>>collision>>preview)){
                error="Malformed STATE record";return false;
            }
            if(primitive<0||primitive>static_cast<int>(ModelingPrimitiveType::Pipe)||
               selection<0||selection>static_cast<int>(ModelingSelectionMode::Face)||
               purpose<0||purpose>static_cast<int>(SemanticObjectPurpose::Fixture)){
                error="Invalid Studio model state enum";return false;
            }
            state.selectedPrimitive=static_cast<ModelingPrimitiveType>(primitive);
            state.selectionMode=static_cast<ModelingSelectionMode>(selection);
            state.selectedPurpose=static_cast<SemanticObjectPurpose>(purpose);
            state.symmetricStretch=symmetric!=0;state.autoCollision=collision!=0;state.liveCanonicalPreview=preview!=0;
        }else if(tag=="PRIMITIVE"){
            ModelingPrimitiveDefinition p;int type=0;
            if(!(in>>std::quoted(p.id)>>type>>p.size.x>>p.size.y>>p.size.z>>p.position.x>>p.position.y>>p.position.z>>
                 p.rotationDegrees.x>>p.rotationDegrees.y>>p.rotationDegrees.z>>p.radialSegments>>p.bevel>>p.wallThickness>>std::quoted(p.surfaceSemantic))){
                error="Malformed PRIMITIVE record";return false;
            }
            if(type<0||type>static_cast<int>(ModelingPrimitiveType::Pipe)){error="Invalid primitive type";return false;}
            p.type=static_cast<ModelingPrimitiveType>(type);state.recipe.primitives.push_back(std::move(p));
        }else if(tag=="MODIFIER"){
            ModelingModifier m;int type=0;int enabled=0,preserveSockets=0,preserveRegions=0;
            if(!(in>>std::quoted(m.id)>>type>>m.vector.x>>m.vector.y>>m.vector.z>>m.amount>>m.count>>enabled>>preserveSockets>>preserveRegions)){
                error="Malformed MODIFIER record";return false;
            }
            if(type<0||type>static_cast<int>(ModelingModifierType::BooleanIntersect)){error="Invalid modifier type";return false;}
            m.type=static_cast<ModelingModifierType>(type);m.enabled=enabled!=0;m.preserveSockets=preserveSockets!=0;m.preserveFunctionalRegions=preserveRegions!=0;
            state.recipe.modifiers.push_back(std::move(m));
        }else if(tag=="SEMANTIC"){
            auto& s=state.recipe.semanticObject;int purpose=0;int floor=0,human=0,sit=0,storage=0,console=0;
            if(!(in>>std::quoted(s.id)>>purpose>>s.sizeMeters.x>>s.sizeMeters.y>>s.sizeMeters.z>>
                 s.interactionPoint.x>>s.interactionPoint.y>>s.interactionPoint.z>>
                 s.facingDirection.x>>s.facingDirection.y>>s.facingDirection.z>>
                 s.interactionReachMeters>>s.approachClearanceMeters>>s.minimumHeadClearanceMeters>>s.snapStepMeters>>
                 floor>>human>>sit>>storage>>console)){
                error="Malformed SEMANTIC record";return false;
            }
            if(purpose<0||purpose>static_cast<int>(SemanticObjectPurpose::Fixture)){error="Invalid semantic purpose";return false;}
            s.purpose=static_cast<SemanticObjectPurpose>(purpose);s.requiresFloorContact=floor!=0;s.requiresHumanClearance=human!=0;
            s.sitInteraction=sit!=0;s.storageInteraction=storage!=0;s.consoleInteraction=console!=0;
            state.recipe.semanticPurposeAssigned=true;
        }else if(tag=="END"){
            ended=true;break;
        }else{
            error="Unknown Studio document record: "+tag;return false;
        }
    }
    if(!header||!kind||!recipe||!ended){error="Incomplete Studio model document";return false;}
    if(state.recipe.primitives.empty()&&state.recipe.sourceAssetId.empty()){
        error="Studio model document contains no editable/source geometry";return false;
    }
    if(!state.recipe.primitives.empty())state.selectedPrimitiveIndex=std::min(state.selectedPrimitiveIndex,state.recipe.primitives.size()-1);
    else state.selectedPrimitiveIndex=0;
    state.selectionMode=state.recipe.selectionMode;
    state.savedRevision=state.recipe.revision;
    state.status="Model document loaded";
    return true;
}

}

bool StudioModelDocumentCodec::IsStudioModelPath(const std::filesystem::path& path) noexcept {
    return path.extension()==".subspace_studio";
}

bool StudioModelDocumentCodec::Load(const std::filesystem::path& source,ShipyardModelingState& state,std::string& error){
    error.clear();std::ifstream in(source,std::ios::binary);
    if(!in){error="Cannot open Studio model document";return false;}
    if(!Decode(in,state,error))return false;
    return true;
}

bool StudioModelDocumentCodec::Save(const std::filesystem::path& destination,const ShipyardModelingState& state,bool overwrite,std::string& error){
    error.clear();
    if(destination.empty()||!IsStudioModelPath(destination)){error="Studio model documents use .subspace_studio";return false;}
    if(state.recipe.primitives.empty()&&state.recipe.sourceAssetId.empty()){error="Model document has no geometry";return false;}
    std::error_code ec;const auto parent=destination.parent_path();if(!parent.empty())std::filesystem::create_directories(parent,ec);
    if(ec){error="Cannot create Studio document directory: "+ec.message();return false;}
    const bool exists=std::filesystem::exists(destination,ec);if(ec){error="Cannot inspect Studio document target: "+ec.message();return false;}
    if(exists&&!overwrite){error="Save As target exists: choose a different name";return false;}
    const auto pending=std::filesystem::path(destination.string()+".studio_pending");
    const auto recovery=std::filesystem::path(destination.string()+".studio_recovery");
    if(std::filesystem::exists(pending,ec)||std::filesystem::exists(recovery,ec)||ec){error="Pending/recovery file exists; review before saving again";return false;}
    ShipyardModelingState clean=state;clean.savedRevision=clean.recipe.revision;
    {
        std::ofstream out(pending,std::ios::binary|std::ios::trunc);if(!out){error="Cannot create pending Studio document";return false;}
        const auto encoded=Encode(clean);out.write(encoded.data(),static_cast<std::streamsize>(encoded.size()));out.flush();
        if(!out){error="Failed while writing pending Studio document";out.close();std::filesystem::remove(pending,ec);return false;}
    }
    ShipyardModelingState verified;std::string verifyError;
    if(!Load(pending,verified,verifyError)||!Equivalent(clean,verified)){
        error="Studio model round-trip validation failed"+(verifyError.empty()?std::string{}:std::string(": ")+verifyError);
        std::filesystem::remove(pending,ec);return false;
    }
    if(exists){std::filesystem::rename(destination,recovery,ec);if(ec){error="Cannot stage existing Studio document: "+ec.message();std::filesystem::remove(pending,ec);return false;}}
    std::filesystem::rename(pending,destination,ec);
    if(ec){
        error="Cannot finalize Studio document: "+ec.message();
        if(exists){std::error_code rollback;std::filesystem::rename(recovery,destination,rollback);if(rollback)error+="; recovery preserved at "+recovery.string();}
        return false;
    }
    if(exists){std::filesystem::remove(recovery,ec);if(ec)error="Saved; previous version remains for review at "+recovery.string();}
    return true;
}

} // namespace subspace
