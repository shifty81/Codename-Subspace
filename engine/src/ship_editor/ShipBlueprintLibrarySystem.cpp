#include "ship_editor/ShipBlueprintLibrarySystem.h"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace subspace {
namespace {

template<class T> bool ReadValue(std::istringstream& in,T& v){return static_cast<bool>(in>>v);}
bool ReadQuoted(std::istringstream& in,std::string& v){return static_cast<bool>(in>>std::quoted(v));}

} // namespace

std::string ShipBlueprintLibrarySystem::CanonicalId(const ShipBlueprintDocument& b){
    std::uint32_t h=2166136261u;
    auto mix=[&](const std::string&s){for(unsigned char c:s){h^=c;h*=16777619u;}};
    mix(b.name);
    for(const auto&m:b.recipe.modules)mix(m.moduleId);
    for(const auto&e:b.equipmentSlots)mix(e.installedDefinitionId);
    {std::ostringstream a;a<<b.appearance.primary.r<<','<<b.appearance.primary.g<<','<<b.appearance.primary.b<<';'<<b.appearance.secondary.r<<','<<b.appearance.secondary.g<<','<<b.appearance.secondary.b<<';'<<b.appearance.trim.r<<','<<b.appearance.trim.g<<','<<b.appearance.trim.b;mix(a.str());}
    for(const auto&d:b.appearance.decals){mix(d.decalAsset);mix(std::to_string(d.moduleIndex));}
    std::ostringstream o;o<<"bp_"<<std::hex<<h;return o.str();
}

bool ShipBlueprintLibrarySystem::Save(const ShipBlueprintDocument& b,const std::string& path,std::string* error){
    std::ofstream f(path,std::ios::trunc);
    if(!f){if(error)*error="could not open blueprint path";return false;}
    const std::string id=b.blueprintId.empty()?CanonicalId(b):b.blueprintId;
    f<<"SUBSPACE_SHIP_BLUEPRINT_V1\n";
    f<<"META "<<std::quoted(id)<<' '<<std::quoted(b.name)<<' '<<std::quoted(b.author)<<' '<<b.revision<<"\n";
    f<<"RECIPE "<<std::quoted(b.recipe.recipeId)<<' '<<std::quoted(b.recipe.role)<<' '<<b.recipe.seed<<' '
     <<std::quoted(b.recipe.sourceFamily)<<' '<<std::quoted(b.recipe.manufacturerFamily)<<' '<<std::quoted(b.recipe.decalCode)<<' '
     <<b.recipe.widthScale<<' '<<b.recipe.lengthScale<<"\n";
    f<<"ORIENTATION "<<b.recipe.forwardVisualYawDegrees<<' '<<std::quoted(b.recipe.forwardAuthority)<<' '<<b.recipe.cockpitModuleIndex<<"\n";
    for(const auto&t:b.tags)f<<"TAG "<<std::quoted(t)<<"\n";
    for(const auto&m:b.recipe.modules){
        f<<"MODULE "<<std::quoted(m.moduleId)<<' '<<m.x<<' '<<m.y<<' '<<m.z<<' '<<m.scaleX<<' '<<m.scaleY<<' '<<m.scaleZ<<' '
         <<m.yawDegrees<<' '<<m.pitchDegrees<<' '<<m.rollDegrees<<' '<<static_cast<int>(m.material)<<' '<<(m.mirrorX?1:0)<<' '<<(m.mirrorY?1:0)<<' '<<(m.mirrorZ?1:0)<<"\n";
    }
    for(const auto&a:b.recipe.attachments){
        f<<"ATTACH "<<a.parentModuleIndex<<' '<<a.childModuleIndex<<' '<<std::quoted(a.parentSocket)<<' '<<std::quoted(a.childSocket)<<' '
         <<a.measuredGap<<' '<<(a.certified?1:0)<<"\n";
    }
    for(const auto&s:b.equipmentSlots){
        f<<"EQUIP "<<std::quoted(s.slotId)<<' '<<s.moduleIndex<<' '<<static_cast<int>(s.type)<<' '<<static_cast<int>(s.size)<<' '
         <<std::quoted(s.installedItemInstanceId)<<' '<<std::quoted(s.installedDefinitionId)<<' '<<(s.required?1:0)<<"\n";
    }
    const auto paint=[&](const char* name,const ShipPaintLayer&p){f<<"PAINT "<<name<<' '<<p.r<<' '<<p.g<<' '<<p.b<<' '<<p.a<<' '<<p.metallic<<' '<<p.roughness<<"\n";};
    paint("PRIMARY",b.appearance.primary);paint("SECONDARY",b.appearance.secondary);paint("TRIM",b.appearance.trim);
    f<<"WEAR "<<b.appearance.factoryWear<<"\n";
    for(const auto&d:b.appearance.decals){
        f<<"DECAL "<<std::quoted(d.id)<<' '<<std::quoted(d.decalAsset)<<' '<<d.moduleIndex<<' '<<d.u<<' '<<d.v<<' '<<d.scale<<' '
         <<d.rotationDegrees<<' '<<d.opacity<<' '<<(d.mirror?1:0)<<"\n";
    }
    if(!f.good()){if(error)*error="failed while writing blueprint";return false;}
    return true;
}

