#include "rendering/ImportedPlanetVisualSystem.h"
namespace subspace {
ImportedPlanetVisualProfile ImportedPlanetVisualSystem::ProfileFor(PlanetType type){
    switch(type){
    case PlanetType::Rocky: return {"smac","textures/image_00.png",{"textures/image_03.png"},false,.88f,1.18f,-.58f,1.006f,1.018f,.035f};
    case PlanetType::Desert: return {"barren","textures/image_21.jpg",{},false,.72f,.96f,-.55f,1.004f,1.012f,.075f};
    case PlanetType::Ice: return {"frozen","textures/image_14.jpg",{},false,.64f,.82f,-.48f,1.005f,1.022f,-.035f};
    case PlanetType::Oceanic: return {"continental","textures/image_09.jpg",{"textures/image_12.png"},false,.92f,1.34f,-.72f,1.008f,1.024f,.018f};
    case PlanetType::Volcanic: return {"lava","textures/image_17.jpg",{},true,.78f,.92f,-.44f,1.004f,1.014f,.11f};
    case PlanetType::Barren: return {"barren","textures/image_21.jpg",{},false,.58f,.72f,-.40f,1.003f,1.010f,-.02f};
    case PlanetType::GasGiant: return {"gas","textures/image_05.jpg",{"textures/image_07.png","textures/image_24.png"},false,1.16f,1.62f,-1.08f,1.012f,1.030f,.055f};
    }
    return {"smac","textures/image_00.png",{},false,.88f,1.18f,-.58f,1.006f,1.018f,.0f};
}
std::filesystem::path ImportedPlanetVisualSystem::PackRoot(const std::filesystem::path& repositoryRoot){
    return repositoryRoot/"content"/"derived"/"various_planets_v1";
}
bool ImportedPlanetVisualSystem::PackReady(const std::filesystem::path& repositoryRoot){
    std::error_code ec; const auto root=PackRoot(repositoryRoot);
    return std::filesystem::exists(root/"VARIOUS_PLANETS_READY.txt",ec) && std::filesystem::exists(root/"planet_pack_manifest.json",ec);
}
}
