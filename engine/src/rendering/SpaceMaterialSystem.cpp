#include "rendering/SpaceMaterialSystem.h"

namespace subspace {

SpaceMaterialProfile SpaceMaterialSystem::GetProfile(SpaceMaterialKind kind)
{
    SpaceMaterialProfile p;
    p.kind = kind;
    switch (kind) {
        case SpaceMaterialKind::ShipHull:
            p.roughness=.42f; p.metallic=.76f; p.rimStrength=.18f; p.celBands=6.0f; p.specularStrength=.48f; p.fresnelStrength=.19f; p.edgeHighlight=.24f; p.cavityStrength=.14f; p.wearStrength=.05f; break;
        case SpaceMaterialKind::IndustrialHull:
            p.roughness=.64f; p.metallic=.74f; p.rimStrength=.13f; p.celBands=5.0f; p.specularStrength=.34f; p.fresnelStrength=.14f; p.edgeHighlight=.18f; p.cavityStrength=.22f; p.wearStrength=.12f; break;
        case SpaceMaterialKind::Canopy:
            p.roughness=.10f; p.metallic=.08f; p.emissive=.035f; p.rimStrength=.50f; p.celBands=8.0f; p.specularStrength=.94f; p.fresnelStrength=.64f; p.edgeHighlight=.42f; break;
        case SpaceMaterialKind::EngineHousing:
            p.roughness=.52f; p.metallic=.86f; p.rimStrength=.16f; p.celBands=5.0f; p.specularStrength=.46f; p.fresnelStrength=.18f; p.edgeHighlight=.20f; p.cavityStrength=.24f; p.wearStrength=.16f; break;
        case SpaceMaterialKind::ThrusterCore:
            p.roughness=.10f; p.metallic=.12f; p.emissive=1.0f; p.rimStrength=.70f; p.celBands=8.0f; p.specularStrength=.82f; p.fresnelStrength=.36f; p.edgeHighlight=.22f; break;
        case SpaceMaterialKind::ArmorPlate:
            p.roughness=.34f; p.metallic=.84f; p.rimStrength=.20f; p.celBands=7.0f; p.specularStrength=.56f; p.fresnelStrength=.22f; p.edgeHighlight=.34f; p.cavityStrength=.18f; p.wearStrength=.08f; break;
        case SpaceMaterialKind::StructuralMetal:
            p.roughness=.57f; p.metallic=.91f; p.rimStrength=.13f; p.celBands=5.0f; p.specularStrength=.40f; p.fresnelStrength=.14f; p.edgeHighlight=.18f; p.cavityStrength=.34f; p.wearStrength=.12f; break;
        case SpaceMaterialKind::Radiator:
            p.roughness=.78f; p.metallic=.66f; p.rimStrength=.10f; p.celBands=5.0f; p.specularStrength=.22f; p.fresnelStrength=.10f; p.edgeHighlight=.14f; p.cavityStrength=.22f; break;
        case SpaceMaterialKind::HeatShield:
            p.roughness=.93f; p.metallic=.24f; p.rimStrength=.08f; p.celBands=4.0f; p.specularStrength=.10f; p.fresnelStrength=.06f; p.edgeHighlight=.10f; p.cavityStrength=.24f; p.wearStrength=.28f; break;
        case SpaceMaterialKind::DecalSurface:
            p.roughness=.46f; p.metallic=.20f; p.rimStrength=.08f; p.celBands=6.0f; p.specularStrength=.24f; p.fresnelStrength=.08f; p.edgeHighlight=.08f; break;
        case SpaceMaterialKind::AsteroidRock:
            p.roughness=.94f; p.metallic=.05f; p.rimStrength=.08f; p.celBands=4.0f; p.specularStrength=.08f; p.fresnelStrength=.03f; break;
        case SpaceMaterialKind::OreVein:
            p.roughness=.38f; p.metallic=.62f; p.emissive=.07f; p.rimStrength=.24f; p.celBands=5.0f; p.specularStrength=.56f; p.fresnelStrength=.18f; break;
        case SpaceMaterialKind::StationHull:
            p.roughness=.58f; p.metallic=.82f; p.rimStrength=.14f; p.celBands=5.0f; p.specularStrength=.40f; p.fresnelStrength=.16f; break;
        case SpaceMaterialKind::PlanetRock:
            p.roughness=.91f; p.metallic=.02f; p.rimStrength=.10f; p.celBands=7.0f; p.specularStrength=.06f; p.fresnelStrength=.05f;
            p.surfaceMode=1.0f; p.surfaceVariation=.64f; p.detailScale=11.0f; p.detailR=.20f; p.detailG=.19f; p.detailB=.18f; break;
        case SpaceMaterialKind::PlanetDesert:
            p.roughness=.86f; p.metallic=.01f; p.rimStrength=.12f; p.celBands=7.0f; p.specularStrength=.08f; p.fresnelStrength=.06f;
            p.surfaceMode=2.0f; p.surfaceVariation=.55f; p.detailScale=8.0f; p.bandStrength=.16f; p.detailR=.92f; p.detailG=.68f; p.detailB=.32f; break;
        case SpaceMaterialKind::PlanetOcean:
            p.roughness=.22f; p.metallic=.03f; p.rimStrength=.26f; p.celBands=8.0f; p.specularStrength=.74f; p.fresnelStrength=.42f;
            p.surfaceMode=3.0f; p.surfaceVariation=.72f; p.detailScale=6.5f; p.detailR=.22f; p.detailG=.48f; p.detailB=.24f; break;
        case SpaceMaterialKind::PlanetIce:
            p.roughness=.36f; p.metallic=.02f; p.rimStrength=.31f; p.celBands=8.0f; p.specularStrength=.54f; p.fresnelStrength=.30f;
            p.surfaceMode=4.0f; p.surfaceVariation=.50f; p.detailScale=13.0f; p.detailR=.88f; p.detailG=.96f; p.detailB=1.0f; break;
        case SpaceMaterialKind::PlanetVolcanic:
            p.roughness=.88f; p.metallic=.04f; p.rimStrength=.12f; p.celBands=7.0f; p.specularStrength=.10f; p.fresnelStrength=.06f;
            p.surfaceMode=5.0f; p.surfaceVariation=.88f; p.detailScale=17.0f; p.lavaGlow=.80f; p.detailR=.98f; p.detailG=.22f; p.detailB=.025f; break;
        case SpaceMaterialKind::PlanetBarren:
            p.roughness=.96f; p.metallic=.01f; p.rimStrength=.08f; p.celBands=6.0f; p.specularStrength=.04f; p.fresnelStrength=.03f;
            p.surfaceMode=6.0f; p.surfaceVariation=.76f; p.detailScale=19.0f; p.detailR=.48f; p.detailG=.42f; p.detailB=.36f; break;
        case SpaceMaterialKind::PlanetGas:
            p.roughness=.74f; p.metallic=0.0f; p.rimStrength=.27f; p.celBands=9.0f; p.specularStrength=.08f; p.fresnelStrength=.20f;
            p.surfaceMode=7.0f; p.surfaceVariation=.78f; p.detailScale=7.0f; p.bandStrength=.92f; p.detailR=.88f; p.detailG=.72f; p.detailB=.49f; break;
        case SpaceMaterialKind::Sun:
            p.roughness=0.0f; p.metallic=0.0f; p.emissive=1.0f; p.rimStrength=.90f; p.celBands=12.0f; p.specularStrength=0.0f; p.fresnelStrength=.28f;
            p.surfaceMode=8.0f; p.surfaceVariation=.78f; p.detailScale=8.0f; p.bandStrength=.38f; p.detailR=1.0f; p.detailG=.78f; p.detailB=.28f; break;
        case SpaceMaterialKind::MissileBody:
            p.roughness=.48f; p.metallic=.84f; p.rimStrength=.16f; p.celBands=5.0f; p.specularStrength=.42f; p.fresnelStrength=.17f; break;
        case SpaceMaterialKind::MissileExhaust:
            p.roughness=0.0f; p.metallic=0.0f; p.emissive=1.0f; p.rimStrength=.82f; p.celBands=9.0f; p.specularStrength=0.0f; p.fresnelStrength=.25f; break;
        case SpaceMaterialKind::Dust:
            p.roughness=1.0f; p.metallic=0.0f; p.rimStrength=0.0f; p.celBands=2.0f; p.specularStrength=0.0f; p.fresnelStrength=0.0f; break;
        case SpaceMaterialKind::Debris:
            p.roughness=.82f; p.metallic=.44f; p.rimStrength=.09f; p.celBands=4.0f; p.specularStrength=.18f; p.fresnelStrength=.07f; break;
    }
    return p;
}

const char* SpaceMaterialSystem::VertexShader120()
{
    return R"GLSL(#version 120
varying vec3 vNormal;
varying vec3 vEyePos;
varying vec3 vObjectNormal;
varying vec4 vColor;
varying vec2 vTexCoord;
void main() {
    vec4 eye = gl_ModelViewMatrix * gl_Vertex;
    vEyePos = eye.xyz;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vObjectNormal = normalize(gl_Normal);
    vColor = gl_Color;
    vTexCoord = gl_MultiTexCoord0.xy;
    gl_Position = gl_ProjectionMatrix * eye;
}
)GLSL";
}

