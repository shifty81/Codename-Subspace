#include "ship_editor/ShipyardDesignExchangeSystem.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {

const ShipyardModuleRecord* FindRecord(const std::vector<ShipyardModuleRecord>& catalog,const std::string& id){
    for(const auto& r:catalog) if(r.source.moduleId==id) return &r;
    return nullptr;
}

std::string Lower(std::string value){
    std::transform(value.begin(),value.end(),value.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return value;
}

std::string InstanceId(std::size_t index){
    std::ostringstream o;o<<"M"<<std::setw(4)<<std::setfill('0')<<(index+1);return o.str();
}

std::string JsonString(const std::string& value){
    std::ostringstream o;o<<'"';
    for(unsigned char c:value){
        switch(c){
            case '"':o<<"\\\"";break;
            case '\\':o<<"\\\\";break;
            case '\b':o<<"\\b";break;
            case '\f':o<<"\\f";break;
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

void Vec3(std::ostringstream& o,float x,float y,float z){o<<'['<<x<<','<<y<<','<<z<<']';}

} // namespace

std::string ShipyardDesignExchangeSystem::Serialize(const ShipBlueprintDocument& b,
                                                     const std::vector<ShipyardModuleRecord>& catalog,
                                                     const std::string& toolVersion){
    int hullSections=0,engineCount=0,hardpointCount=0,detailCount=0;
    for(const auto& m:b.recipe.modules){
        if(const auto* r=FindRecord(catalog,m.moduleId)){
            hullSections+=r->moduleClass==ShipyardModuleClass::Hull?1:0;
            engineCount+=r->semantic==ShipyardModuleSemantic::MainEngine?1:0;
            hardpointCount+=r->moduleClass==ShipyardModuleClass::Hardpoint?1:0;
            detailCount+=r->moduleClass==ShipyardModuleClass::Detail?1:0;
        }
    }
    const float detailDensity=b.recipe.modules.empty()?0.0f:static_cast<float>(detailCount)/static_cast<float>(b.recipe.modules.size());

    std::ostringstream o;o<<std::fixed<<std::setprecision(6);
    o<<"{\n";
    o<<"  \"schema\": \"subspace.shipyard_design\",\n";
    o<<"  \"version\": 1,\n";
    o<<"  \"tool\": {\"name\": \"Codename Subspace Native Shipyard\", \"version\": "<<JsonString(toolVersion)<<"},\n";
    o<<"  \"coordinateSystem\": {\"right\": \"+X\", \"forward\": \"+Y\", \"up\": \"+Z\", \"units\": \"meters\", \"sourceObj\": \"X-right/Y-up/+Z-forward\", \"sourceToShipyard\": \"(X,Z,Y)\"},\n";
    o<<"  \"ship\": {\n";
    o<<"    \"name\": "<<JsonString(b.name)<<",\n";
    o<<"    \"author\": "<<JsonString(b.author)<<",\n";
    o<<"    \"role\": "<<JsonString(b.recipe.role)<<",\n";
    o<<"    \"seed\": "<<b.recipe.seed<<",\n";
    o<<"    \"decalCode\": "<<JsonString(b.recipe.decalCode)<<",\n";
    o<<"    \"orientation\": {\"canonicalForward\": \"+Y\", \"visualYawDeg\": "<<b.recipe.forwardVisualYawDegrees
     <<", \"authority\": "<<JsonString(b.recipe.forwardAuthority)<<", \"cockpitModuleIndex\": "<<b.recipe.cockpitModuleIndex<<"},\n";
    o<<"    \"appearance\": {\"primary\": ";Vec3(o,b.appearance.primary.r,b.appearance.primary.g,b.appearance.primary.b);
    o<<", \"secondary\": ";Vec3(o,b.appearance.secondary.r,b.appearance.secondary.g,b.appearance.secondary.b);
    o<<", \"accent\": ";Vec3(o,b.appearance.trim.r,b.appearance.trim.g,b.appearance.trim.b);o<<"},\n";
    o<<"    \"generation\": {\"hullSections\": "<<hullSections<<", \"engineCount\": "<<engineCount<<", \"hardpointCount\": "<<hardpointCount<<", \"detailDensity\": "<<detailDensity<<", \"designerEnhanced\": true, \"globalScale\": 1.000000}\n";
    o<<"  },\n";
    o<<"  \"decals\": [\n";
    for(std::size_t i=0;i<b.appearance.decals.size();++i){const auto& d=b.appearance.decals[i];
        o<<"    {\"id\": "<<JsonString(d.id)<<", \"asset\": "<<JsonString(d.decalAsset)<<", \"moduleIndex\": "<<d.moduleIndex
         <<", \"uv\": ["<<d.u<<','<<d.v<<"], \"scale\": "<<d.scale<<", \"rotationDeg\": "<<d.rotationDegrees<<", \"opacity\": "<<d.opacity<<", \"mirror\": "<<(d.mirror?"true":"false")<<"}"<<(i+1<b.appearance.decals.size()?",":"")<<"\n";}
    o<<"  ],\n";
    o<<"  \"modules\": [\n";
    for(std::size_t i=0;i<b.recipe.modules.size();++i){
        const auto& m=b.recipe.modules[i];const auto* r=FindRecord(catalog,m.moduleId);
        std::string parentId,parentSocket,childSocket;
        for(const auto& a:b.recipe.attachments){
            if(a.childModuleIndex==i&&a.parentModuleIndex<b.recipe.modules.size()){
                parentId=InstanceId(a.parentModuleIndex);parentSocket=a.parentSocket;childSocket=a.childSocket;break;
            }
        }
        o<<"    {\n";
        o<<"      \"instanceId\": "<<JsonString(InstanceId(i))<<",\n";
        o<<"      \"moduleId\": "<<JsonString(m.moduleId)<<",\n";
        o<<"      \"moduleClass\": "<<JsonString(r?Lower(ShipyardModuleSystem::ClassName(r->moduleClass)):"component")<<",\n";
        o<<"      \"semantic\": "<<JsonString(r?ShipyardModuleSystem::SemanticName(r->semantic):"COMPONENT")<<",\n";
        o<<"      \"size\": "<<JsonString(r?ShipyardModuleSystem::SizeName(r->size):"M")<<",\n";
        o<<"      \"anchor\": "<<JsonString(r?r->placementRole:"")<<",\n";
        o<<"      \"transform\": {\"position\": ";Vec3(o,m.x,m.y,m.z);o<<", \"rotationEulerDeg\": ";Vec3(o,m.pitchDegrees,m.rollDegrees,m.yawDegrees);o<<", \"scale\": ";Vec3(o,m.scaleX,m.scaleY,m.scaleZ);o<<", \"mirrorX\": "<<(m.mirrorX?"true":"false")<<", \"mirrorY\": "<<(m.mirrorY?"true":"false")<<", \"mirrorZ\": "<<(m.mirrorZ?"true":"false")<<"},\n";
        o<<"      \"connection\": {\"parentInstanceId\": "<<JsonString(parentId)<<", \"parentSocket\": "<<JsonString(parentSocket)<<", \"childSocket\": "<<JsonString(childSocket)<<"},\n";
        o<<"      \"equipmentSlots\": []\n";
        o<<"    }"<<(i+1<b.recipe.modules.size()?",":"")<<"\n";
    }
    o<<"  ]\n";
    o<<"}\n";
    return o.str();
}

bool ShipyardDesignExchangeSystem::Save(const ShipBlueprintDocument& b,
                                        const std::vector<ShipyardModuleRecord>& catalog,
                                        const std::string& path,
                                        std::string* error,
                                        const std::string& toolVersion){
    std::ofstream f(path,std::ios::binary|std::ios::trunc);
    if(!f){if(error)*error="could not open Shipyard design path";return false;}
    f<<Serialize(b,catalog,toolVersion);
    if(!f.good()){if(error)*error="failed while writing Shipyard design";return false;}
    return true;
}

} // namespace subspace