bool ShipBlueprintLibrarySystem::Load(const std::string& path,ShipBlueprintDocument& b,std::string* error){
    std::ifstream f(path);
    if(!f){if(error)*error="could not open blueprint path";return false;}
    std::string line;
    if(!std::getline(f,line)||line!="SUBSPACE_SHIP_BLUEPRINT_V1"){if(error)*error="unsupported blueprint schema";return false;}
    b={};
    while(std::getline(f,line)){
        if(line.empty())continue;
        std::istringstream in(line);std::string kind;in>>kind;
        if(kind=="META"){
            if(!ReadQuoted(in,b.blueprintId)||!ReadQuoted(in,b.name)||!ReadQuoted(in,b.author)||!ReadValue(in,b.revision)){if(error)*error="invalid META row";return false;}
        }else if(kind=="RECIPE"){
            if(!ReadQuoted(in,b.recipe.recipeId)||!ReadQuoted(in,b.recipe.role)||!ReadValue(in,b.recipe.seed)||!ReadQuoted(in,b.recipe.sourceFamily)||!ReadQuoted(in,b.recipe.manufacturerFamily)||!ReadQuoted(in,b.recipe.decalCode)||!ReadValue(in,b.recipe.widthScale)||!ReadValue(in,b.recipe.lengthScale)){if(error)*error="invalid RECIPE row";return false;}
        }else if(kind=="ORIENTATION"){
            if(!ReadValue(in,b.recipe.forwardVisualYawDegrees)||!ReadQuoted(in,b.recipe.forwardAuthority)||!ReadValue(in,b.recipe.cockpitModuleIndex)){if(error)*error="invalid ORIENTATION row";return false;}
        }else if(kind=="TAG"){
            std::string t;if(!ReadQuoted(in,t)){if(error)*error="invalid TAG row";return false;}b.tags.push_back(t);
        }else if(kind=="MODULE"){
            VisualModulePlacement m;int material=0,mirrorX=0,mirrorY=0,mirrorZ=0;
            if(!ReadQuoted(in,m.moduleId)||!ReadValue(in,m.x)||!ReadValue(in,m.y)||!ReadValue(in,m.z)||!ReadValue(in,m.scaleX)||!ReadValue(in,m.scaleY)||!ReadValue(in,m.scaleZ)||!ReadValue(in,m.yawDegrees)||!ReadValue(in,m.pitchDegrees)||!ReadValue(in,m.rollDegrees)||!ReadValue(in,material)){if(error)*error="invalid MODULE row";return false;}
            if(in>>mirrorX)m.mirrorX=mirrorX!=0;if(in>>mirrorY)m.mirrorY=mirrorY!=0;if(in>>mirrorZ)m.mirrorZ=mirrorZ!=0;
            m.material=static_cast<SpaceMaterialKind>(material);b.recipe.modules.push_back(m);
        }else if(kind=="ATTACH"){
            ShipVisualAttachment a;int certified=0;
            if(!ReadValue(in,a.parentModuleIndex)||!ReadValue(in,a.childModuleIndex)||!ReadQuoted(in,a.parentSocket)||!ReadQuoted(in,a.childSocket)||!ReadValue(in,a.measuredGap)||!ReadValue(in,certified)){if(error)*error="invalid ATTACH row";return false;}
            a.certified=certified!=0;b.recipe.attachments.push_back(a);
        }else if(kind=="EQUIP"){
            ShipEquipmentSlot s;int type=0,size=0,required=0;
            if(!ReadQuoted(in,s.slotId)||!ReadValue(in,s.moduleIndex)||!ReadValue(in,type)||!ReadValue(in,size)||!ReadQuoted(in,s.installedItemInstanceId)||!ReadQuoted(in,s.installedDefinitionId)||!ReadValue(in,required)){if(error)*error="invalid EQUIP row";return false;}
            s.type=static_cast<ShipEquipmentSlotType>(type);s.size=static_cast<ShipyardModuleSize>(size);s.required=required!=0;b.equipmentSlots.push_back(s);
        }else if(kind=="PAINT"){
            std::string name;ShipPaintLayer* p=nullptr;in>>name;if(name=="PRIMARY")p=&b.appearance.primary;else if(name=="SECONDARY")p=&b.appearance.secondary;else if(name=="TRIM")p=&b.appearance.trim;
            if(!p||!ReadValue(in,p->r)||!ReadValue(in,p->g)||!ReadValue(in,p->b)||!ReadValue(in,p->a)||!ReadValue(in,p->metallic)||!ReadValue(in,p->roughness)){if(error)*error="invalid PAINT row";return false;}
        }else if(kind=="WEAR"){
            if(!ReadValue(in,b.appearance.factoryWear)){if(error)*error="invalid WEAR row";return false;}
        }else if(kind=="DECAL"){
            ShipDecalLayer d;int mirror=0;
            if(!ReadQuoted(in,d.id)||!ReadQuoted(in,d.decalAsset)||!ReadValue(in,d.moduleIndex)||!ReadValue(in,d.u)||!ReadValue(in,d.v)||!ReadValue(in,d.scale)||!ReadValue(in,d.rotationDegrees)||!ReadValue(in,d.opacity)||!ReadValue(in,mirror)){if(error)*error="invalid DECAL row";return false;}
            d.mirror=mirror!=0;b.appearance.decals.push_back(d);
        }
    }
    if(b.name.empty())b.name="Untitled Ship";
    if(b.recipe.recipeId.empty())b.recipe.recipeId=b.blueprintId;
    if(b.recipe.sourceFamily=="SHIPYARD_V07_CC0"&&b.recipe.forwardAuthority=="LEGACY"){
        b.recipe.forwardVisualYawDegrees=180.0f;
        b.recipe.forwardAuthority="COCKPIT";
        for(std::size_t i=0;i<b.recipe.modules.size();++i){
            if(b.recipe.modules[i].moduleId.find("_command_")!=std::string::npos){b.recipe.cockpitModuleIndex=static_cast<int>(i);break;}
        }
    }
    return true;
}

} // namespace subspace
