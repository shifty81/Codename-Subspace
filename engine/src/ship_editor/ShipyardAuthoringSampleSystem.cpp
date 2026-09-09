#include "ship_editor/ShipyardAuthoringSampleSystem.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {

std::string JsonString(const std::string& value){
    std::ostringstream o;o<<'"';
    for(unsigned char c:value){
        switch(c){
            case '"':o<<"\\\"";break;
            case '\\':o<<"\\\\";break;
            case '\n':o<<"\\n";break;
            case '\r':o<<"\\r";break;
            case '\t':o<<"\\t";break;
            default:
                if(c<0x20){o<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<static_cast<int>(c)<<std::dec<<std::setfill(' ');}
                else o<<static_cast<char>(c);
                break;
        }
    }
    o<<'"';return o.str();
}

const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){
    for(const auto& r:catalog)if(r.source.moduleId==id)return &r;
    return nullptr;
}

bool Different(float a,float b,float eps=.0005f){return std::fabs(a-b)>eps;}

std::string InstanceId(std::size_t index){std::ostringstream o;o<<"M"<<std::setw(4)<<std::setfill('0')<<(index+1);return o.str();}

void WritePlacement(std::ostringstream& o,const VisualModulePlacement& p,const std::vector<ShipyardModuleRecord>& catalog,std::size_t index){
    const auto* r=FindRecord(catalog,p.moduleId);
    o<<"{\"instanceId\":"<<JsonString(InstanceId(index))
     <<",\"moduleId\":"<<JsonString(p.moduleId)
     <<",\"class\":"<<JsonString(r?ShipyardModuleSystem::ClassName(r->moduleClass):"UNKNOWN")
     <<",\"semantic\":"<<JsonString(r?ShipyardModuleSystem::SemanticName(r->semantic):"UNKNOWN")
     <<",\"position\":["<<p.x<<','<<p.y<<','<<p.z<<']'
     <<",\"rotationDeg\":["<<p.pitchDegrees<<','<<p.yawDegrees<<','<<p.rollDegrees<<']'
     <<",\"scale\":["<<p.scaleX<<','<<p.scaleY<<','<<p.scaleZ<<']'
     <<",\"mirrorX\":"<<(p.mirrorX?"true":"false")<<",\"mirrorY\":"<<(p.mirrorY?"true":"false")<<",\"mirrorZ\":"<<(p.mirrorZ?"true":"false")<<'}';
}

void WriteRecipe(std::ostringstream& o,const ProceduralShipVisualRecipe& r,const std::vector<ShipyardModuleRecord>& catalog){
    o<<"{\"role\":"<<JsonString(r.role)<<",\"seed\":"<<r.seed<<",\"generatorVersion\":"<<JsonString(r.forwardAuthority)
     <<",\"modules\":[";
    for(std::size_t i=0;i<r.modules.size();++i){if(i)o<<',';WritePlacement(o,r.modules[i],catalog,i);}o<<"],\"attachments\":[";
    for(std::size_t i=0;i<r.attachments.size();++i){const auto& a=r.attachments[i];if(i)o<<',';
        o<<"{\"parent\":"<<a.parentModuleIndex<<",\"child\":"<<a.childModuleIndex
         <<",\"parentSocket\":"<<JsonString(a.parentSocket)<<",\"childSocket\":"<<JsonString(a.childSocket)
         <<",\"certified\":"<<(a.certified?"true":"false")<<'}';
    }
    o<<"]}";
}

} // namespace

