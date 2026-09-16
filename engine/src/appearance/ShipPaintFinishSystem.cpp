#include "appearance/ShipPaintFinishSystem.h"
#include <algorithm>

namespace subspace {
const std::vector<ShipPaintFinishPreset>& ShipPaintFinishSystem::Presets(){
    static const std::vector<ShipPaintFinishPreset> p={
        {ShipPaintFinish::MatteCeramic,"MATTE_CERAMIC","Matte Ceramic",.10f,.78f,.05f,.55f,.35f,0,0,1.3f,350},
        {ShipPaintFinish::SatinAlloy,"SATIN_ALLOY","Satin Alloy",.70f,.34f,.18f,.24f,.60f,.08f,0,1.3f,400},
        {ShipPaintFinish::PolishedTitanium,"POLISHED_TITANIUM","Polished Titanium",.94f,.12f,.35f,.08f,.82f,.10f,0,1.3f,400},
        {ShipPaintFinish::BrushedSteel,"BRUSHED_STEEL","Brushed Steel",.92f,.28f,.08f,.22f,.72f,.82f,0,1.3f,400},
        {ShipPaintFinish::BlackChrome,"BLACK_CHROME","Black Chrome",.96f,.08f,.85f,.06f,1.0f,.18f,0,1.3f,400},
        {ShipPaintFinish::Pearlescent,"PEARLESCENT","Pearlescent",.35f,.20f,.90f,.08f,.88f,.0f,.42f,1.35f,520},
        {ShipPaintFinish::Iridescent,"IRIDESCENT","Iridescent",.45f,.16f,.86f,.07f,.92f,.0f,.90f,1.35f,650},
        {ShipPaintFinish::FactoryPaint,"FACTORY_PAINT","Factory Paint",.48f,.30f,.76f,.18f,.62f,.0f,.0f,1.3f,400},
        {ShipPaintFinish::Weathered,"WEATHERED","Weathered Metal",.78f,.66f,.0f,.35f,.40f,.16f,.0f,1.3f,400},
    };return p;
}
const ShipPaintFinishPreset& ShipPaintFinishSystem::Describe(ShipPaintFinish f){
    const auto& p=Presets();auto it=std::find_if(p.begin(),p.end(),[&](const auto& v){return v.finish==f;});return it==p.end()?p[1]:*it;
}
const char* ShipPaintFinishSystem::Name(ShipPaintFinish f){return Describe(f).label.c_str();}
void ShipPaintFinishSystem::Apply(ShipPaintLayer& l,ShipPaintFinish f){const auto& p=Describe(f);l.finish=f;l.metallic=p.metallic;l.roughness=p.roughness;l.clearcoat=p.clearcoat;l.clearcoatRoughness=p.clearcoatRoughness;l.specular=p.specular;l.anisotropy=p.anisotropy;l.iridescence=p.iridescence;l.iridescenceIor=p.iridescenceIor;l.iridescenceThicknessNm=p.iridescenceThicknessNm;}
ShipPaintFinish ShipPaintFinishSystem::Next(ShipPaintFinish f,int delta){const auto& p=Presets();auto it=std::find_if(p.begin(),p.end(),[&](const auto& v){return v.finish==f;});int i=it==p.end()?0:static_cast<int>(std::distance(p.begin(),it));i=(i+delta)%static_cast<int>(p.size());if(i<0)i+=static_cast<int>(p.size());return p[static_cast<std::size_t>(i)].finish;}
}