const char* SpaceMaterialSystem::FragmentShader120()
{
    return R"GLSL(#version 120
varying vec3 vNormal;
varying vec3 vEyePos;
varying vec3 vObjectNormal;
varying vec4 vColor;
varying vec2 vTexCoord;
uniform sampler2D uBaseColorTexture;
uniform float uUseBaseColorTexture;
uniform vec3 uLightDirection;
uniform vec3 uSunColor;
uniform vec3 uDetailColor;
uniform float uAmbient;
uniform float uCelBands;
uniform float uRimStrength;
uniform float uMetallic;
uniform float uRoughness;
uniform float uEmission;
uniform float uSpecularStrength;
uniform float uFresnelStrength;
uniform float uSurfaceMode;
uniform float uSurfaceVariation;
uniform float uDetailScale;
uniform float uBandStrength;
uniform float uLavaGlow;
uniform float uSurfaceSeed;
uniform float uEdgeHighlight;
uniform float uCavityStrength;
uniform float uWearStrength;
uniform float uTime;

float hash31(vec3 p) {
    return fract(sin(dot(p, vec3(127.1,311.7,74.7)) + uSurfaceSeed*0.017) * 43758.5453);
}
float smoothNoise(vec3 p) {
    vec3 i=floor(p), f=fract(p); f=f*f*(3.0-2.0*f);
    float n000=hash31(i+vec3(0,0,0)); float n100=hash31(i+vec3(1,0,0));
    float n010=hash31(i+vec3(0,1,0)); float n110=hash31(i+vec3(1,1,0));
    float n001=hash31(i+vec3(0,0,1)); float n101=hash31(i+vec3(1,0,1));
    float n011=hash31(i+vec3(0,1,1)); float n111=hash31(i+vec3(1,1,1));
    float x00=mix(n000,n100,f.x), x10=mix(n010,n110,f.x);
    float x01=mix(n001,n101,f.x), x11=mix(n011,n111,f.x);
    return mix(mix(x00,x10,f.y),mix(x01,x11,f.y),f.z);
}
float fbm(vec3 p) {
    float value=0.0, amp=0.55;
    for(int i=0;i<4;++i){ value += smoothNoise(p)*amp; p=p*2.03+vec3(1.7,-2.1,0.9); amp*=0.48; }
    return clamp(value,0.0,1.0);
}

void main() {
    vec3 n = normalize(vNormal);
    vec3 on = normalize(vObjectNormal);
    // Pass429: rotate only the procedural surface sampling coordinates. The
    // actual sphere lighting/normal stays physically stable while continents,
    // regolith, storms and bands visibly rotate around the planet.
    vec3 sampleOn = on;
    if(uSurfaceMode > 0.5 && uSurfaceMode < 7.5) {
        float spinRate = 0.012 + fract(uSurfaceSeed*0.0017)*0.018;
        float spin = uTime*spinRate;
        float cs=cos(spin), sn=sin(spin);
        sampleOn = normalize(vec3(on.x*cs-on.y*sn,on.x*sn+on.y*cs,on.z));
    }
    vec3 l = normalize(gl_LightSource[0].position.xyz);
    vec3 v = normalize(-vEyePos);
    float ndl = max(dot(n,l), 0.0);
    float bands = max(uCelBands, 1.0);
    float cel = floor(ndl * bands + 0.42) / bands;

    float detail = fbm(sampleOn * max(1.0,uDetailScale));
    vec4 sourceTex = uUseBaseColorTexture > 0.5 ? texture2D(uBaseColorTexture, vTexCoord) : vec4(1.0);
    vec3 albedo = vColor.rgb * sourceTex.rgb;
    float outputAlpha = vColor.a * sourceTex.a;
    float extraEmission = 0.0;
    if(uSurfaceMode < 0.5) {
        float micro = fbm(on*17.0 + vec3(2.0,5.0,11.0));
        float cavity = (1.0-abs(on.z))*uCavityStrength*(0.36+0.64*micro);
        albedo *= 1.0-clamp(cavity,0.0,0.34);
        float wear = smoothstep(0.72,0.96,micro)*uWearStrength;
        albedo = mix(albedo,albedo*1.18+vec3(0.035),wear);
    }

    if(uSurfaceMode > 0.5) {
        float amount = clamp(uSurfaceVariation,0.0,1.0);
        if(uSurfaceMode < 1.5) { // rock
            float crater = smoothstep(0.32,0.78,detail);
            albedo = mix(albedo*0.72,uDetailColor,crater*amount);
        } else if(uSurfaceMode < 2.5) { // desert
            float dunes = 0.5+0.5*sin((sampleOn.z*9.0 + detail*2.2 + sampleOn.x*2.0)*3.14159);
            albedo = mix(albedo,uDetailColor,(0.18+0.62*dunes)*amount);
        } else if(uSurfaceMode < 3.5) { // ocean + seeded continents
            float land = smoothstep(0.60,0.72,detail + sampleOn.z*0.05);
            albedo = mix(albedo*mix(0.72,1.10,detail),uDetailColor,land*amount);
        } else if(uSurfaceMode < 4.5) { // ice
            float cracks = smoothstep(0.58,0.82,abs(sin(detail*24.0 + sampleOn.x*10.0)));
            albedo = mix(albedo,uDetailColor,(0.45+0.42*detail)*amount);
            albedo *= 1.0-0.16*cracks;
        } else if(uSurfaceMode < 5.5) { // volcanic
            float crust = smoothstep(0.26,0.72,detail);
            float lava = smoothstep(0.76,0.91,fbm(sampleOn*max(12.0,uDetailScale*1.6)+vec3(8.0,1.0,4.0)));
            albedo = mix(albedo*0.44,albedo,crust);
            albedo = mix(albedo,uDetailColor,lava*0.92);
            extraEmission = lava*uLavaGlow;
        } else if(uSurfaceMode < 6.5) { // barren / regolith
            float basin = smoothstep(0.40,0.72,detail);
            albedo = mix(albedo*0.64,uDetailColor,basin*amount);
        } else if(uSurfaceMode < 7.5) { // gas giant
            float latitude = sampleOn.z;
            float jets = 0.5+0.5*sin(latitude*31.0 + detail*7.0 + uSurfaceSeed*0.01);
            float storms = smoothstep(0.78,0.94,fbm(sampleOn*13.0+vec3(3.0,11.0,7.0)));
            albedo = mix(albedo,uDetailColor,jets*clamp(uBandStrength,0.0,1.0));
            albedo = mix(albedo,vec3(0.94,0.88,0.78),storms*0.24);
        } else { // stellar photosphere
            float drift=uTime*0.055;
            float cells=fbm(on*max(5.0,uDetailScale)+vec3(drift,-drift*.72,drift*.31));
            float granules=fbm(on*22.0+vec3(-drift*.36,drift*.42,drift));
            float hot=smoothstep(.44,.82,cells*.72+granules*.38);
            float dark=smoothstep(.18,.48,cells);
            albedo=mix(vColor.rgb*.74,uDetailColor,hot*clamp(uSurfaceVariation,0.0,1.0));
            albedo*=.90+.26*dark;
            extraEmission=.72+.34*hot;
        }
    }

    float rim = pow(1.0 - max(dot(n,v),0.0), 2.2) * uRimStrength;
    vec3 h = normalize(l + v);
    float rough = clamp(uRoughness + (detail-0.5)*0.08*uSurfaceVariation,0.02,1.0);
    float specPower = mix(10.0, 92.0, 1.0-rough);
    float spec = pow(max(dot(n,h),0.0), specPower) * uSpecularStrength * (0.28 + 0.72*uMetallic);
    float fresnel = pow(1.0-max(dot(n,v),0.0),5.0)*uFresnelStrength;
    float edge = pow(1.0-max(dot(n,v),0.0),2.2)*uEdgeHighlight;
    vec3 hemi = albedo * (uAmbient + 0.055*(n.z*0.5+0.5));
    vec3 lit = hemi + albedo*(cel*uSunColor) + vec3(spec) + albedo*rim + vec3(fresnel) + vec3(edge) + albedo*uEmission;
    lit += uDetailColor*extraEmission;
    lit = lit / (vec3(1.0) + lit*0.22);
    lit = pow(max(lit,vec3(0.0)),vec3(0.94));
    gl_FragColor = vec4(lit, outputAlpha);
}
)GLSL";
}

} // namespace subspace