std::string ShipyardAuthoringSampleSystem::Serialize(const ProceduralShipVisualRecipe& baseline,
                                                      const ProceduralShipVisualRecipe& edited,
                                                      const ShipAppearanceState& appearance,
                                                      const std::vector<ShipyardModuleRecord>& catalog,
                                                      const std::vector<std::string>& validationErrors,
                                                      const std::vector<std::string>& validationWarnings,
                                                      const std::string& author){
    std::ostringstream o;o<<std::fixed<<std::setprecision(6);
    o<<"{\n  \"schema\": \"subspace.shipyard_authoring_sample\",\n  \"version\": 1,\n";
    o<<"  \"author\": "<<JsonString(author)<<",\n";
    o<<"  \"reviewRequired\": true,\n";
    o<<"  \"generatorCertification\": "<<(validationErrors.empty()?"\"PASS\"":"\"DRAFT\"")<<",\n";
    o<<"  \"intent\": \"PLAYER_GENERATOR_REFINEMENT_EXAMPLE\",\n";
    o<<"  \"appearance\": {\"primary\":["<<appearance.primary.r<<','<<appearance.primary.g<<','<<appearance.primary.b
     <<"],\"secondary\":["<<appearance.secondary.r<<','<<appearance.secondary.g<<','<<appearance.secondary.b
     <<"],\"accent\":["<<appearance.trim.r<<','<<appearance.trim.g<<','<<appearance.trim.b<<"]},\n";
    o<<"  \"baseline\": ";WriteRecipe(o,baseline,catalog);o<<",\n";
    o<<"  \"edited\": ";WriteRecipe(o,edited,catalog);o<<",\n";
    o<<"  \"deltas\": [\n";
    bool first=true;const std::size_t shared=std::min(baseline.modules.size(),edited.modules.size());
    auto emit=[&](const std::string& body){if(!first)o<<",\n";first=false;o<<"    "<<body;};
    for(std::size_t i=0;i<shared;++i){
        const auto& a=baseline.modules[i];const auto& b=edited.modules[i];
        if(a.moduleId!=b.moduleId)emit("{\"op\":\"REPLACE\",\"instanceId\":"+JsonString(InstanceId(i))+",\"from\":"+JsonString(a.moduleId)+",\"to\":"+JsonString(b.moduleId)+"}");
        const bool moved=Different(a.x,b.x)||Different(a.y,b.y)||Different(a.z,b.z);
        const bool rotated=Different(a.pitchDegrees,b.pitchDegrees)||Different(a.yawDegrees,b.yawDegrees)||Different(a.rollDegrees,b.rollDegrees);
        const bool scaled=Different(a.scaleX,b.scaleX)||Different(a.scaleY,b.scaleY)||Different(a.scaleZ,b.scaleZ);
        const bool mirrored=a.mirrorX!=b.mirrorX;
        if(moved||rotated||scaled||mirrored){
            std::ostringstream d;d<<"{\"op\":\"TRANSFORM\",\"instanceId\":"<<JsonString(InstanceId(i))
             <<",\"moved\":"<<(moved?"true":"false")<<",\"rotated\":"<<(rotated?"true":"false")
             <<",\"scaled\":"<<(scaled?"true":"false")<<",\"mirrored\":"<<(mirrored?"true":"false")
             <<",\"toPosition\":["<<b.x<<','<<b.y<<','<<b.z<<"],\"toRotationDeg\":["<<b.pitchDegrees<<','<<b.yawDegrees<<','<<b.rollDegrees<<"]}";emit(d.str());
        }
    }
    for(std::size_t i=shared;i<edited.modules.size();++i){std::ostringstream d;d<<"{\"op\":\"ADD\",\"instanceId\":"<<JsonString(InstanceId(i))<<",\"moduleId\":"<<JsonString(edited.modules[i].moduleId)<<'}';emit(d.str());}
    for(std::size_t i=shared;i<baseline.modules.size();++i){std::ostringstream d;d<<"{\"op\":\"REMOVE\",\"instanceId\":"<<JsonString(InstanceId(i))<<",\"moduleId\":"<<JsonString(baseline.modules[i].moduleId)<<'}';emit(d.str());}
    o<<"\n  ],\n";
    o<<"  \"validation\": {\"errors\":[";for(std::size_t i=0;i<validationErrors.size();++i){if(i)o<<',';o<<JsonString(validationErrors[i]);}
    o<<"],\"warnings\":[";for(std::size_t i=0;i<validationWarnings.size();++i){if(i)o<<',';o<<JsonString(validationWarnings[i]);}o<<"]}\n";
    o<<"}\n";return o.str();
}

bool ShipyardAuthoringSampleSystem::Save(const ProceduralShipVisualRecipe& baseline,
                                         const ProceduralShipVisualRecipe& edited,
                                         const ShipAppearanceState& appearance,
                                         const std::vector<ShipyardModuleRecord>& catalog,
                                         const std::vector<std::string>& validationErrors,
                                         const std::vector<std::string>& validationWarnings,
                                         const std::string& path,
                                         std::string* error,
                                         const std::string& author){
    std::ofstream f(path,std::ios::binary|std::ios::trunc);if(!f){if(error)*error="could not open authoring sample path";return false;}
    f<<Serialize(baseline,edited,appearance,catalog,validationErrors,validationWarnings,author);
    if(!f.good()){if(error)*error="failed while writing authoring sample";return false;}return true;
}

} // namespace subspace
