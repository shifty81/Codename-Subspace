#include "rendering/ForwardSpacePresentationSystem.h"

#include <algorithm>

namespace subspace {

SpaceBackdropProfile ForwardSpacePresentationSystem::ForSector(const GalaxySector& sector) const {
    SpaceBackdropProfile p;
    const float luminosity = sector.hasStar ? sector.star.luminosity : 1.0f;
    p.exposure = std::clamp(0.86f + luminosity * 0.18f, 0.82f, 1.22f);
    p.warmBias = sector.hasStar ? std::clamp(sector.star.colorR - sector.star.colorB, -0.35f, 0.35f) : 0.0f;
    const std::uint64_t mix = static_cast<std::uint64_t>((sector.x + 4096) * 73856093) ^
                              static_cast<std::uint64_t>((sector.y + 4096) * 19349663);
    p.galacticBandStrength = 0.08f + static_cast<float>(mix % 13u) * 0.006f;
    p.nebulaHaze = 0.015f + static_cast<float>((mix >> 5) % 11u) * 0.006f;
    p.localDust = 0.025f + static_cast<float>((mix >> 9) % 9u) * 0.005f;
    const float hueA=static_cast<float>((mix>>17)&0xffu)/255.0f;
    const float hueB=static_cast<float>((mix>>25)&0x7fu)/127.0f;
    p.nebulaR=std::clamp(0.055f+hueA*0.10f+p.warmBias*0.06f,0.035f,0.22f);
    p.nebulaG=std::clamp(0.085f+hueB*0.13f,0.055f,0.26f);
    p.nebulaB=std::clamp(0.16f+(1.0f-hueA)*0.18f-p.warmBias*0.04f,0.12f,0.38f);
    p.dustLaneStrength=0.06f+static_cast<float>((mix>>3)%9u)*0.012f;
    p.starTwinkle=0.08f+static_cast<float>((mix>>12)%7u)*0.018f;
    p.galacticBandTilt=-0.18f+static_cast<float>((mix>>20)%13u)*0.03f;
    p.ultraDistantStars = 170 + static_cast<int>(mix % 70u);
    p.distantStars = 250 + static_cast<int>((mix >> 7) % 120u);
    p.localStars = 90 + static_cast<int>((mix >> 13) % 80u);
    p.identity = p.nebulaHaze > 0.06f ? "DUSTED STELLAR FIELD" : (p.warmBias > 0.12f ? "WARM STAR FIELD" : "DEEP STELLAR FIELD");
    return p;
}

ShipPresentationProfile ForwardSpacePresentationSystem::ForShip(const std::string& role, bool player, float screenFraction) const {
    ShipPresentationProfile p;
    std::string r=role; for(auto& c:r) if(c>='a'&&c<='z') c=static_cast<char>(c-'a'+'A');
    if(r.find("CARRIER")!=std::string::npos){p.identity="CARRIER";p.lengthScale=1.24f;p.widthScale=1.30f;p.machineryExposure=.42f;}
    else if(r.find("HAUL")!=std::string::npos){p.identity="HAULER";p.lengthScale=1.28f;p.widthScale=1.10f;p.machineryExposure=.46f;}
    else if(r.find("MIN")!=std::string::npos||r.find("INDUSTR")!=std::string::npos){p.identity="INDUSTRIAL";p.lengthScale=1.18f;p.widthScale=1.16f;p.machineryExposure=.58f;p.platingBreakup=.74f;}
    else if(r.find("SALV")!=std::string::npos){p.identity="SALVAGE";p.lengthScale=1.12f;p.widthScale=1.08f;p.machineryExposure=.66f;}
    else if(r.find("FIGHT")!=std::string::npos||r.find("COMBAT")!=std::string::npos||r.find("DEFEND")!=std::string::npos){p.identity="COMBAT";p.lengthScale=1.12f;p.widthScale=.91f;p.platingBreakup=.84f;p.thrusterGlow=1.18f;}
    else if(r.find("EXPLO")!=std::string::npos||r.find("PATH")!=std::string::npos){p.identity="EXPLORATION";p.lengthScale=1.08f;p.widthScale=.96f;p.machineryExposure=.25f;}
    if(player){p.platingBreakup=std::max(p.platingBreakup,.78f);p.navigationLights=1.0f;p.hullCohesion=.94f;p.emissiveAccent=.52f;}
    if(p.identity=="INDUSTRIAL"||p.identity=="SALVAGE"||p.identity=="HAULER"){p.materialRoughness=.72f;p.metallicResponse=.78f;}
    if(p.identity=="COMBAT"){p.hullCohesion=.90f;p.materialRoughness=.48f;p.retroThrusterIntensity=1.0f;p.maneuverThrusterIntensity=.96f;}
    if(screenFraction<.012f)p.lod=3;else if(screenFraction<.035f)p.lod=2;else if(screenFraction<.10f)p.lod=1;else p.lod=0;
    p.lodDetailScale=p.lod==0?1.0f:(p.lod==1?.72f:(p.lod==2?.38f:.12f));
    return p;
}

VectorVisualProfile ForwardSpacePresentationSystem::VectorVisual(VectorTravelStage stage, double progress) const {
    const float p = static_cast<float>(std::clamp(progress, 0.0, 1.0));
    VectorVisualProfile v;
    switch(stage) {
        case VectorTravelStage::Aligning:
            // Alignment is intentionally restrained: the ship visibly turns onto the
            // destination vector before space begins to tear open around it.
            v.distortion = p * 0.055f;
            v.cameraPullback = p * 0.03f;
            v.chaseBias = p * 0.72f;
            v.engineOverdrive = p * 0.24f;
            v.shipEnvelope = p * 0.10f;
            v.audioIntensity = p * 0.18f;
            break;
        case VectorTravelStage::Charging: {
            // The tunnel should form late in the charge rather than already looking
            // like cruise.  This gives the entry a readable accelerate -> break-through beat.
            const float tunnelP = std::clamp((p - 0.24f) / 0.76f, 0.0f, 1.0f);
            const float breakP = std::clamp((p - 0.55f) / 0.35f, 0.0f, 1.0f);
            v.distortion = 0.055f + p*0.50f;
            v.starStretch = p*p*0.58f;
            v.cameraPullback = 0.03f+p*0.13f;
            v.entryFlash=breakP*breakP;
            v.chromaticShift=.04f+p*.20f;
            v.audioIntensity=.18f+p*.78f;
            v.tunnelOpacity=tunnelP*.40f;
            v.tunnelFlow=.14f+tunnelP*.68f;
            v.tunnelTwist=tunnelP*.48f;
            v.tunnelPulse=.10f+tunnelP*.74f;
            v.shipEnvelope=.10f+p*.72f;
            v.chaseBias=.72f+p*.28f;
            v.engineOverdrive=.24f+p*.76f;
            v.foregroundStreaks=tunnelP*.58f;
            break;
        }
        case VectorTravelStage::Cruise:
            v.distortion = 0.82f;
            v.starStretch = 1.0f;
            v.tunnelOpacity = 0.86f;
            v.cameraPullback = 0.18f;
            v.tunnelFlow=1.0f;
            v.chromaticShift=.24f;
            v.audioIntensity=.92f;
            v.tunnelTwist=.72f;
            v.tunnelPulse=.88f;
            v.shipEnvelope=.94f;
            v.chaseBias=1.0f;
            v.engineOverdrive=1.0f;
            v.foregroundStreaks=.90f;
            break;
        case VectorTravelStage::Decelerating:
            v.distortion = 0.78f*(1.0f-p);
            v.starStretch = 1.0f-p*.82f;
            v.tunnelOpacity = 0.82f*(1.0f-p);
            v.exitReveal = p;
            v.destinationReveal=p;
            v.audioIntensity=.88f*(1.0f-p);
            v.tunnelFlow=1.0f-p*.72f;
            v.tunnelTwist=.66f*(1.0f-p);
            v.tunnelPulse=.78f*(1.0f-p);
            v.shipEnvelope=.90f*(1.0f-p);
            v.chaseBias=1.0f-p*.55f;
            v.engineOverdrive=1.0f-p*.72f;
            v.foregroundStreaks=.82f*(1.0f-p);
            break;
        case VectorTravelStage::Complete:
            v.exitReveal = 1.0f;
            break;
        default: break;
    }
    return v;
}

} // namespace subspace
