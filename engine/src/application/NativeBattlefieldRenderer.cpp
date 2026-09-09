#include "application/NativeBattlefieldRenderer.h"
#include "celestial/SystemSpatialScale.h"

#include "core/logging/Logger.h"
#include "core/physics/PhysicsComponent.h"
#include "core/resources/ObjAssetLoader.h"
#include "content/ShipyardModuleSystem.h"
#include "content/ShipyardCanonicalAssetBridge.h"
#include "assets/CanonicalAssetRegistry.h"
#include "content/ShipyardGeometryAnalysisSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/ShipyardPaintZoneSystem.h"
#include "ships/ShipyardVisualAuthoritySystem.h"
#include "ships/ShipyardAuthoredShipSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"
#include "input/InputState.h"
#include "mining/MiningSalvageLoop.h"
#include "rendering/SpaceMaterialSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "rendering/PlanetSurfaceSystem.h"
#include "rendering/PlanetPresentationSystem.h"
#include "rendering/ImportedPlanetVisualSystem.h"
#include "rendering/CelestialEnvironmentSystem.h"
#include "station/StationKitbashVisualSystem.h"
#include "station/StationRuntimeQualitySystem.h"
#include "station/StationActivityPresentationSystem.h"
#include "rendering/SolarPresentationSystem.h"
#include "rendering/PlanetAtmospherePresentationSystem.h"
#include "rendering/PlanetWeatherSystem.h"
#include "rendering/GasGiantWeatherSystem.h"
#include "effects/PropulsionVisualSystem.h"
#include "editor/EditorGizmoSystem.h"
#include "editor/EditorAssetThumbnailSystem.h"
#include "station/StationNavigationLightSystem.h"
#include "station/StationHangarPresentationSystem.h"
#include "station/StationServiceEnvelopeSystem.h"
#include "ui/UIRenderer.h"
#include "ui/SubspaceUiFramework.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#include <objbase.h>
#include <GL/gl.h>
#endif

namespace subspace {

namespace fs = std::filesystem;

struct PlanetTextureDiagnostic {
    int width=0;
    int height=0;
    int mipLevels=1;
    float anisotropy=1.0f;
};

struct NativeBattlefieldRenderer::VisualAssets {
    std::unordered_map<std::string, ObjMeshData> shipModules;
    ProceduralVisualCatalog generatedVisualCatalog;
    std::vector<ShipyardModuleRecord> shipyardCatalog;
    assets::CanonicalAssetRegistry canonicalAssetRegistry;
    fs::path repositoryRoot;
    bool moduleLibraryReady = false;
    bool shipyardReady = false;
    std::size_t shipyardModuleCount = 0;
    std::size_t shipyardMaterialRegions = 0;
    std::size_t shipyardResolvedMtlRegions = 0;
    std::size_t shipyardTexturedRegions = 0;
    std::size_t shipyardSemanticMaterialRegions = 0;
    std::size_t shipyardUnmappedMaterialRegions = 0;
    std::size_t shipyardUnzonedModules = 0;
    std::size_t shipyardUnassignedTriangles = 0;
    std::unordered_map<std::string, unsigned int> planetTextures;
    std::unordered_map<std::string, PlanetTextureDiagnostic> planetTextureDiagnostics;
    float lastCelestialTelemetryLog = -100.0f;
    std::unordered_map<std::string, unsigned int> shipTextures;
    bool importedPlanetPackReady = false;
};

namespace {
constexpr float kSectorPositionScale = SystemSpatialScale::SectorToWorld;

float Distance2D(const Vector3& a, const Vector3& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}


#ifdef _WIN32
GLuint LoadWicTexture2D(const fs::path& path, PlanetTextureDiagnostic* diagnostic=nullptr);
#endif

VisualModuleSource BuildModuleSource(const std::string& name,const ObjMeshData& mesh) {
    float minX=0,maxX=0,minY=0,maxY=0,minZ=0,maxZ=0;
    if(!mesh.positions.empty()){
        minX=maxX=mesh.positions.front().x;minY=maxY=mesh.positions.front().y;minZ=maxZ=mesh.positions.front().z;
        for(const auto& v:mesh.positions){minX=std::min(minX,v.x);maxX=std::max(maxX,v.x);minY=std::min(minY,v.y);maxY=std::max(maxY,v.y);minZ=std::min(minZ,v.z);maxZ=std::max(maxZ,v.z);}
    }
    VisualModuleSource source{name,std::max(.20f,(maxX-minX)*.5f),std::max(.20f,(maxZ-minZ)*.5f),std::max(.20f,(maxY-minY)*.5f)};
    const auto contacts=ShipyardGeometryAnalysisSystem::Analyze(mesh);
    auto copy=[](const ShipyardSurfaceContact& c){return VisualModuleSurfaceContact{c.point,c.normal,c.supportingArea,c.confidence,c.valid};};
    source.forwardSurface=copy(contacts.forward);source.aftSurface=copy(contacts.aft);source.portSurface=copy(contacts.port);source.starboardSurface=copy(contacts.starboard);
    source.dorsalSurface=copy(contacts.dorsal);source.ventralSurface=copy(contacts.ventral);
    return source;
}

#ifdef _WIN32
void CacheShipMaterialTextures(NativeBattlefieldRenderer::VisualAssets& assets,const ObjMeshData& mesh){
    for(const auto& material:mesh.materials){
        if(material.baseColorTexturePath.empty()||assets.shipTextures.count(material.baseColorTexturePath))continue;
        const GLuint texture=LoadWicTexture2D(fs::path(material.baseColorTexturePath));
        if(texture)assets.shipTextures.emplace(material.baseColorTexturePath,texture);
    }
}
#endif

fs::path FindRepositoryRoot() {
    std::error_code ec;
    fs::path probe = fs::current_path(ec);
    if (ec) return {};
    for (int i = 0; i < 7 && !probe.empty(); ++i) {
        if (fs::exists(probe / "content" / "derived" / "greyoxide_shipyard_v07", ec)) return probe;
        if (fs::exists(probe / "assets" / "Models" / "ships" / "modules", ec)) return probe;
        if (fs::exists(probe / "Assets" / "Models" / "ships" / "modules", ec)) return probe;
        if (fs::exists(probe / "GameData" / "Assets" / "Models" / "ships" / "modules", ec)) return probe;
        const fs::path parent = probe.parent_path();
        if (parent == probe) break;
        probe = parent;
    }
    return {};
}

void LoadVisualAssets(NativeBattlefieldRenderer::VisualAssets& assets) {
    assets.repositoryRoot = FindRepositoryRoot();
    if (assets.repositoryRoot.empty()) {
        Logger::Instance().Warning("Renderer", "Pass336 modular ship asset root was not found; geometric fallback remains available.");
        return;
    }

    // Prefer the repository authoring library when present because it contains
    // every current module; packaged builds normally fall back to GameData.
    const std::array<fs::path,3> candidates = {{
        assets.repositoryRoot / "assets" / "Models" / "ships" / "modules",
        assets.repositoryRoot / "Assets" / "Models" / "ships" / "modules",
        assets.repositoryRoot / "GameData" / "Assets" / "Models" / "ships" / "modules"
    }};
    fs::path moduleRoot;
    std::size_t bestCount = 0;
    std::error_code ec;
    for (const auto& candidate : candidates) {
        if (!fs::exists(candidate, ec)) continue;
        std::size_t count = 0;
        for (const auto& entry : fs::directory_iterator(candidate, ec)) {
            if (!ec && entry.is_regular_file() && entry.path().extension() == ".obj") ++count;
        }
        if (count > bestCount) { bestCount = count; moduleRoot = candidate; }
    }
    if (moduleRoot.empty()) {
        Logger::Instance().Warning("Renderer", "Pass336 found project root but no modular OBJ directory; fallback ship geometry remains available.");
        return;
    }

    ObjAssetLoader loader;
    std::vector<std::string> loadedNames;
    std::vector<VisualModuleSource> loadedSources;
    for (const auto& entry : fs::directory_iterator(moduleRoot, ec)) {
        if (ec || !entry.is_regular_file() || entry.path().extension() != ".obj") continue;
        const std::string name = entry.path().stem().string();
        ObjMeshData mesh;
        std::string error;
        if (loader.LoadFile(entry.path().string(), mesh, &error) && !mesh.triangles.empty()) {
            loadedSources.push_back(BuildModuleSource(name,mesh));
#ifdef _WIN32
            CacheShipMaterialTextures(assets,mesh);
#endif
            assets.shipModules.emplace(name, std::move(mesh));
            loadedNames.push_back(name);
        } else {
            Logger::Instance().Warning("Renderer", "Could not load native ship module " + entry.path().string() + ": " + error);
        }
    }
    // Pass421-425: Greyoxide Shipyard v0.7 lives in a governed derived
    // directory after the explicit Project Control intake action.  External
    // files never overwrite native module IDs and are ignored cleanly when the
    // optional CC0 source has not been fetched yet.
    const fs::path shipyardRoot = assets.repositoryRoot / "content" / "derived" / "greyoxide_shipyard_v07" / "certified" / "modules";
    if (fs::exists(shipyardRoot, ec)) {
        for (const auto& entry : fs::recursive_directory_iterator(shipyardRoot, ec)) {
            if (ec || !entry.is_regular_file() || entry.path().extension() != ".obj") continue;
            std::string name = entry.path().stem().string();
            if (name.rfind("shipyard_a_", 0) != 0) continue;
            ObjMeshData mesh;
            std::string error;
            if (loader.LoadFile(entry.path().string(), mesh, &error) && !mesh.triangles.empty()) {
                loadedSources.push_back(BuildModuleSource(name,mesh));
#ifdef _WIN32
                CacheShipMaterialTextures(assets,mesh);
#endif
                assets.shipyardMaterialRegions += mesh.materials.size();
                for(const auto& material:mesh.materials){if(material.resolvedFromMtl)++assets.shipyardResolvedMtlRegions;if(!material.baseColorTexturePath.empty())++assets.shipyardTexturedRegions;}
                if(mesh.materialNames.empty())++assets.shipyardUnzonedModules;
                for(const auto& materialName:mesh.materialNames){
                    const auto zone=ShipyardPaintZoneSystem::ForMaterial(materialName);
                    if(zone==ShipyardPaintZone::Inherited)++assets.shipyardUnmappedMaterialRegions;
                    else ++assets.shipyardSemanticMaterialRegions;
                }
                for(const auto& tri:mesh.triangles)if(tri.materialIndex<0)++assets.shipyardUnassignedTriangles;
                assets.shipModules[name] = std::move(mesh);
                loadedNames.push_back(name);
                ++assets.shipyardModuleCount;
            } else {
                Logger::Instance().Warning("Renderer", "Could not load certified Shipyard module " + entry.path().string() + ": " + error);
            }
        }
    }

    // R5 authored universe ships from the same Shipyard Strikes Back source.
    // These are real capturable ships; they are never fed back as generator templates.
    const fs::path authoredRoot = assets.repositoryRoot / "content" / "derived" / "shipyard_strikes_back_v1" / "authored_ships";
    if (fs::exists(authoredRoot, ec)) {
        for (const auto& entry : fs::directory_iterator(authoredRoot, ec)) {
            if (ec || !entry.is_regular_file() || entry.path().extension() != ".obj") continue;
            ObjMeshData mesh; std::string error; const std::string name=entry.path().stem().string();
            if (loader.LoadFile(entry.path().string(), mesh, &error) && !mesh.triangles.empty()) {
#ifdef _WIN32
                CacheShipMaterialTextures(assets,mesh);
#endif
                assets.shipModules[name]=std::move(mesh); loadedNames.push_back(name);
            } else Logger::Instance().Warning("Renderer","Could not load authored Shipyard ship "+entry.path().string()+": "+error);
        }
    }

    std::sort(loadedNames.begin(), loadedNames.end());
    assets.moduleLibraryReady = loadedNames.size() >= 8;
    assets.shipyardCatalog = ShipyardModuleSystem::BuildCatalog(loadedSources);
    assets.generatedVisualCatalog = ProceduralVisualVariantSystem::Build(loadedSources, 0x5A17C0DEu, 12);
    const auto shipyardShowcases = ShipyardModuleSystem::BuildShowcaseRecipes(loadedSources);
    assets.generatedVisualCatalog.shipRecipes.insert(assets.generatedVisualCatalog.shipRecipes.end(),
        shipyardShowcases.begin(), shipyardShowcases.end());

    // Pass615-654: every runtime-visible recipe, including the showcase lane,
    // passes the same whole-ship authority. No late insertion bypasses PCG
    // occupancy, command exposure, propulsion orientation or exhaust clearance.
    std::size_t repairedPcg=0,rejectedPcg=0;
    for(auto& recipe:assets.generatedVisualCatalog.shipRecipes){
        const auto before=ShipSpatialAssemblySystem::Validate(assets.shipyardCatalog,recipe);
        if(!before.valid && ShipPcgRuntimeClosureSystem::RepairCandidate(assets.shipyardCatalog,recipe,5)) ++repairedPcg;
        const auto certification=ShipPcgRuntimeClosureSystem::EvaluateCandidate(assets.shipyardCatalog,recipe,false,true);
        recipe.runtimePcgCertified=certification.accepted;
        recipe.runtimeCertificationMessage=certification.accepted?"PCG_RUNTIME_CERTIFIED":(certification.messages.empty()?"PCG_RUNTIME_REVIEW":certification.messages.front());
        if(!certification.accepted) ++rejectedPcg;
    }
    assets.canonicalAssetRegistry.Clear();
    const auto canonicalCount=ShipyardCanonicalAssetBridge::PopulateRegistry(assets.canonicalAssetRegistry,assets.shipyardCatalog);
    assets.shipyardReady = !shipyardShowcases.empty();
    Logger::Instance().Info("Renderer", "Pass421-425 modular visual library loaded " +
        std::to_string(loadedNames.size()) + " OBJ modules (" + std::to_string(assets.shipyardModuleCount) +
        " Shipyard R6 modules) and generated " +
        std::to_string(assets.generatedVisualCatalog.shipRecipes.size()) + " deterministic ship recipes at load. Shipyard materials: " +
        std::to_string(assets.shipyardResolvedMtlRegions) + "/" + std::to_string(assets.shipyardMaterialRegions) + " MTL-resolved, " +
        std::to_string(assets.shipyardTexturedRegions) + " base-color textured; semantic zones " +
        std::to_string(assets.shipyardSemanticMaterialRegions) + ", unmapped visible regions " +
        std::to_string(assets.shipyardUnmappedMaterialRegions) + ", unzoned modules " +
        std::to_string(assets.shipyardUnzonedModules) + ", unassigned triangles " +
        std::to_string(assets.shipyardUnassignedTriangles) + "; canonical asset identities " +
        std::to_string(canonicalCount) + "; PCG spatial repairs " + std::to_string(repairedPcg) +
        ", runtime-review recipes " + std::to_string(rejectedPcg) + ".");
    if(assets.shipyardUnmappedMaterialRegions>0||assets.shipyardUnzonedModules>0||assets.shipyardUnassignedTriangles>0){
        Logger::Instance().Warning("Renderer","Shipyard semantic-material coverage is incomplete; certified content must not silently fall back to neutral gray.");
    }
}

#ifdef _WIN32
constexpr float kPi = 3.14159265358979323846f;

struct NativeUiFontFace { GLuint base=0; int height=0; };
std::array<NativeUiFontFace,4> gUiFonts{};
bool gUiFontsReady=false;

GLuint LoadWicTexture2D(const fs::path& path, PlanetTextureDiagnostic* diagnostic) {
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool uninit = SUCCEEDED(co);
    IWICImagingFactory* factory=nullptr; IWICBitmapDecoder* decoder=nullptr; IWICBitmapFrameDecode* frame=nullptr; IWICFormatConverter* converter=nullptr;
    GLuint texture=0;
    HRESULT hr=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory));
    if(SUCCEEDED(hr)) hr=factory->CreateDecoderFromFilename(path.wstring().c_str(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&decoder);
    if(SUCCEEDED(hr)) hr=decoder->GetFrame(0,&frame);
    if(SUCCEEDED(hr)) hr=factory->CreateFormatConverter(&converter);
    if(SUCCEEDED(hr)) hr=converter->Initialize(frame,GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,nullptr,0.0,WICBitmapPaletteTypeCustom);
    UINT w=0,h=0;if(SUCCEEDED(hr))hr=converter->GetSize(&w,&h);
    std::vector<unsigned char> pixels;
    if(SUCCEEDED(hr)&&w&&h){pixels.resize(static_cast<std::size_t>(w)*h*4);hr=converter->CopyPixels(nullptr,w*4,static_cast<UINT>(pixels.size()),pixels.data());}
    int mipLevels=1;float anisotropy=1.0f;
    if(SUCCEEDED(hr)&&!pixels.empty()){
        glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
        // Upload a complete CPU-generated mip chain. The old single-level GL_LINEAR
        // path caused high-frequency planet artwork to shimmer/grain at orbital scale.
        std::vector<unsigned char> levelPixels=std::move(pixels);UINT lw=w,lh=h;int level=0;
        for(;;){
            glTexImage2D(GL_TEXTURE_2D,level,GL_RGBA,static_cast<GLsizei>(lw),static_cast<GLsizei>(lh),0,GL_RGBA,GL_UNSIGNED_BYTE,levelPixels.data());
            if(lw==1&&lh==1)break;
            const UINT nw=std::max<UINT>(1,lw/2),nh=std::max<UINT>(1,lh/2);std::vector<unsigned char> next(static_cast<std::size_t>(nw)*nh*4);
            for(UINT y=0;y<nh;++y)for(UINT x=0;x<nw;++x)for(int c=0;c<4;++c){
                unsigned sum=0,count=0;for(UINT oy=0;oy<2;++oy)for(UINT ox=0;ox<2;++ox){const UINT sx=std::min(lw-1,x*2+ox),sy=std::min(lh-1,y*2+oy);sum+=levelPixels[(static_cast<std::size_t>(sy)*lw+sx)*4+c];++count;}
                next[(static_cast<std::size_t>(y)*nw+x)*4+c]=static_cast<unsigned char>(sum/count);
            }
            levelPixels.swap(next);lw=nw;lh=nh;++level;
        }
        mipLevels=level+1;
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
        const char* extensions=reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if(extensions&&std::strstr(extensions,"GL_EXT_texture_filter_anisotropic")){
            GLfloat maxSupported=1.0f;glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,&maxSupported);anisotropy=std::clamp(static_cast<float>(maxSupported),1.0f,8.0f);glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAX_ANISOTROPY_EXT,anisotropy);
        }
        glBindTexture(GL_TEXTURE_2D,0);
    }
    if(diagnostic){diagnostic->width=static_cast<int>(w);diagnostic->height=static_cast<int>(h);diagnostic->mipLevels=mipLevels;diagnostic->anisotropy=anisotropy;}
    if(converter)converter->Release();if(frame)frame->Release();if(decoder)decoder->Release();if(factory)factory->Release();if(uninit)CoUninitialize();
    return texture;
}

void LoadImportedPlanetTextures(NativeBattlefieldRenderer::VisualAssets& assets){
    assets.importedPlanetPackReady=ImportedPlanetVisualSystem::PackReady(assets.repositoryRoot);
    if(!assets.importedPlanetPackReady)return;
    const auto root=ImportedPlanetVisualSystem::PackRoot(assets.repositoryRoot);
    const PlanetType types[]={PlanetType::Rocky,PlanetType::Desert,PlanetType::Ice,PlanetType::Oceanic,PlanetType::Volcanic,PlanetType::Barren,PlanetType::GasGiant};
    for(auto type:types){
        const auto profile=ImportedPlanetVisualSystem::ProfileFor(type);
        std::vector<std::string> rels{profile.surfaceTexture};rels.insert(rels.end(),profile.cloudTextures.begin(),profile.cloudTextures.end());
        for(const auto& rel:rels){if(rel.empty()||assets.planetTextures.count(rel))continue;PlanetTextureDiagnostic diagnostic;const auto tex=LoadWicTexture2D(root/rel,&diagnostic);if(tex){assets.planetTextures.emplace(rel,tex);assets.planetTextureDiagnostics.emplace(rel,diagnostic);}}
    }
    std::size_t totalPixels=0;int largestW=0,largestH=0,maxMips=1;float maxAniso=1.0f;
    for(const auto& kv:assets.planetTextureDiagnostics){totalPixels+=static_cast<std::size_t>(std::max(0,kv.second.width))*static_cast<std::size_t>(std::max(0,kv.second.height));largestW=std::max(largestW,kv.second.width);largestH=std::max(largestH,kv.second.height);maxMips=std::max(maxMips,kv.second.mipLevels);maxAniso=std::max(maxAniso,kv.second.anisotropy);}
    Logger::Instance().Info("Renderer","Pass475 Various Planets fidelity provider loaded "+std::to_string(assets.planetTextures.size())+" source texture surfaces/cloud layers; largest="+std::to_string(largestW)+"x"+std::to_string(largestH)+", mipLevels="+std::to_string(maxMips)+", anisotropy="+std::to_string(maxAniso)+", sourcePixels="+std::to_string(totalPixels)+".");
}

void ReleaseImportedPlanetTextures(NativeBattlefieldRenderer::VisualAssets& assets){
    for(auto& kv:assets.planetTextures){GLuint id=static_cast<GLuint>(kv.second);if(id)glDeleteTextures(1,&id);}assets.planetTextures.clear();assets.planetTextureDiagnostics.clear();assets.importedPlanetPackReady=false;
}

bool InitializeNativeUiFonts(){
    HDC dc=wglGetCurrentDC();if(!dc)return false;
    const int heights[]={14,16,19,24};
    for(std::size_t i=0;i<gUiFonts.size();++i){
        HFONT font=CreateFontW(-heights[i],0,0,0,FW_MEDIUM,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
            OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,FF_DONTCARE|DEFAULT_PITCH,L"Segoe UI");
        if(!font)continue;HGDIOBJ old=SelectObject(dc,font);const GLuint base=glGenLists(96);
        const BOOL ok=base!=0&&wglUseFontBitmapsW(dc,32,96,base);SelectObject(dc,old);DeleteObject(font);
        if(ok){gUiFonts[i]={base,heights[i]};}else if(base)glDeleteLists(base,96);
    }
    gUiFontsReady=std::any_of(gUiFonts.begin(),gUiFonts.end(),[](const NativeUiFontFace&f){return f.base!=0;});
    return gUiFontsReady;
}
void ShutdownNativeUiFonts(){for(auto&f:gUiFonts){if(f.base)glDeleteLists(f.base,96);f={};}gUiFontsReady=false;}
const NativeUiFontFace* PickUiFont(float legacyPixel){
    const int wanted=std::clamp(static_cast<int>(std::lround(12.0f+legacyPixel*4.2f)),14,24);const NativeUiFontFace*best=nullptr;int delta=999;
    for(const auto&f:gUiFonts)if(f.base){const int d=std::abs(f.height-wanted);if(d<delta){delta=d;best=&f;}}return best;
}
bool DrawNativeUiText(const std::string& text,float x,float y,float legacyPixel,float r,float g,float b,float a){
    if(!gUiFontsReady)return false;const auto*font=PickUiFont(legacyPixel);if(!font)return false;
    glColor4f(r,g,b,a);glRasterPos2f(x,y+static_cast<float>(font->height));glListBase(font->base-32);
    std::string clean;clean.reserve(text.size());for(unsigned char c:text)clean.push_back((c>=32&&c<=126)?static_cast<char>(c):'?');
    glCallLists(static_cast<GLsizei>(clean.size()),GL_UNSIGNED_BYTE,clean.data());return true;
}

std::string ToUpperAscii(std::string value) {
    for (char& c : value) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    }
    return value;
}

struct Rgba { float r, g, b, a; };

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#endif

using GlCreateShaderProc = GLuint (APIENTRY*)(GLenum);
using GlShaderSourceProc = void (APIENTRY*)(GLuint, GLsizei, const char* const*, const GLint*);
using GlCompileShaderProc = void (APIENTRY*)(GLuint);
using GlGetShaderivProc = void (APIENTRY*)(GLuint, GLenum, GLint*);
using GlGetShaderInfoLogProc = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
using GlDeleteShaderProc = void (APIENTRY*)(GLuint);
using GlCreateProgramProc = GLuint (APIENTRY*)();
using GlAttachShaderProc = void (APIENTRY*)(GLuint, GLuint);
using GlLinkProgramProc = void (APIENTRY*)(GLuint);
using GlGetProgramivProc = void (APIENTRY*)(GLuint, GLenum, GLint*);
using GlGetProgramInfoLogProc = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
using GlDeleteProgramProc = void (APIENTRY*)(GLuint);
using GlUseProgramProc = void (APIENTRY*)(GLuint);
using GlGetUniformLocationProc = GLint (APIENTRY*)(GLuint, const char*);
using GlUniform1fProc = void (APIENTRY*)(GLint, GLfloat);
using GlUniform1iProc = void (APIENTRY*)(GLint, GLint);
using GlUniform3fProc = void (APIENTRY*)(GLint, GLfloat, GLfloat, GLfloat);

struct CompatShaderRuntime {
    GlCreateShaderProc createShader=nullptr; GlShaderSourceProc shaderSource=nullptr;
    GlCompileShaderProc compileShader=nullptr; GlGetShaderivProc getShaderiv=nullptr;
    GlGetShaderInfoLogProc getShaderInfoLog=nullptr; GlDeleteShaderProc deleteShader=nullptr;
    GlCreateProgramProc createProgram=nullptr; GlAttachShaderProc attachShader=nullptr;
    GlLinkProgramProc linkProgram=nullptr; GlGetProgramivProc getProgramiv=nullptr;
    GlGetProgramInfoLogProc getProgramInfoLog=nullptr; GlDeleteProgramProc deleteProgram=nullptr;
    GlUseProgramProc useProgram=nullptr; GlGetUniformLocationProc getUniformLocation=nullptr;
    GlUniform1fProc uniform1f=nullptr; GlUniform1iProc uniform1i=nullptr; GlUniform3fProc uniform3f=nullptr;
    GLuint program=0; bool ready=false;
    GLint lightDirection=-1,sunColor=-1,detailColor=-1,ambient=-1,celBands=-1,rimStrength=-1,
          metallic=-1,roughness=-1,emission=-1,specularStrength=-1,fresnelStrength=-1,
          surfaceMode=-1,surfaceVariation=-1,detailScale=-1,bandStrength=-1,lavaGlow=-1,surfaceSeed=-1,
          edgeHighlight=-1,cavityStrength=-1,wearStrength=-1,time=-1,baseColorTexture=-1,useBaseColorTexture=-1;
};

CompatShaderRuntime gShader;
Rgba gSolarColor{1.0f,0.86f,0.62f,1.0f};
Vector3 gSolarDirection{-0.35f,-0.52f,0.78f};
Vector3 gCameraEye{0.0f,0.0f,10.0f};
float gSolarAmbient=0.12f;
float gSolarLuminosity=1.0f;
float gRenderTime=0.0f;

enum class CelestialRenderTier { Near, Mid, Far };
struct CelestialFrameTelemetry { int nearBodies=0,midBodies=0,farBodies=0,importedCloudLayers=0,proceduralCloudLayers=0; };
CelestialFrameTelemetry gCelestialFrameTelemetry{};

struct SurfaceShaderOverrideState {
    bool active=false;
    float detailR=.4f,detailG=.42f,detailB=.44f;
    float variation=0.0f,detailScale=4.0f,bandStrength=0.0f,lavaGlow=0.0f,surfaceSeed=1.0f;
};
SurfaceShaderOverrideState gSurfaceOverride;

void* LoadGlProc(const char* name) {
    void* p=reinterpret_cast<void*>(wglGetProcAddress(name));
    if(!p || p==reinterpret_cast<void*>(1) || p==reinterpret_cast<void*>(2) ||
       p==reinterpret_cast<void*>(3) || p==reinterpret_cast<void*>(-1)) {
        static HMODULE module=LoadLibraryA("opengl32.dll");
        p=module?reinterpret_cast<void*>(GetProcAddress(module,name)):nullptr;
    }
    return p;
}

template <typename T> T LoadGl(const char* name){return reinterpret_cast<T>(LoadGlProc(name));}

bool CompileShaderStage(GLuint shader,const char* source,std::string& error){
    gShader.shaderSource(shader,1,&source,nullptr); gShader.compileShader(shader);
    GLint ok=0; gShader.getShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(ok) return true;
    GLint length=0; gShader.getShaderiv(shader,GL_INFO_LOG_LENGTH,&length);
    std::string log(static_cast<std::size_t>(std::max(1,length)),'\0'); GLsizei written=0;
    gShader.getShaderInfoLog(shader,static_cast<GLsizei>(log.size()),&written,log.data());
    error=log; return false;
}

bool InitializeSpaceShader(){
    gShader.createShader=LoadGl<GlCreateShaderProc>("glCreateShader");
    gShader.shaderSource=LoadGl<GlShaderSourceProc>("glShaderSource");
    gShader.compileShader=LoadGl<GlCompileShaderProc>("glCompileShader");
    gShader.getShaderiv=LoadGl<GlGetShaderivProc>("glGetShaderiv");
    gShader.getShaderInfoLog=LoadGl<GlGetShaderInfoLogProc>("glGetShaderInfoLog");
    gShader.deleteShader=LoadGl<GlDeleteShaderProc>("glDeleteShader");
    gShader.createProgram=LoadGl<GlCreateProgramProc>("glCreateProgram");
    gShader.attachShader=LoadGl<GlAttachShaderProc>("glAttachShader");
    gShader.linkProgram=LoadGl<GlLinkProgramProc>("glLinkProgram");
    gShader.getProgramiv=LoadGl<GlGetProgramivProc>("glGetProgramiv");
    gShader.getProgramInfoLog=LoadGl<GlGetProgramInfoLogProc>("glGetProgramInfoLog");
    gShader.deleteProgram=LoadGl<GlDeleteProgramProc>("glDeleteProgram");
    gShader.useProgram=LoadGl<GlUseProgramProc>("glUseProgram");
    gShader.getUniformLocation=LoadGl<GlGetUniformLocationProc>("glGetUniformLocation");
    gShader.uniform1f=LoadGl<GlUniform1fProc>("glUniform1f");
    gShader.uniform1i=LoadGl<GlUniform1iProc>("glUniform1i");
    gShader.uniform3f=LoadGl<GlUniform3fProc>("glUniform3f");
    if(!gShader.createShader||!gShader.shaderSource||!gShader.compileShader||!gShader.getShaderiv||
       !gShader.getShaderInfoLog||!gShader.deleteShader||!gShader.createProgram||!gShader.attachShader||
       !gShader.linkProgram||!gShader.getProgramiv||!gShader.getProgramInfoLog||!gShader.deleteProgram||
       !gShader.useProgram||!gShader.getUniformLocation||!gShader.uniform1f||!gShader.uniform1i||!gShader.uniform3f) return false;

    GLuint vs=gShader.createShader(GL_VERTEX_SHADER), fs=gShader.createShader(GL_FRAGMENT_SHADER);
    std::string error;
    if(!CompileShaderStage(vs,SpaceMaterialSystem::VertexShader120(),error)){
        Logger::Instance().Warning("Renderer","Pass338 vertex shader compile failed; using fixed-function fallback: "+error);
        gShader.deleteShader(vs);gShader.deleteShader(fs);return false;
    }
    if(!CompileShaderStage(fs,SpaceMaterialSystem::FragmentShader120(),error)){
        Logger::Instance().Warning("Renderer","Pass338 fragment shader compile failed; using fixed-function fallback: "+error);
        gShader.deleteShader(vs);gShader.deleteShader(fs);return false;
    }
    gShader.program=gShader.createProgram(); gShader.attachShader(gShader.program,vs);gShader.attachShader(gShader.program,fs);gShader.linkProgram(gShader.program);
    GLint linked=0;gShader.getProgramiv(gShader.program,GL_LINK_STATUS,&linked);
    gShader.deleteShader(vs);gShader.deleteShader(fs);
    if(!linked){
        GLint length=0;gShader.getProgramiv(gShader.program,GL_INFO_LOG_LENGTH,&length);
        std::string log(static_cast<std::size_t>(std::max(1,length)),'\0');GLsizei written=0;
        gShader.getProgramInfoLog(gShader.program,static_cast<GLsizei>(log.size()),&written,log.data());
        Logger::Instance().Warning("Renderer","Pass338 shader link failed; using fixed-function fallback: "+log);
        gShader.deleteProgram(gShader.program);gShader.program=0;return false;
    }
    gShader.lightDirection=gShader.getUniformLocation(gShader.program,"uLightDirection");
    gShader.sunColor=gShader.getUniformLocation(gShader.program,"uSunColor");
    gShader.detailColor=gShader.getUniformLocation(gShader.program,"uDetailColor");
    gShader.ambient=gShader.getUniformLocation(gShader.program,"uAmbient");
    gShader.celBands=gShader.getUniformLocation(gShader.program,"uCelBands");
    gShader.rimStrength=gShader.getUniformLocation(gShader.program,"uRimStrength");
    gShader.metallic=gShader.getUniformLocation(gShader.program,"uMetallic");
    gShader.roughness=gShader.getUniformLocation(gShader.program,"uRoughness");
    gShader.emission=gShader.getUniformLocation(gShader.program,"uEmission");
    gShader.specularStrength=gShader.getUniformLocation(gShader.program,"uSpecularStrength");
    gShader.fresnelStrength=gShader.getUniformLocation(gShader.program,"uFresnelStrength");
    gShader.surfaceMode=gShader.getUniformLocation(gShader.program,"uSurfaceMode");
    gShader.surfaceVariation=gShader.getUniformLocation(gShader.program,"uSurfaceVariation");
    gShader.detailScale=gShader.getUniformLocation(gShader.program,"uDetailScale");
    gShader.bandStrength=gShader.getUniformLocation(gShader.program,"uBandStrength");
    gShader.lavaGlow=gShader.getUniformLocation(gShader.program,"uLavaGlow");
    gShader.surfaceSeed=gShader.getUniformLocation(gShader.program,"uSurfaceSeed");
    gShader.edgeHighlight=gShader.getUniformLocation(gShader.program,"uEdgeHighlight");
    gShader.cavityStrength=gShader.getUniformLocation(gShader.program,"uCavityStrength");
    gShader.wearStrength=gShader.getUniformLocation(gShader.program,"uWearStrength");
    gShader.time=gShader.getUniformLocation(gShader.program,"uTime");
    gShader.baseColorTexture=gShader.getUniformLocation(gShader.program,"uBaseColorTexture");
    gShader.useBaseColorTexture=gShader.getUniformLocation(gShader.program,"uUseBaseColorTexture");
    gShader.ready=true;
    gShader.useProgram(gShader.program);
    if(gShader.baseColorTexture>=0)gShader.uniform1i(gShader.baseColorTexture,0);
    if(gShader.useBaseColorTexture>=0)gShader.uniform1f(gShader.useBaseColorTexture,0.0f);
    Logger::Instance().Info("Renderer","Pass338 compatibility GLSL material/surface shader active.");
    return true;
}

void ShutdownSpaceShader(){
    if(gShader.ready&&gShader.useProgram)gShader.useProgram(0);
    if(gShader.program&&gShader.deleteProgram)gShader.deleteProgram(gShader.program);
    gShader=CompatShaderRuntime{};
}

void ApplyShaderMaterial(SpaceMaterialKind kind,float extraEmission){
    if(!gShader.ready)return;
    auto p=SpaceMaterialSystem::GetProfile(kind);
    if(gSurfaceOverride.active && p.surfaceMode>0.5f){
        p.detailR=gSurfaceOverride.detailR;p.detailG=gSurfaceOverride.detailG;p.detailB=gSurfaceOverride.detailB;
        p.surfaceVariation=gSurfaceOverride.variation;p.detailScale=gSurfaceOverride.detailScale;
        p.bandStrength=gSurfaceOverride.bandStrength;p.lavaGlow=gSurfaceOverride.lavaGlow;
    }
    gShader.useProgram(gShader.program);
    if(gShader.useBaseColorTexture>=0)gShader.uniform1f(gShader.useBaseColorTexture,0.0f);
    if(gShader.lightDirection>=0)gShader.uniform3f(gShader.lightDirection,gSolarDirection.x,gSolarDirection.y,gSolarDirection.z);
    if(gShader.sunColor>=0)gShader.uniform3f(gShader.sunColor,gSolarColor.r,gSolarColor.g,gSolarColor.b);
    if(gShader.detailColor>=0)gShader.uniform3f(gShader.detailColor,p.detailR,p.detailG,p.detailB);
    if(gShader.ambient>=0)gShader.uniform1f(gShader.ambient,gSolarAmbient);
    if(gShader.celBands>=0)gShader.uniform1f(gShader.celBands,p.celBands);
    if(gShader.rimStrength>=0)gShader.uniform1f(gShader.rimStrength,p.rimStrength);
    if(gShader.metallic>=0)gShader.uniform1f(gShader.metallic,p.metallic);
    if(gShader.roughness>=0)gShader.uniform1f(gShader.roughness,p.roughness);
    if(gShader.emission>=0)gShader.uniform1f(gShader.emission,std::max(p.emissive,extraEmission));
    if(gShader.specularStrength>=0)gShader.uniform1f(gShader.specularStrength,p.specularStrength);
    if(gShader.fresnelStrength>=0)gShader.uniform1f(gShader.fresnelStrength,p.fresnelStrength);
    if(gShader.surfaceMode>=0)gShader.uniform1f(gShader.surfaceMode,p.surfaceMode);
    if(gShader.surfaceVariation>=0)gShader.uniform1f(gShader.surfaceVariation,p.surfaceVariation);
    if(gShader.detailScale>=0)gShader.uniform1f(gShader.detailScale,p.detailScale);
    if(gShader.bandStrength>=0)gShader.uniform1f(gShader.bandStrength,p.bandStrength);
    if(gShader.lavaGlow>=0)gShader.uniform1f(gShader.lavaGlow,p.lavaGlow);
    if(gShader.surfaceSeed>=0)gShader.uniform1f(gShader.surfaceSeed,gSurfaceOverride.active?gSurfaceOverride.surfaceSeed:1.0f);
    if(gShader.edgeHighlight>=0)gShader.uniform1f(gShader.edgeHighlight,p.edgeHighlight);
    if(gShader.cavityStrength>=0)gShader.uniform1f(gShader.cavityStrength,p.cavityStrength);
    if(gShader.wearStrength>=0)gShader.uniform1f(gShader.wearStrength,p.wearStrength);
    if(gShader.time>=0)gShader.uniform1f(gShader.time,gRenderTime);
}

void SetShipBaseTexture(GLuint texture){
    if(texture){glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture);}
    else{glBindTexture(GL_TEXTURE_2D,0);glDisable(GL_TEXTURE_2D);}
    if(gShader.ready){gShader.useProgram(gShader.program);if(gShader.baseColorTexture>=0)gShader.uniform1i(gShader.baseColorTexture,0);if(gShader.useBaseColorTexture>=0)gShader.uniform1f(gShader.useBaseColorTexture,texture?1.0f:0.0f);}
}

void DisableShader(){if(gShader.ready&&gShader.useProgram)gShader.useProgram(0);}

void Color(const Rgba& c) { glColor4f(c.r, c.g, c.b, c.a); }

void SetMaterial(const Rgba& c, float shininess = 24.0f, float emission = 0.0f,
                 SpaceMaterialKind kind = SpaceMaterialKind::ShipHull, float metallicOverride=-1.0f, float roughnessOverride=-1.0f) {
    ApplyShaderMaterial(kind,emission);
    if(gShader.ready){if(metallicOverride>=0.0f&&gShader.metallic>=0)gShader.uniform1f(gShader.metallic,std::clamp(metallicOverride,0.0f,1.0f));if(roughnessOverride>=0.0f&&gShader.roughness>=0)gShader.uniform1f(gShader.roughness,std::clamp(roughnessOverride,0.0f,1.0f));}
    glColor4f(c.r, c.g, c.b, c.a);
    const GLfloat diffuse[4] = {c.r, c.g, c.b, c.a};
    const GLfloat specular[4] = {0.34f, 0.38f, 0.42f, c.a};
    const GLfloat emissive[4] = {c.r*emission, c.g*emission, c.b*emission, c.a};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, std::clamp(shininess, 0.0f, 96.0f));
}

void FilledCircle(float x, float y, float z, float radius, const Rgba& c, int segments = 48) {
    Color(c);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x, y, z);
    for (int i = 0; i <= segments; ++i) {
        const float a = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * kPi;
        glVertex3f(x + std::cos(a) * radius, y + std::sin(a) * radius, z);
    }
    glEnd();
}

void Ring(float x, float y, float z, float radius, const Rgba& c, float width = 1.0f, int segments = 64) {
    glLineWidth(width);
    Color(c);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        const float a = static_cast<float>(i) / static_cast<float>(segments) * 2.0f * kPi;
        glVertex3f(x + std::cos(a) * radius, y + std::sin(a) * radius, z);
    }
    glEnd();
}

void Line(float x1, float y1, float z1, float x2, float y2, float z2, const Rgba& c, float width = 1.0f) {
    glLineWidth(width);
    Color(c);
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

void FilledRect(float x, float y, float z, float w, float h, const Rgba& c) {
    Color(c);
    glBegin(GL_QUADS);
    glVertex3f(x, y, z); glVertex3f(x + w, y, z);
    glVertex3f(x + w, y + h, z); glVertex3f(x, y + h, z);
    glEnd();
}

void DrawBox(float cx, float cy, float cz, float sx, float sy, float sz, const Rgba& c, SpaceMaterialKind kind=SpaceMaterialKind::ShipHull) {
    const float x0=cx-sx*0.5f,x1=cx+sx*0.5f;
    const float y0=cy-sy*0.5f,y1=cy+sy*0.5f;
    const float z0=cz-sz*0.5f,z1=cz+sz*0.5f;
    SetMaterial(c, 32.0f, 0.0f, kind);
    glBegin(GL_QUADS);
    glNormal3f(0,0,1); glVertex3f(x0,y0,z1); glVertex3f(x1,y0,z1); glVertex3f(x1,y1,z1); glVertex3f(x0,y1,z1);
    glNormal3f(0,0,-1); glVertex3f(x0,y1,z0); glVertex3f(x1,y1,z0); glVertex3f(x1,y0,z0); glVertex3f(x0,y0,z0);
    glNormal3f(1,0,0); glVertex3f(x1,y0,z0); glVertex3f(x1,y1,z0); glVertex3f(x1,y1,z1); glVertex3f(x1,y0,z1);
    glNormal3f(-1,0,0); glVertex3f(x0,y0,z1); glVertex3f(x0,y1,z1); glVertex3f(x0,y1,z0); glVertex3f(x0,y0,z0);
    glNormal3f(0,1,0); glVertex3f(x0,y1,z1); glVertex3f(x1,y1,z1); glVertex3f(x1,y1,z0); glVertex3f(x0,y1,z0);
    glNormal3f(0,-1,0); glVertex3f(x0,y0,z0); glVertex3f(x1,y0,z0); glVertex3f(x1,y0,z1); glVertex3f(x0,y0,z1);
    glEnd();
}

Rgba PlanetColor(PlanetType t) {
    switch (t) {
        case PlanetType::Rocky:    return {0.44f,0.40f,0.35f,1.0f};
        case PlanetType::Desert:   return {0.70f,0.43f,0.20f,1.0f};
        case PlanetType::Ice:      return {0.48f,0.72f,0.84f,1.0f};
        case PlanetType::Oceanic:  return {0.11f,0.32f,0.61f,1.0f};
        case PlanetType::Volcanic: return {0.43f,0.12f,0.07f,1.0f};
        case PlanetType::Barren:   return {0.37f,0.31f,0.27f,1.0f};
        case PlanetType::GasGiant: return {0.58f,0.46f,0.34f,1.0f};
    }
    return {0.5f,0.5f,0.5f,1.0f};
}

void SetupScreenProjection(int w, int h) {
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0.0, static_cast<double>(w), static_cast<double>(h), 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

void SetupPerspectiveProjection(const NativeBattlefieldFrame& frame) {
    const StrategicViewBasis b = StrategicViewProjection::Build(
        *frame.camera, static_cast<float>(frame.viewportWidth), static_cast<float>(frame.viewportHeight));
    gCameraEye=b.eye;

    const float top = b.nearPlane*b.tanHalfFov;
    const float right = top*b.aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, b.nearPlane, b.farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const GLfloat m[16] = {
        b.right.x, b.up.x, -b.forward.x, 0.0f,
        b.right.y, b.up.y, -b.forward.y, 0.0f,
        b.right.z, b.up.z, -b.forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(m);
    glTranslatef(-b.eye.x, -b.eye.y, -b.eye.z);
}

void ConfigureSolarLighting(const NativeBattlefieldFrame& frame) {
    if(!frame.sector || !frame.sector->hasStar) return;
    const auto& star=frame.sector->star;
    gSolarColor={star.colorR,star.colorG,star.colorB,1.0f};
    const Vector3 starWorld=NativeBattlefieldRenderer::SectorToWorld(star.position);
    const Vector3 reference=frame.camera?frame.camera->GetCenter():Vector3{};
    const auto light=SolarPresentationSystem::EvaluateLight(star,starWorld,reference);
    gSolarDirection=light.direction;
    gSolarLuminosity=light.diffuseIntensity;
    gSolarAmbient=light.ambientIntensity;
}

void SetupSceneLighting() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    const GLfloat ambient[4] = {gSolarAmbient*0.72f,gSolarAmbient*0.82f,gSolarAmbient,1.0f};
    const GLfloat diffuse[4] = {gSolarColor.r*gSolarLuminosity,gSolarColor.g*gSolarLuminosity,gSolarColor.b*gSolarLuminosity,1.0f};
    const GLfloat specular[4] = {0.46f*gSolarColor.r,0.52f*gSolarColor.g,0.62f*gSolarColor.b,1.0f};
    const GLfloat direction[4] = {gSolarDirection.x,gSolarDirection.y,gSolarDirection.z,0.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, direction);
}

void DisableSceneLighting() {
    DisableShader();
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
}

void DrawSphere(float cx, float cy, float cz, float radius, const Rgba& c, int slices=36, int stacks=18, SpaceMaterialKind kind=SpaceMaterialKind::PlanetRock) {
    SetMaterial(c, 18.0f, 0.0f, kind);
    for (int stack=0; stack<stacks; ++stack) {
        const float v0 = static_cast<float>(stack)/stacks;
        const float v1 = static_cast<float>(stack+1)/stacks;
        const float lat0 = -kPi*0.5f + v0*kPi;
        const float lat1 = -kPi*0.5f + v1*kPi;
        const float z0 = std::sin(lat0), zr0 = std::cos(lat0);
        const float z1 = std::sin(lat1), zr1 = std::cos(lat1);
        glBegin(GL_QUAD_STRIP);
        for (int slice=0; slice<=slices; ++slice) {
            const float lon = static_cast<float>(slice)/slices*2.0f*kPi;
            const float x = std::cos(lon), y = std::sin(lon);
            glNormal3f(x*zr0,y*zr0,z0); glVertex3f(cx+radius*x*zr0,cy+radius*y*zr0,cz+radius*z0);
            glNormal3f(x*zr1,y*zr1,z1); glVertex3f(cx+radius*x*zr1,cy+radius*y*zr1,cz+radius*z1);
        }
        glEnd();
    }
}

void DrawUnlitSphere(float cx,float cy,float cz,float radius,const Rgba& c,int slices=32,int stacks=14) {
    DisableShader(); Color(c);
    for(int stack=0;stack<stacks;++stack){
        const float v0=static_cast<float>(stack)/stacks,v1=static_cast<float>(stack+1)/stacks;
        const float lat0=-kPi*0.5f+v0*kPi,lat1=-kPi*0.5f+v1*kPi;
        const float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);
        glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){
            const float lon=static_cast<float>(slice)/slices*2.0f*kPi;
            const float x=std::cos(lon),y=std::sin(lon);
            glVertex3f(cx+radius*x*zr0,cy+radius*y*zr0,cz+radius*z0);
            glVertex3f(cx+radius*x*zr1,cy+radius*y*zr1,cz+radius*z1);
        }
        glEnd();
    }
}

float AnimatedCloudMask(float longitude,float latitude,float seed,float time,float drift) {
    const float phase=seed*0.00071f;
    const float weatherLon=PlanetPresentationSystem::ProceduralWeatherLongitude(longitude,time,drift);
    const float a=0.50f+0.25f*std::sin(weatherLon*3.0f+latitude*1.8f+phase);
    const float b=0.18f*std::sin(weatherLon*7.0f-latitude*4.5f+phase*2.3f-time*0.013f*drift);
    const float c=0.12f*std::sin(weatherLon*13.0f+latitude*8.0f-phase*1.7f+time*0.009f*drift);
    float t=std::clamp((a+b+c-0.43f)/0.34f,0.0f,1.0f);
    return t*t*(3.0f-2.0f*t);
}

void DrawAnimatedCloudSphere(float cx,float cy,float cz,float radius,const Rgba& c,float seed,float time,float drift,
                             int slices=44,int stacks=22) {
    DisableShader();
    for(int stack=0;stack<stacks;++stack){
        const float v0=static_cast<float>(stack)/stacks,v1=static_cast<float>(stack+1)/stacks;
        const float lat0=-kPi*0.5f+v0*kPi,lat1=-kPi*0.5f+v1*kPi;
        const float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);
        glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){
            const float lon=static_cast<float>(slice)/slices*2.0f*kPi;
            const float x=std::cos(lon),y=std::sin(lon);
            const float a0=c.a*AnimatedCloudMask(lon,lat0,seed,time,drift);
            const float a1=c.a*AnimatedCloudMask(lon,lat1,seed,time,drift);
            glColor4f(c.r,c.g,c.b,a0);glVertex3f(cx+radius*x*zr0,cy+radius*y*zr0,cz+radius*z0);
            glColor4f(c.r,c.g,c.b,a1);glVertex3f(cx+radius*x*zr1,cy+radius*y*zr1,cz+radius*z1);
        }
        glEnd();
    }
}


float PlanetNoise(float lon,float lat,float seed,float spin){
    const float q=seed*.01731f;
    const float a=std::sin((lon+spin)*2.3f+std::sin(lat*2.0f+q)*1.1f);
    const float b=std::sin((lon+spin)*5.7f-lat*3.1f+q*1.7f);
    const float c=std::sin((lon+spin)*12.1f+lat*7.3f-q*.9f);
    return std::clamp(.5f+.25f*a+.16f*b+.09f*c,0.0f,1.0f);
}
Rgba PlanetSurfaceVertexColor(const PlanetData& planet,const PlanetSurfaceProfile& s,float lon,float lat,const Vector3& n){
    const float spin=gRenderTime*(.0035f+.000015f*std::fmod(std::fabs(s.surfaceSeed),19.0f));
    const float q=PlanetNoise(lon,lat,s.surfaceSeed,spin);
    float mix=q;
    Rgba c{s.baseColor[0],s.baseColor[1],s.baseColor[2],1.0f};
    if(planet.type==PlanetType::Oceanic){
        const float landThreshold=std::clamp(s.oceanFraction,0.48f,0.86f);
        const float coast=std::clamp((q-landThreshold+.05f)/.10f,0.0f,1.0f);
        c.r=c.r*(1-coast)+s.detailColor[0]*coast;c.g=c.g*(1-coast)+s.detailColor[1]*coast;c.b=c.b*(1-coast)+s.detailColor[2]*coast;
    } else if(planet.type==PlanetType::GasGiant){
        const float band=.5f+.5f*std::sin(lat*(9.0f+3.0f*s.bandStrength)+q*2.4f);
        mix=.22f+.68f*band;
        c.r=c.r*(1-mix)+s.detailColor[0]*mix;c.g=c.g*(1-mix)+s.detailColor[1]*mix;c.b=c.b*(1-mix)+s.detailColor[2]*mix;
    } else {
        mix=std::clamp((q-.28f)*.95f*s.surfaceVariation,0.0f,.82f);
        c.r=c.r*(1-mix)+s.detailColor[0]*mix;c.g=c.g*(1-mix)+s.detailColor[1]*mix;c.b=c.b*(1-mix)+s.detailColor[2]*mix;
    }
    const float ndotl=n.x*gSolarDirection.x+n.y*gSolarDirection.y+n.z*gSolarDirection.z;
    const float day=PlanetPresentationSystem::SmoothTerminator(ndotl);
    const float light=std::clamp(gSolarAmbient*.72f+day*gSolarLuminosity*.88f,.055f,1.18f);
    c.r=std::clamp(c.r*light,0.0f,1.0f);c.g=std::clamp(c.g*light,0.0f,1.0f);c.b=std::clamp(c.b*light,0.0f,1.0f);
    return c;
}
void DrawDedicatedPlanetSurface(const PlanetData& planet,const PlanetSurfaceProfile& s,float cx,float cy,float cz,float radius,int slices,int stacks){
    DisableSceneLighting();glDisable(GL_BLEND);glDepthMask(GL_TRUE);
    for(int stack=0;stack<stacks;++stack){
        const float v0=float(stack)/stacks,v1=float(stack+1)/stacks;const float lat0=-kPi*.5f+v0*kPi,lat1=-kPi*.5f+v1*kPi;
        const float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){const float lon=float(slice)/slices*2*kPi;const float x=std::cos(lon),y=std::sin(lon);Vector3 n0{x*zr0,y*zr0,z0},n1{x*zr1,y*zr1,z1};
            auto c0=PlanetSurfaceVertexColor(planet,s,lon,lat0,n0);glColor4f(c0.r,c0.g,c0.b,1);glVertex3f(cx+radius*n0.x,cy+radius*n0.y,cz+radius*n0.z);
            auto c1=PlanetSurfaceVertexColor(planet,s,lon,lat1,n1);glColor4f(c1.r,c1.g,c1.b,1);glVertex3f(cx+radius*n1.x,cy+radius*n1.y,cz+radius*n1.z);}
        glEnd();
    }
}
void DrawLitPlanetClouds(float cx,float cy,float cz,float radius,const Rgba& c,float seed,float drift,int slices,int stacks){
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
    for(int stack=0;stack<stacks;++stack){float v0=float(stack)/stacks,v1=float(stack+1)/stacks,lat0=-kPi*.5f+v0*kPi,lat1=-kPi*.5f+v1*kPi;float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){float lon=float(slice)/slices*2*kPi,x=std::cos(lon),y=std::sin(lon);Vector3 n0{x*zr0,y*zr0,z0},n1{x*zr1,y*zr1,z1};
            auto emit=[&](const Vector3& n,float lat){float nd=n.x*gSolarDirection.x+n.y*gSolarDirection.y+n.z*gSolarDirection.z;float day=PlanetPresentationSystem::CloudDaylight(nd);float a=c.a*AnimatedCloudMask(lon,lat,seed,gRenderTime,drift)*day;glColor4f(c.r*day,c.g*day,c.b*day,a);glVertex3f(cx+radius*n.x,cy+radius*n.y,cz+radius*n.z);};emit(n0,lat0);emit(n1,lat1);}
        glEnd();}
    glDepthMask(GL_TRUE);glDisable(GL_BLEND);
}
void DrawAtmosphereLimb(float cx,float cy,float cz,float radius,const Rgba& c,int slices,int stacks){
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);glDepthMask(GL_FALSE);
    for(int stack=0;stack<stacks;++stack){float v0=float(stack)/stacks,v1=float(stack+1)/stacks,lat0=-kPi*.5f+v0*kPi,lat1=-kPi*.5f+v1*kPi;float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){float lon=float(slice)/slices*2*kPi,x=std::cos(lon),y=std::sin(lon);Vector3 n0{x*zr0,y*zr0,z0},n1{x*zr1,y*zr1,z1};
            auto emit=[&](const Vector3& n){Vector3 pos{cx+radius*n.x,cy+radius*n.y,cz+radius*n.z};Vector3 view=(gCameraEye-pos).normalized();float vd=std::max(0.0f,n.x*view.x+n.y*view.y+n.z*view.z);float limb=PlanetPresentationSystem::AtmosphereLimb(vd);float nd=n.x*gSolarDirection.x+n.y*gSolarDirection.y+n.z*gSolarDirection.z;float sun=.20f+.80f*PlanetPresentationSystem::SmoothTerminator(nd);const auto atmosphere=PlanetAtmospherePresentationSystem::Default();float a=c.a*PlanetAtmospherePresentationSystem::Alpha(vd,atmosphere,sun);glColor4f(c.r,c.g,c.b,a);glVertex3f(pos.x,pos.y,pos.z);};emit(n0);emit(n1);}
        glEnd();}
    glDepthMask(GL_TRUE);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);
}

void DrawTorus(float cx,float cy,float cz,float majorRadius,float minorRadius,const Rgba& c,
               int majorSegments=40,int minorSegments=8,SpaceMaterialKind kind=SpaceMaterialKind::StationHull) {
    SetMaterial(c,42.0f,0.0f,kind);
    for(int i=0;i<majorSegments;++i){
        const float a0=static_cast<float>(i)/majorSegments*2*kPi;
        const float a1=static_cast<float>(i+1)/majorSegments*2*kPi;
        glBegin(GL_QUAD_STRIP);
        for(int j=0;j<=minorSegments;++j){
            const float b=static_cast<float>(j)/minorSegments*2*kPi;
            const float cb=std::cos(b), sb=std::sin(b);
            for(float a : {a0,a1}){
                const float ca=std::cos(a), sa=std::sin(a);
                const float rr=majorRadius+minorRadius*cb;
                glNormal3f(ca*cb,sa*cb,sb);
                glVertex3f(cx+ca*rr,cy+sa*rr,cz+minorRadius*sb);
            }
        }
        glEnd();
    }
}

void DrawStandaloneShipyardBackdrop(const NativeBattlefieldFrame& frame) {
    DisableShader();
    SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight);
    // Opaque engineering-canvas authority. This intentionally does not depend
    // on the window clear color, celestial scene, or alpha composition.
    FilledRect(0,0,0,w,h,{0.003f,0.008f,0.014f,1.0f});

    // Subtle center-view grid gives scale/orientation without competing with
    // the authored ship or the dark left/right Shipyard panels.
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const float left=400.0f,right=std::max(left+64.0f,w-360.0f),top=82.0f,bottom=std::max(top+64.0f,h-62.0f);
    for(float x=left;x<=right;x+=48.0f)Line(x,top,0,x,bottom,0,{0.08f,0.24f,0.29f,0.11f},1.0f);
    for(float y=top;y<=bottom;y+=48.0f)Line(left,y,0,right,y,0,{0.08f,0.24f,0.29f,0.11f},1.0f);
    Line((left+right)*.5f,top,0,(left+right)*.5f,bottom,0,{0.12f,0.46f,0.52f,0.18f},1.0f);
    Line(left,(top+bottom)*.5f,0,right,(top+bottom)*.5f,0,{0.12f,0.46f,0.52f,0.18f},1.0f);
    glDisable(GL_BLEND);
}

void DrawStarfield(const NativeBattlefieldFrame& frame) {
    DisableShader();
    SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const Vector3 c=frame.camera->GetCenter();
    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight);
    const auto& profile=frame.backdrop;

    // Pass296-300: layered deep-space backdrop. The galactic-density band is
    // deliberately subtle; most systems remain dark rather than becoming a
    // saturated nebula wallpaper.
    const float bandY=h*0.46f;
    for(int i=0;i<9;++i){
        const float x=w*(0.05f+i*0.12f)-c.x*(0.03f+i*0.004f);
        const float y=bandY+(i-4)*18.0f+c.y*0.015f + (x-w*.5f)*profile.galacticBandTilt*.08f;
        FilledCircle(x,y,0,150.0f+i*12.0f,{profile.nebulaR*.60f,profile.nebulaG*.60f,profile.nebulaB*.72f,profile.galacticBandStrength*0.13f},48);
    }

    // Pass516: deterministic volumetric-looking haze and a broken dust lane.
    // These are deliberately low-alpha so the field remains black industrial
    // space rather than a saturated wallpaper, while different systems now
    // have a recognizable visual identity.
    for(int i=0;i<7;++i){
        const float phase=float(i)*1.71f+float((frame.sector?frame.sector->x:0)*0.37f);
        const float x=w*(0.12f+0.13f*i)-c.x*(0.018f+0.004f*i);
        const float y=h*(0.30f+0.07f*std::sin(phase))+c.y*(0.010f+0.002f*i);
        const float radius=120.0f+35.0f*float(i%3);
        FilledCircle(x,y,0,radius,{profile.nebulaR,profile.nebulaG,profile.nebulaB,profile.nebulaHaze*(0.34f+0.05f*i)},56);
    }
    for(int i=0;i<5;++i){
        const float x=w*(0.18f+0.18f*i)-c.x*.021f;
        const float y=bandY+(i-2)*27.0f+c.y*.012f;
        FilledCircle(x,y,0,92.0f+i*13.0f,{0.0f,0.0f,0.0f,profile.dustLaneStrength*(0.10f+0.02f*i)},44);
    }

    const int counts[3]={profile.ultraDistantStars,profile.distantStars,profile.localStars};
    const float parallaxes[3]={0.035f,0.32f,1.75f};
    const float alphas[3]={0.44f,0.62f,0.80f};
    const float sizes[3]={1.0f,1.35f,2.0f};
    for(int layer=0;layer<3;++layer){
        glPointSize(sizes[layer]);
        glBegin(GL_POINTS);
        for(int i=0;i<counts[layer];++i){
            uint32_t seed=static_cast<uint32_t>((i+1+layer*997)*747796405u+2891336453u);
            seed^=seed>>16; seed*=2246822519u; seed^=seed>>13;
            const float fx=static_cast<float>(seed&0xffffu)/65535.0f;
            const float fy=static_cast<float>((seed>>16)&0xffffu)/65535.0f;
            float x=fx*w-c.x*parallaxes[layer];
            float y=fy*h+c.y*parallaxes[layer]*0.52f;
            x=std::fmod(x,w);if(x<0)x+=w;y=std::fmod(y,h);if(y<0)y+=h;
            const float temperature=static_cast<float>((seed>>7)&0xffu)/255.0f;
            const float b=(0.52f+temperature*0.42f)*profile.exposure;
            const float warm=profile.warmBias;
            const float twinkle=1.0f-profile.starTwinkle*.5f + profile.starTwinkle*.5f*std::sin(frame.elapsedSeconds*(0.55f+temperature*1.7f)+float(seed&31u));
            glColor4f(std::clamp(b*(0.82f+warm*0.30f)*twinkle,0.0f,1.0f),
                      std::clamp(b*(0.90f+warm*0.04f)*twinkle,0.0f,1.0f),
                      std::clamp(b*(1.00f-warm*0.25f)*twinkle,0.0f,1.0f),alphas[layer]);
            glVertex2f(x,y);
        }
        glEnd();
    }

    // Sparse bright anchor stars give the backdrop depth at a glance without
    // creating navigationally meaningful fake contacts.
    for(int i=0;i<14;++i){
        std::uint32_t seed=static_cast<std::uint32_t>((i+77)*2654435761u);
        const float fx=float(seed&0xffffu)/65535.0f,fy=float((seed>>16)&0xffffu)/65535.0f;
        float x=std::fmod(fx*w-c.x*.11f,w);if(x<0)x+=w;
        float y=std::fmod(fy*h+c.y*.08f,h);if(y<0)y+=h;
        const float pulse=.72f+.28f*std::sin(frame.elapsedSeconds*(.35f+.04f*i)+i);
        FilledCircle(x,y,0,1.2f+(i%3)*.45f,{.70f+.20f*profile.warmBias,.82f,.98f-.16f*profile.warmBias,.22f+.18f*pulse},12);
    }

    // Very sparse local particulate provides near-field parallax in dusty or
    // industrial regions without reading as snow flying through the cockpit.
    glPointSize(1.6f);glBegin(GL_POINTS);
    for(int i=0;i<36;++i){
        const float x=std::fmod(i*191.0f-c.x*4.8f,w);
        const float y=std::fmod(i*113.0f+c.y*3.2f,h);
        Color({0.42f,0.48f,0.50f,profile.localDust*0.72f});
        glVertex2f(x<0?x+w:x,y<0?y+h:y);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void DrawVectorTravelBackdrop(const NativeBattlefieldFrame& frame) {
    const auto& v=frame.vectorVisual;
    if(v.tunnelOpacity<=0.001f&&v.starStretch<=0.001f)return;
    DisableShader();SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight);
    const float baseCx=w*0.5f,baseCy=h*0.46f;
    const float time=frame.elapsedSeconds;
    const float flow=std::max(0.05f,v.tunnelFlow);
    const float minDim=std::min(w,h);

    // Animated radial slipstream. Rings continuously move toward the viewer,
    // twist, and pulse instead of displaying a static tunnel image.
    if(v.tunnelOpacity>0.01f){
        FilledRect(0,0,0,w,h,{0.001f,0.004f,0.010f,0.48f*v.tunnelOpacity});
        for(int ring=0;ring<18;++ring){
            const float phase=std::fmod(static_cast<float>(ring)/18.0f + time*0.22f*flow,1.0f);
            const float eased=phase*phase;
            const float radius=minDim*(0.055f+eased*0.68f);
            const float twist=time*(0.38f+v.tunnelTwist*0.74f)+ring*0.47f;
            const float cx=baseCx+std::sin(twist)*minDim*(0.008f+0.018f*v.tunnelTwist)*eased;
            const float cy=baseCy+std::cos(twist*0.73f)*minDim*(0.006f+0.012f*v.tunnelTwist)*eased;
            const float alpha=(0.08f+0.22f*(1.0f-phase))*v.tunnelOpacity;
            Ring(cx,cy,0,radius,{0.08f+0.08f*(1.0f-phase),0.30f+0.22f*(1.0f-phase),0.42f+0.34f*(1.0f-phase),alpha},
                 1.0f+2.0f*(1.0f-phase),80);
        }

        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        glBegin(GL_LINES);
        for(int i=0;i<220;++i){
            uint32_t hsh=static_cast<uint32_t>((i+113)*2654435761u);hsh^=hsh>>15;hsh*=2246822519u;hsh^=hsh>>13;
            const float base=static_cast<float>(hsh&0xffffu)/65535.0f;
            const float a=static_cast<float>((hsh>>16)&0xffffu)/65535.0f*2.0f*kPi + time*0.13f*v.tunnelTwist;
            const float travel=std::fmod(base+time*(0.34f+0.42f*flow),1.0f);
            const float r0=minDim*(0.07f+travel*travel*0.58f);
            const float len=(12.0f+travel*95.0f)*(0.40f+0.60f*v.starStretch);
            const float x0=baseCx+std::cos(a)*r0,y0=baseCy+std::sin(a)*r0;
            const float x1=baseCx+std::cos(a)*(r0+len),y1=baseCy+std::sin(a)*(r0+len);
            Color({0.38f,0.72f,0.96f,(0.08f+0.42f*travel)*v.tunnelOpacity});
            glVertex2f(x0,y0);glVertex2f(x1,y1);
        }
        glEnd();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    }
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

void DrawVectorTravelForeground(const NativeBattlefieldFrame& frame) {
    const auto& v=frame.vectorVisual;
    if(v.distortion<=0.001f&&v.foregroundStreaks<=0.001f&&v.entryFlash<=0.001f&&v.exitReveal<=0.001f)return;
    DisableShader();SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight),cx=w*0.5f,cy=h*0.46f;
    const float time=frame.elapsedSeconds,minDim=std::min(w,h);

    if(v.foregroundStreaks>0.01f){
        glBegin(GL_LINES);
        for(int i=0;i<68;++i){
            uint32_t hsh=static_cast<uint32_t>((i+271)*747796405u+2891336453u);hsh^=hsh>>16;
            const float a=static_cast<float>(hsh&0xffffu)/65535.0f*2.0f*kPi-time*0.17f;
            const float lane=std::fmod(static_cast<float>((hsh>>16)&0xffu)/255.0f+time*(0.55f+v.tunnelFlow*0.65f),1.0f);
            const float r=minDim*(0.12f+lane*0.58f);
            const float len=24.0f+86.0f*lane;
            Color({0.62f,0.88f,1.0f,(0.05f+0.22f*lane)*v.foregroundStreaks});
            glVertex2f(cx+std::cos(a)*r,cy+std::sin(a)*r);
            glVertex2f(cx+std::cos(a)*(r+len),cy+std::sin(a)*(r+len));
        }
        glEnd();
    }

    const float pulse=0.82f+0.18f*std::sin(time*7.0f);
    if(v.shipEnvelope>0.01f){
        Ring(cx,cy+minDim*0.075f,0,minDim*(0.085f+0.008f*pulse),
             {0.22f,0.72f,1.0f,0.28f*v.shipEnvelope},2.2f,72);
        Ring(cx,cy+minDim*0.075f,0,minDim*(0.105f+0.010f*pulse),
             {0.12f,0.42f,0.82f,0.14f*v.shipEnvelope},1.2f,72);
    }
    if(v.distortion>0.02f)Ring(cx,cy,0,minDim*(0.20f+v.distortion*0.18f),{0.28f,0.70f,0.96f,0.18f*v.distortion},1.8f,80);
    if(v.entryFlash>0.50f){
        const float flash=(v.entryFlash-0.50f)*2.0f;
        FilledRect(0,0,0,w,h,{0.36f,0.72f,0.96f,0.10f*flash});
    }
    if(v.exitReveal>0.65f){
        const float flash=(v.exitReveal-0.65f)/0.35f;
        Ring(cx,cy,0,minDim*(0.12f+flash*0.54f),{0.68f,0.92f,1.0f,0.22f*(1.0f-flash)},3.0f,88);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

void DrawTacticalPlane(const NativeBattlefieldFrame& frame) {
    if (!frame.playerPhysics) return;
    DisableSceneLighting();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const auto& p=frame.playerPhysics->position;
    for(float radius : {5.0f,10.0f,18.0f})
        Ring(p.x,p.y,-0.06f,radius,{0.10f,0.42f,0.52f,radius<=10.0f?0.085f:0.040f},0.8f,64);
    for(int i=0;i<8;++i){
        const float a=static_cast<float>(i)/8.0f*2.0f*kPi;
        Line(p.x+std::cos(a)*4.5f,p.y+std::sin(a)*4.5f,-0.065f,
             p.x+std::cos(a)*18.0f,p.y+std::sin(a)*18.0f,-0.065f,
             {0.08f,0.34f,0.42f,0.030f},0.8f);
    }
    glDisable(GL_BLEND);
    SetupSceneLighting();
}

Vector3 RemapObjVertex(const Vector3& v) {
    // Module library convention: OBJ +Z is ship-forward, +Y is model-up.
    // Runtime convention: gameplay +Y is ship-forward, visual +Z is height.
    return {v.x,v.z,v.y};
}

Vector3 Cross3(const Vector3& a,const Vector3& b){
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}

Vector3 RotateX3(const Vector3& v,float radians){const float c=std::cos(radians),s=std::sin(radians);return {v.x,v.y*c-v.z*s,v.y*s+v.z*c};}
Vector3 RotateY3(const Vector3& v,float radians){const float c=std::cos(radians),s=std::sin(radians);return {v.x*c+v.z*s,v.y,-v.x*s+v.z*c};}
Vector3 RotateZ3(const Vector3& v,float radians){const float c=std::cos(radians),s=std::sin(radians);return {v.x*c-v.y*s,v.x*s+v.y*c,v.z};}

Vector3 TransformPlacementPoint(const VisualModulePlacement& p,Vector3 v){
    v.x*=p.mirrorX?-p.scaleX:p.scaleX;v.y*=p.mirrorY?-p.scaleY:p.scaleY;v.z*=p.mirrorZ?-p.scaleZ:p.scaleZ;
    if(std::fabs(p.rollDegrees)>.001f)v=RotateY3(v,p.rollDegrees*kPi/180.0f);
    if(std::fabs(p.pitchDegrees)>.001f)v=RotateX3(v,p.pitchDegrees*kPi/180.0f);
    if(std::fabs(p.yawDegrees)>.001f)v=RotateZ3(v,p.yawDegrees*kPi/180.0f);
    return v+Vector3{p.x,p.y,p.z};
}

struct SourceLocalBounds{Vector3 min{1.0e30f,1.0e30f,1.0e30f};Vector3 max{-1.0e30f,-1.0e30f,-1.0e30f};bool valid=false;};
SourceLocalBounds CachedSourceLocalBounds(const ObjMeshData& mesh){
    static std::unordered_map<const ObjMeshData*,SourceLocalBounds> cache;const auto found=cache.find(&mesh);if(found!=cache.end())return found->second;if(cache.size()>1024)cache.clear();SourceLocalBounds out;for(const auto& source:mesh.positions){const auto v=RemapObjVertex(source);out.min.x=std::min(out.min.x,v.x);out.min.y=std::min(out.min.y,v.y);out.min.z=std::min(out.min.z,v.z);out.max.x=std::max(out.max.x,v.x);out.max.y=std::max(out.max.y,v.y);out.max.z=std::max(out.max.z,v.z);out.valid=true;}cache.emplace(&mesh,out);return out;
}

struct RecipeLocalBounds{Vector3 min{1.0e30f,1.0e30f,1.0e30f};Vector3 max{-1.0e30f,-1.0e30f,-1.0e30f};bool valid=false;};
RecipeLocalBounds BuildRecipeLocalBounds(const NativeBattlefieldRenderer::VisualAssets& assets,const ProceduralShipVisualRecipe& recipe){
    RecipeLocalBounds out;
    for(const auto& placement:recipe.modules){
        const auto it=assets.shipModules.find(placement.moduleId);if(it==assets.shipModules.end()||it->second.positions.empty())continue;
        const auto sourceBounds=CachedSourceLocalBounds(it->second);if(!sourceBounds.valid)continue;const auto mn=sourceBounds.min,mx=sourceBounds.max;
        const Vector3 corners[8]={{mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},{mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}};
        for(const auto& c:corners){const auto v=TransformPlacementPoint(placement,c);out.min.x=std::min(out.min.x,v.x);out.min.y=std::min(out.min.y,v.y);out.min.z=std::min(out.min.z,v.z);out.max.x=std::max(out.max.x,v.x);out.max.y=std::max(out.max.y,v.y);out.max.z=std::max(out.max.z,v.z);out.valid=true;}
    }
    return out;
}

std::size_t ShieldRecipeHash(const ProceduralShipVisualRecipe& recipe){
    std::size_t h=std::hash<std::string>{}(recipe.recipeId)^std::hash<std::uint32_t>{}(recipe.seed);
    auto mix=[&](std::size_t v){h^=v+0x9e3779b97f4a7c15ull+(h<<6)+(h>>2);};
    auto hf=[&](float v){mix(std::hash<float>{}(v));};
    for(const auto& p:recipe.modules){mix(std::hash<std::string>{}(p.moduleId));hf(p.x);hf(p.y);hf(p.z);hf(p.scaleX);hf(p.scaleY);hf(p.scaleZ);hf(p.yawDegrees);hf(p.pitchDegrees);hf(p.rollDegrees);mix(p.mirrorX?1:0);mix(p.mirrorY?2:0);mix(p.mirrorZ?4:0);}
    return h;
}

struct ShieldLocalTriangle{Vector3 a,b,c,n,center;float phase=0.0f;};
const std::vector<ShieldLocalTriangle>& CachedShieldTriangles(const NativeBattlefieldRenderer::VisualAssets& assets,const ProceduralShipVisualRecipe& recipe){
    static std::unordered_map<std::size_t,std::vector<ShieldLocalTriangle>> cache;
    const auto key=ShieldRecipeHash(recipe);auto found=cache.find(key);if(found!=cache.end())return found->second;
    if(cache.size()>12)cache.clear();
    std::vector<ShieldLocalTriangle> built;
    std::size_t triangleCount=0;for(const auto& p:recipe.modules){const auto it=assets.shipModules.find(p.moduleId);if(it!=assets.shipModules.end())triangleCount+=it->second.triangles.size();}
    built.reserve(triangleCount);
    for(const auto& p:recipe.modules){
        const auto it=assets.shipModules.find(p.moduleId);if(it==assets.shipModules.end())continue;const auto& mesh=it->second;
        const auto sourceBounds=CachedSourceLocalBounds(mesh);const Vector3 sourceCenter=sourceBounds.valid?(sourceBounds.min+sourceBounds.max)*.5f:Vector3{};const Vector3 moduleCenter=TransformPlacementPoint(p,sourceCenter);
        for(const auto& tri:mesh.triangles){
            if(tri.position[0]<0||tri.position[1]<0||tri.position[2]<0||tri.position[0]>=static_cast<int>(mesh.positions.size())||tri.position[1]>=static_cast<int>(mesh.positions.size())||tri.position[2]>=static_cast<int>(mesh.positions.size()))continue;
            const Vector3 a=TransformPlacementPoint(p,RemapObjVertex(mesh.positions[tri.position[0]]));
            const Vector3 b=TransformPlacementPoint(p,RemapObjVertex(mesh.positions[tri.position[1]]));
            const Vector3 c=TransformPlacementPoint(p,RemapObjVertex(mesh.positions[tri.position[2]]));
            Vector3 n=Cross3(b-a,c-a).normalized();if(n.length()<=1.0e-6f)continue;
            const Vector3 center=(a+b+c)*(1.0f/3.0f);if((n.x*(center.x-moduleCenter.x)+n.y*(center.y-moduleCenter.y)+n.z*(center.z-moduleCenter.z))<0.0f)n=n*-1.0f;
            built.push_back({a,b,c,n,center,center.x*.73f+center.y*.41f+center.z*.57f});
        }
    }
    return cache.emplace(key,std::move(built)).first->second;
}

struct ObjMaterialPresentation { Rgba color; float emission=0.0f; float shininess=48.0f; SpaceMaterialKind kind=SpaceMaterialKind::ShipHull; float metallic=-1.0f; float roughness=-1.0f; };

ObjMaterialPresentation ShipyardSourceMaterial(const std::string& name,const Rgba& inheritedPaint,float alpha,SpaceMaterialKind fallbackKind,const ShipAppearanceState* appearance){
    const auto paintColor=[&](const ShipPaintLayer& p){return Rgba{p.r,p.g,p.b,p.a*alpha};};
    const auto zone=ShipyardPaintZoneSystem::ForMaterial(name);
    Rgba zonePaint=inheritedPaint;zonePaint.a*=alpha;
    if(appearance){
        if(zone==ShipyardPaintZone::Primary)zonePaint=paintColor(appearance->primary);
        else if(zone==ShipyardPaintZone::Secondary)zonePaint=paintColor(appearance->secondary);
        else if(zone==ShipyardPaintZone::Trim)zonePaint=paintColor(appearance->trim);

        // Generic Greyoxide engine materials are named Flat/Flat.001 just like
        // hull plates, so blindly painting them as ordinary hull leaves aft
        // propulsion looking like pale untextured clay.  Propulsion uses the
        // same authored faces but receives a machinery-specific finish: the
        // main Flat zone follows secondary paint while retaining a metallic
        // engine treatment; Flat.001 becomes the darker mechanical zone.
        if(fallbackKind==SpaceMaterialKind::EngineHousing && zone==ShipyardPaintZone::Primary)
            zonePaint=paintColor(appearance->secondary);
    }
    auto scaled=[&](const Rgba& base,float k){return Rgba{std::clamp(base.r*k,0.0f,1.0f),std::clamp(base.g*k,0.0f,1.0f),std::clamp(base.b*k,0.0f,1.0f),base.a};};
    switch(zone){
    case ShipyardPaintZone::Primary:
        if(fallbackKind==SpaceMaterialKind::EngineHousing)return {scaled(zonePaint,.72f),0.0f,76.0f,SpaceMaterialKind::EngineHousing,.52f,.32f};
        if(fallbackKind==SpaceMaterialKind::ThrusterCore)return {{.10f,.13f,.16f,zonePaint.a},.34f,82.0f,SpaceMaterialKind::ThrusterCore,.62f,.24f};
        return {scaled(zonePaint,1.0f),0.0f,52.0f,fallbackKind};
    case ShipyardPaintZone::Secondary:{
        auto c=scaled(zonePaint,fallbackKind==SpaceMaterialKind::EngineHousing?.56f:.92f);c.b=std::min(1.0f,c.b+.035f);
        return {c,0.0f,fallbackKind==SpaceMaterialKind::EngineHousing?72.0f:42.0f,
                fallbackKind==SpaceMaterialKind::EngineHousing?SpaceMaterialKind::EngineHousing:SpaceMaterialKind::StructuralMetal,
                fallbackKind==SpaceMaterialKind::EngineHousing?.66f:-1.0f,fallbackKind==SpaceMaterialKind::EngineHousing?.28f:-1.0f};}
    case ShipyardPaintZone::Trim:return {scaled(zonePaint,1.0f),0.0f,48.0f,SpaceMaterialKind::StructuralMetal};
    case ShipyardPaintZone::StructuralDark:return {{.055f,.065f,.075f,zonePaint.a},0.0f,30.0f,SpaceMaterialKind::StructuralMetal};
    case ShipyardPaintZone::LightMetal:return {{.40f,.44f,.49f,zonePaint.a},0.0f,70.0f,SpaceMaterialKind::StructuralMetal};
    case ShipyardPaintZone::DarkMetal:return {{.14f,.16f,.19f,zonePaint.a},0.0f,68.0f,SpaceMaterialKind::StructuralMetal};
    case ShipyardPaintZone::EmissiveWhite:return {{.82f,.91f,1.0f,zonePaint.a},.48f,32.0f,SpaceMaterialKind::ThrusterCore};
    case ShipyardPaintZone::EmissiveBlue:return {{.12f,.55f,1.0f,zonePaint.a},.72f,28.0f,SpaceMaterialKind::ThrusterCore};
    case ShipyardPaintZone::EmissiveOrange:return {{1.0f,.30f,.055f,zonePaint.a},.82f,28.0f,SpaceMaterialKind::ThrusterCore};
    case ShipyardPaintZone::Glass:return {{.08f,.23f,.34f,.50f*alpha},.10f,82.0f,SpaceMaterialKind::Canopy};
    case ShipyardPaintZone::Inherited:break;
    }
    return {zonePaint,0.0f,48.0f,fallbackKind};
}
void DrawObjMesh(const NativeBattlefieldRenderer::VisualAssets& assets,const ObjMeshData& mesh,const Rgba& color,float alpha=1.0f,float emission=0.0f,SpaceMaterialKind kind=SpaceMaterialKind::ShipHull,const ShipAppearanceState* appearance=nullptr) {
    auto drawTriangle=[&](const ObjTriangle& tri){
        const Vector3 a=RemapObjVertex(mesh.positions[tri.position[0]]);
        const Vector3 b=RemapObjVertex(mesh.positions[tri.position[1]]);
        const Vector3 cpos=RemapObjVertex(mesh.positions[tri.position[2]]);
        Vector3 face=Cross3(b-a,cpos-a).normalized();if(face.length()<=1.0e-5f)face={0,0,1};
        const Vector3 verts[3]={a,b,cpos};
        for(int i=0;i<3;++i){
            Vector3 n=face;
            if(tri.normal[i]>=0&&static_cast<std::size_t>(tri.normal[i])<mesh.normals.size())n=RemapObjVertex(mesh.normals[tri.normal[i]]).normalized();
            glNormal3f(n.x,n.y,n.z);
            if(tri.texcoord[i]>=0&&static_cast<std::size_t>(tri.texcoord[i])<mesh.texcoords.size()){
                const auto& uv=mesh.texcoords[tri.texcoord[i]];glTexCoord2f(uv.u,1.0f-uv.v);
            } else glTexCoord2f(0.0f,0.0f);
            glVertex3f(verts[i].x,verts[i].y,verts[i].z);
        }
    };
    auto materialFor=[&](std::size_t index)->const ObjMaterialData*{return index<mesh.materials.size()?&mesh.materials[index]:nullptr;};
    auto textureFor=[&](const ObjMaterialData* material)->GLuint{
        if(!material||material->baseColorTexturePath.empty())return 0;
        const auto it=assets.shipTextures.find(material->baseColorTexturePath);return it==assets.shipTextures.end()?0:static_cast<GLuint>(it->second);
    };
    auto applySource=[&](const ObjMaterialData* material,ObjMaterialPresentation style,bool paintableZone){
        if(!material||!material->resolvedFromMtl)return style;
        if(material->hasDiffuseColor){
            // Paintable zones are livery-authoritative. Preserve just enough
            // authored Kd variation for panel character instead of allowing
            // Greyoxide's neutral source gray to wash the chosen colors out.
            const float sourceWeight=(appearance&&paintableZone)?.04f:.34f;
            const float paintWeight=1.0f-sourceWeight;
            style.color.r=std::clamp(material->diffuseR*sourceWeight+style.color.r*paintWeight,0.0f,1.0f);
            style.color.g=std::clamp(material->diffuseG*sourceWeight+style.color.g*paintWeight,0.0f,1.0f);
            style.color.b=std::clamp(material->diffuseB*sourceWeight+style.color.b*paintWeight,0.0f,1.0f);
        }
        style.color.a*=material->alpha;
        style.shininess=material->shininess;
        style.metallic=material->metallic;style.roughness=material->roughness;
        style.emission=std::max(style.emission,std::max({material->emissiveR,material->emissiveG,material->emissiveB}));
        return style;
    };
    if(mesh.materialNames.empty()){
        SetShipBaseTexture(0);Rgba c=color;c.a*=alpha;SetMaterial(c,48.0f,emission,kind);glBegin(GL_TRIANGLES);for(const auto& tri:mesh.triangles)drawTriangle(tri);glEnd();return;
    }
    for(std::size_t mi=0;mi<mesh.materialNames.size();++mi){
        const auto* sourceMaterial=materialFor(mi);
        const auto zone=ShipyardPaintZoneSystem::ForMaterial(mesh.materialNames[mi]);
        auto style=applySource(sourceMaterial,ShipyardSourceMaterial(mesh.materialNames[mi],color,alpha,kind,appearance),ShipyardPaintZoneSystem::IsPaintable(zone));
        SetMaterial(style.color,style.shininess,std::max(emission,style.emission),style.kind,style.metallic,style.roughness);
        SetShipBaseTexture(textureFor(sourceMaterial));
        const bool transparent=style.color.a<.99f;if(transparent){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);}
        glBegin(GL_TRIANGLES);for(const auto& tri:mesh.triangles)if(tri.materialIndex==static_cast<int>(mi))drawTriangle(tri);glEnd();
        if(transparent)glDisable(GL_BLEND);SetShipBaseTexture(0);
    }
    bool hasUnassigned=false;for(const auto& tri:mesh.triangles)if(tri.materialIndex<0){hasUnassigned=true;break;}
    if(hasUnassigned){SetShipBaseTexture(0);Rgba c=color;c.a*=alpha;SetMaterial(c,48.0f,emission,kind);glBegin(GL_TRIANGLES);for(const auto& tri:mesh.triangles)if(tri.materialIndex<0)drawTriangle(tri);glEnd();}
}

bool DrawModule(const NativeBattlefieldRenderer::VisualAssets& assets,const char* name,
                float x,float y,float z,float scale,const Rgba& color,float alpha=1.0f,
                SpaceMaterialKind kind=SpaceMaterialKind::ShipHull) {
    const auto it=assets.shipModules.find(name);
    if(it==assets.shipModules.end())return false;
    glPushMatrix();
    glTranslatef(x,y,z); glScalef(scale,scale,scale);
    DrawObjMesh(assets,it->second,color,alpha,0.0f,kind);
    glPopMatrix();
    return true;
}

bool DrawModulePlacement(const NativeBattlefieldRenderer::VisualAssets& assets,
                         const VisualModulePlacement& placement,
                         const Rgba& color,float alpha=1.0f,const ShipAppearanceState* appearance=nullptr) {
    const auto it=assets.shipModules.find(placement.moduleId);
    if(it==assets.shipModules.end())return false;
    glPushMatrix();
    glTranslatef(placement.x,placement.y,placement.z);
    glRotatef(placement.yawDegrees,0,0,1);
    if(std::abs(placement.pitchDegrees)>.01f) glRotatef(placement.pitchDegrees,1,0,0);
    if(std::abs(placement.rollDegrees)>.01f) glRotatef(placement.rollDegrees,0,1,0);
    glScalef(placement.mirrorX?-placement.scaleX:placement.scaleX,
             placement.mirrorY?-placement.scaleY:placement.scaleY,
             placement.mirrorZ?-placement.scaleZ:placement.scaleZ);
    DrawObjMesh(assets,it->second,color,alpha,placement.material==SpaceMaterialKind::ThrusterCore?0.18f:0.0f,placement.material,appearance);
    glPopMatrix();
    return true;
}

void DrawShipyardSelectionOverlay(const NativeBattlefieldRenderer::VisualAssets& assets,
                                  const VisualModulePlacement& placement,
                                  const ShipyardModuleRecord* authoringRecord=nullptr,
                                  int selectedSocketIndex=-1,
                                  bool socketEdit=false) {
    const auto it=assets.shipModules.find(placement.moduleId);
    if(it==assets.shipModules.end()||it->second.positions.empty())return;

    Vector3 mn{1.0e9f,1.0e9f,1.0e9f},mx{-1.0e9f,-1.0e9f,-1.0e9f};
    for(const auto& sourceVertex:it->second.positions){
        const Vector3 v=RemapObjVertex(sourceVertex);
        mn.x=std::min(mn.x,v.x);mn.y=std::min(mn.y,v.y);mn.z=std::min(mn.z,v.z);
        mx.x=std::max(mx.x,v.x);mx.y=std::max(mx.y,v.y);mx.z=std::max(mx.z,v.z);
    }

    glPushMatrix();
    glTranslatef(placement.x,placement.y,placement.z);
    glRotatef(placement.yawDegrees,0,0,1);
    if(std::abs(placement.pitchDegrees)>.01f)glRotatef(placement.pitchDegrees,1,0,0);
    if(std::abs(placement.rollDegrees)>.01f)glRotatef(placement.rollDegrees,0,1,0);
    glScalef(placement.mirrorX?-placement.scaleX:placement.scaleX,
             placement.mirrorY?-placement.scaleY:placement.scaleY,
             placement.mirrorZ?-placement.scaleZ:placement.scaleZ);

    DisableShader();glDisable(GL_TEXTURE_2D);glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);glLineWidth(2.4f);glColor4f(.10f,.92f,1.0f,.96f);
    const Vector3 c[8]={{mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},
                        {mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}};
    const int e[12][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    glBegin(GL_LINES);for(const auto& edge:e){glVertex3f(c[edge[0]].x,c[edge[0]].y,c[edge[0]].z);glVertex3f(c[edge[1]].x,c[edge[1]].y,c[edge[1]].z);}glEnd();

    const Vector3 center{(mn.x+mx.x)*.5f,(mn.y+mx.y)*.5f,(mn.z+mx.z)*.5f};
    const float axisLen=std::max({mx.x-mn.x,mx.y-mn.y,mx.z-mn.z})*.22f;
    glLineWidth(3.0f);glBegin(GL_LINES);
    glColor4f(.92f,.28f,.18f,.95f);glVertex3f(center.x,center.y,center.z);glVertex3f(center.x+axisLen,center.y,center.z);
    glColor4f(.18f,.92f,.42f,.95f);glVertex3f(center.x,center.y,center.z);glVertex3f(center.x,center.y+axisLen,center.z);
    glColor4f(.18f,.58f,1.0f,.95f);glVertex3f(center.x,center.y,center.z);glVertex3f(center.x,center.y,center.z+axisLen);
    glEnd();

    // Selected-part attachment authority diagnostic.  The red root marker is
    // intentionally rendered in the module's local frame, after the same
    // placement transform as the mesh.  This makes a bad source/root choice
    // visually obvious instead of forcing authors to infer it from the final
    // assembly.  For WING modules this is the face that MUST mate to the hull.
    const ShipyardModuleRecord* record=authoringRecord;
    if(!record){for(const auto& candidate:assets.shipyardCatalog)
        if(candidate.source.moduleId==placement.moduleId){record=&candidate;break;}}
    if(record){
        if(socketEdit){
            const float marker=std::max(.055f,axisLen*.22f);
            glDisable(GL_DEPTH_TEST);
            for(std::size_t si=0;si<record->sockets.size();++si){
                const auto& socket=record->sockets[si];const bool selected=selectedSocketIndex>=0&&si==static_cast<std::size_t>(selectedSocketIndex);
                const bool exhaust=socket.type=="exhaust";
                if(selected)glColor4f(1.0f,.82f,.16f,.99f);
                else if(exhaust)glColor4f(1.0f,.30f,.12f,.92f);
                else glColor4f(.12f,.86f,1.0f,.82f);
                glLineWidth(selected?4.0f:2.0f);glBegin(GL_LINES);
                glVertex3f(socket.x,socket.y,socket.z);
                glVertex3f(socket.x+socket.dirX*marker*3.0f,socket.y+socket.dirY*marker*3.0f,socket.z+socket.dirZ*marker*3.0f);
                glEnd();
                glPointSize(selected?11.0f:6.0f);glBegin(GL_POINTS);glVertex3f(socket.x,socket.y,socket.z);glEnd();
            }
            glPointSize(1.0f);
        }
        const auto mount=std::find_if(record->sockets.begin(),record->sockets.end(),[](const ShipyardAssemblySocket& s){return s.name=="mount";});
        if(mount!=record->sockets.end()){
            const float marker=std::max(.08f,axisLen*.34f);
            glDisable(GL_DEPTH_TEST);glLineWidth(4.0f);glColor4f(1.0f,.10f,.08f,.98f);
            glBegin(GL_LINES);
            glVertex3f(mount->x-marker,mount->y,mount->z);glVertex3f(mount->x+marker,mount->y,mount->z);
            glVertex3f(mount->x,mount->y-marker,mount->z);glVertex3f(mount->x,mount->y+marker,mount->z);
            glVertex3f(mount->x,mount->y,mount->z-marker);glVertex3f(mount->x,mount->y,mount->z+marker);
            glVertex3f(mount->x,mount->y,mount->z);
            glVertex3f(mount->x+mount->dirX*marker*2.5f,mount->y+mount->dirY*marker*2.5f,mount->z+mount->dirZ*marker*2.5f);
            glEnd();
            glPointSize(8.0f);glBegin(GL_POINTS);glVertex3f(mount->x,mount->y,mount->z);glEnd();glPointSize(1.0f);
        }
    }

    glLineWidth(1.0f);glEnable(GL_DEPTH_TEST);glDisable(GL_BLEND);SetupSceneLighting();
    glPopMatrix();
}

void DrawChamferedPlate(float cx,float cy,float cz,float sx,float sy,float sz,float chamfer,
                        const Rgba& color,SpaceMaterialKind kind) {
    const float hx=sx*.5f,hy=sy*.5f,hz=sz*.5f;
    const float c=std::min(std::max(0.02f,chamfer),std::min(hx,hy)*.45f);
    const float vx[8]={-hx+c,hx-c,hx,hx,hx-c,-hx+c,-hx,-hx};
    const float vy[8]={-hy,-hy,-hy+c,hy-c,hy,hy,hy-c,-hy+c};
    SetMaterial(color,54.0f,0.0f,kind);
    glBegin(GL_POLYGON); glNormal3f(0,0,1); for(int i=0;i<8;++i)glVertex3f(cx+vx[i],cy+vy[i],cz+hz); glEnd();
    glBegin(GL_POLYGON); glNormal3f(0,0,-1); for(int i=7;i>=0;--i)glVertex3f(cx+vx[i],cy+vy[i],cz-hz); glEnd();
    glBegin(GL_QUADS);
    for(int i=0;i<8;++i){const int j=(i+1)%8; const float dx=vx[j]-vx[i],dy=vy[j]-vy[i];
        const float len=std::sqrt(dx*dx+dy*dy); glNormal3f(len>1e-5f?dy/len:0.0f,len>1e-5f?-dx/len:0.0f,0);
        glVertex3f(cx+vx[i],cy+vy[i],cz-hz); glVertex3f(cx+vx[j],cy+vy[j],cz-hz);
        glVertex3f(cx+vx[j],cy+vy[j],cz+hz); glVertex3f(cx+vx[i],cy+vy[i],cz+hz); }
    glEnd();
}

Rgba ManufacturerAccent(const ProceduralShipVisualRecipe* recipe,const Rgba& fallback,float alpha){
    if(!recipe)return fallback;
    const std::string& family=recipe->manufacturerFamily;
    if(family=="HELIX") return {0.20f,0.72f,0.84f,alpha};
    if(family=="IRONWORKS") return {0.88f,0.50f,0.16f,alpha};
    if(family=="ORBITAL FORGE") return {0.76f,0.66f,0.24f,alpha};
    if(family=="NOMAD") return {0.62f,0.52f,0.32f,alpha};
    if(family=="IRON DOMINION") return {0.82f,0.28f,0.16f,alpha};
    if(family=="VANGUARD") return {0.66f,0.24f,0.20f,alpha};
    return fallback;
}

Rgba DetailColor(const VisualDetailPlacement& d,const Rgba& hull,const Rgba& dark,const Rgba& machinery,const Rgba& accent,float alpha){
    switch(d.kind){
        case VisualDetailKind::ArmorPlate: return {std::min(1.0f,hull.r*1.08f),std::min(1.0f,hull.g*1.08f),std::min(1.0f,hull.b*1.08f),alpha};
        case VisualDetailKind::Fairing: return hull;
        case VisualDetailKind::StructuralRib: case VisualDetailKind::Conduit: case VisualDetailKind::HardpointBase: case VisualDetailKind::MountBridge: case VisualDetailKind::StructuralFill: return machinery;
        case VisualDetailKind::TurretSocket: return accent;
        case VisualDetailKind::Vent: case VisualDetailKind::Radiator: case VisualDetailKind::HeatShield: return dark;
        case VisualDetailKind::DecalStripe: return accent;
        case VisualDetailKind::NavigationLight: return d.x<0.0f?Rgba{0.95f,0.16f,0.10f,alpha}:Rgba{0.18f,0.95f,0.42f,alpha};
        case VisualDetailKind::MaintenanceHatch: return {hull.r*.72f,hull.g*.75f,hull.b*.78f,alpha};
    }
    return hull;
}

void DrawVisualDetail(const VisualDetailPlacement& d,const Rgba& color,float alpha){
    glPushMatrix();
    glTranslatef(d.x,d.y,d.z); glRotatef(d.yawDegrees,0,0,1);
    if(std::abs(d.pitchDegrees)>.01f)glRotatef(d.pitchDegrees,1,0,0);
    if(std::abs(d.rollDegrees)>.01f)glRotatef(d.rollDegrees,0,1,0);
    switch(d.kind){
        case VisualDetailKind::ArmorPlate:
        case VisualDetailKind::Fairing:
        case VisualDetailKind::MaintenanceHatch:
        case VisualDetailKind::HeatShield:
        case VisualDetailKind::DecalStripe:
            DrawChamferedPlate(0,0,0,d.sizeX,d.sizeY,d.sizeZ,std::min(d.sizeX,d.sizeY)*.14f,color,d.material); break;
        case VisualDetailKind::StructuralRib:
        case VisualDetailKind::Conduit:
        case VisualDetailKind::HardpointBase:
        case VisualDetailKind::MountBridge:
        case VisualDetailKind::StructuralFill:
            DrawBox(0,0,0,d.sizeX,d.sizeY,d.sizeZ,color,d.material); break;
        case VisualDetailKind::TurretSocket:
            DrawBox(0,0,0,d.sizeX,d.sizeY,d.sizeZ,color,d.material);
            DrawSphere(0,0,d.sizeZ*.55f,std::max(.09f,d.sizeX*.23f),color,12,6,d.material); break;
        case VisualDetailKind::Vent: {
            const int count=4; for(int i=0;i<count;++i){const float off=(static_cast<float>(i)-1.5f)*(d.sizeX/(count+1));
                DrawBox(off,0,0,d.sizeX*.10f,d.sizeY,d.sizeZ,color,d.material);} break; }
        case VisualDetailKind::Radiator: {
            DrawChamferedPlate(0,0,0,d.sizeX,d.sizeY,d.sizeZ,.08f,color,d.material);
            for(int i=-2;i<=2;++i)DrawBox(static_cast<float>(i)*d.sizeX*.15f,0,d.sizeZ*.6f,d.sizeX*.035f,d.sizeY*.88f,d.sizeZ*.18f,{color.r*.72f,color.g*.74f,color.b*.78f,alpha},SpaceMaterialKind::StructuralMetal);
            break; }
        case VisualDetailKind::NavigationLight:
            DrawSphere(0,0,0,std::max(.045f,d.sizeX*.20f),color,10,5,SpaceMaterialKind::ThrusterCore); break;
    }
    glPopMatrix();
}

const ProceduralShipVisualRecipe* ResolveShipRecipe(const NativeBattlefieldRenderer::VisualAssets& assets,
                                                    bool player,
                                                    const std::string& role,
                                                    std::uint32_t visualSeed,
                                                    const ProceduralShipVisualRecipe* overrideRecipe=nullptr) {
    if(overrideRecipe)return overrideRecipe;
    if (!assets.shipyardReady) return nullptr;
    // R5 hard cutover: every normal runtime ship resolves through Shipyard.
    // Pre-Shipyard procedural/fallback recipes are no longer a visual authority.
    return ProceduralVisualVariantSystem::SelectSourceFamily(assets.generatedVisualCatalog,role,"SHIPYARD_V07_CC0",visualSeed);
}

struct ShipRenderAxisScale { float x=1.0f,y=1.0f,z=1.0f; };
ShipRenderAxisScale ResolveShipAxisScale(float shipScale,const std::string& role,bool player,float screenFraction,
                                         const ProceduralShipVisualRecipe* recipe) {
    const auto p=ForwardSpacePresentationSystem{}.ForShip(role,player,screenFraction);
    return {shipScale*p.widthScale*(recipe?recipe->widthScale:1.0f),
            shipScale*p.lengthScale*(recipe?recipe->lengthScale:1.0f),
            shipScale};
}

float RecipeForwardVisualYawRadians(const ProceduralShipVisualRecipe* recipe) {
    return recipe ? recipe->forwardVisualYawDegrees * kPi / 180.0f : 0.0f;
}

void DrawThrusterPlume(float x,float y,float z,float dx,float dy,float length,float width,float activity,float phase);
Vector3 TransformShipLocalPose(float x,float y,float yaw,const Vector3& local,const ShipRenderAxisScale& axis);
Vector3 TransformShipLocal(const PhysicsComponent& player,const Vector3& local,const ShipRenderAxisScale& axis);
float DirectionalAxisScale(const Vector3& direction,const ShipRenderAxisScale& axis);
Vector3 RotateShipDirection(const PhysicsComponent& player,const Vector3& local);

float AuthoredPropulsionActivity(ShipyardModuleSemantic semantic,const InputState& input,float vectorActivity) {
    if(semantic==ShipyardModuleSemantic::MainEngine||semantic==ShipyardModuleSemantic::EngineNozzle){
        return std::clamp(std::max(input.GetValue(InputAction::ThrustForward),vectorActivity),0.0f,1.0f);
    }
    if(semantic==ShipyardModuleSemantic::RcsThruster){
        const float maneuver=std::max({input.GetValue(InputAction::ThrustReverse),
                                       input.GetValue(InputAction::StrafeLeft),
                                       input.GetValue(InputAction::StrafeRight),
                                       input.GetValue(InputAction::TurnLeft),
                                       input.GetValue(InputAction::TurnRight)});
        return std::clamp(maneuver,0.0f,1.0f);
    }
    return 0.0f;
}

void DrawAuthoredPropulsionEffects(const NativeBattlefieldRenderer::VisualAssets& assets,
                                   const PhysicsComponent& player,const InputState& input,
                                   const ProceduralShipVisualRecipe* recipe,const ShipRenderAxisScale& axis,
                                   float z,float vectorActivity,float time) {
    if(!recipe)return;
    for(const auto& port:ShipyardModuleSystem::BuildPropulsionPorts(assets.shipyardCatalog,*recipe)){
        const float activity=AuthoredPropulsionActivity(port.semantic,input,vectorActivity);
        if(activity<=0.001f)continue;
        const float visualYaw=RecipeForwardVisualYawRadians(recipe);
        const Vector3 world=TransformShipLocalPose(player.position.x,player.position.y,player.rotation.z+visualYaw,port.localPosition,axis);
        const float sy=std::sin(player.rotation.z+visualYaw),cy=std::cos(player.rotation.z+visualYaw);
        const Vector3 dir={port.exhaustDirection.x*cy-port.exhaustDirection.y*sy,
                           port.exhaustDirection.x*sy+port.exhaustDirection.y*cy,
                           port.exhaustDirection.z};
        const Vector3 normalizedDir=dir.normalized();
        DrawThrusterPlume(world.x,world.y,z+world.z,normalizedDir.x,normalizedDir.y,
                          port.plumeLengthHint*DirectionalAxisScale(port.exhaustDirection,axis)*(0.78f+activity*0.62f),
                          port.nozzleRadiusHint*std::min(axis.x,axis.y)*(0.82f+activity*0.24f),activity,time);
    }
}

Rgba ShipRolePaint(const std::string& role,bool player,float alpha){
    const auto r=ToUpperAscii(role);
    if(r.find("MIN")!=std::string::npos)return {0.54f,0.34f,0.10f,alpha};
    if(r.find("HAUL")!=std::string::npos||r.find("INDUSTR")!=std::string::npos)return {0.43f,0.31f,0.18f,alpha};
    if(r.find("EXPLOR")!=std::string::npos)return {0.17f,0.43f,0.47f,alpha};
    if(r.find("COMBAT")!=std::string::npos||r.find("ESCORT")!=std::string::npos)return {0.20f,0.30f,0.48f,alpha};
    return player?Rgba{0.24f,0.38f,0.45f,alpha}:Rgba{0.30f,0.38f,0.43f,alpha};
}

void DrawAppearanceDecals(const NativeBattlefieldRenderer::VisualAssets& assets,
                          const ProceduralShipVisualRecipe& recipe,const ShipAppearanceState& appearance,float alpha){
    if(appearance.decals.empty())return;
    for(const auto& decal:appearance.decals){
        if(decal.moduleIndex>=recipe.modules.size()||decal.opacity<=.01f)continue;
        const auto& placement=recipe.modules[decal.moduleIndex];
        const auto it=assets.shipModules.find(placement.moduleId);if(it==assets.shipModules.end()||it->second.positions.empty())continue;
        Vector3 mn{1.0e9f,1.0e9f,1.0e9f},mx{-1.0e9f,-1.0e9f,-1.0e9f};
        for(const auto& sourceVertex:it->second.positions){const auto v=RemapObjVertex(sourceVertex);mn.x=std::min(mn.x,v.x);mn.y=std::min(mn.y,v.y);mn.z=std::min(mn.z,v.z);mx.x=std::max(mx.x,v.x);mx.y=std::max(mx.y,v.y);mx.z=std::max(mx.z,v.z);}
        const float spanX=std::max(.05f,mx.x-mn.x),spanY=std::max(.05f,mx.y-mn.y);
        const float cx=mn.x+spanX*std::clamp(decal.u,0.05f,.95f),cy=mn.y+spanY*std::clamp(decal.v,0.05f,.95f),z=mx.z+.025f;
        const float size=std::max(.045f,std::min(spanX,spanY)*.20f*std::max(.15f,decal.scale));
        const Rgba c{appearance.trim.r,appearance.trim.g,appearance.trim.b,alpha*decal.opacity};
        glPushMatrix();glTranslatef(placement.x,placement.y,placement.z);glRotatef(placement.yawDegrees,0,0,1);
        if(std::abs(placement.pitchDegrees)>.01f)glRotatef(placement.pitchDegrees,1,0,0);if(std::abs(placement.rollDegrees)>.01f)glRotatef(placement.rollDegrees,0,1,0);
        glScalef(placement.mirrorX?-placement.scaleX:placement.scaleX,
             placement.mirrorY?-placement.scaleY:placement.scaleY,
             placement.mirrorZ?-placement.scaleZ:placement.scaleZ);glTranslatef(cx,cy,z);glRotatef(decal.rotationDegrees,0,0,1);
        DisableShader();glDisable(GL_TEXTURE_2D);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);glColor4f(c.r,c.g,c.b,c.a);
        if(decal.decalAsset=="HAZARD_CHEVRON"){
            glLineWidth(2.2f);glBegin(GL_LINES);for(int i=-1;i<=1;++i){const float y=i*size*.55f;glVertex3f(-size,y-size*.32f,0);glVertex3f(0,y,0);glVertex3f(0,y,0);glVertex3f(size,y-size*.32f,0);}glEnd();glLineWidth(1.0f);
        }else if(decal.decalAsset=="IDENT_BAR"){
            glBegin(GL_QUADS);glVertex3f(-size,-size*.28f,0);glVertex3f(size,-size*.28f,0);glVertex3f(size,size*.28f,0);glVertex3f(-size,size*.28f,0);glEnd();
            glColor4f(.03f,.04f,.05f,c.a);glBegin(GL_QUADS);glVertex3f(-size*.62f,-size*.11f,.001f);glVertex3f(size*.55f,-size*.11f,.001f);glVertex3f(size*.55f,size*.11f,.001f);glVertex3f(-size*.62f,size*.11f,.001f);glEnd();
        }else if(decal.decalAsset=="SURVEY_MARK"){
            glLineWidth(2.0f);glBegin(GL_LINE_LOOP);for(int i=0;i<20;++i){const float a=float(i)/20.0f*2.0f*kPi;glVertex3f(std::cos(a)*size*.62f,std::sin(a)*size*.62f,0);}glEnd();glBegin(GL_LINES);glVertex3f(-size,0,0);glVertex3f(size,0,0);glVertex3f(0,-size,0);glVertex3f(0,size,0);glEnd();glLineWidth(1.0f);
        }else if(decal.decalAsset=="FLEET_HASH"){
            glLineWidth(2.0f);glBegin(GL_LINES);for(int i=-1;i<=1;++i){const float x=i*size*.48f;glVertex3f(x,-size,0);glVertex3f(x,size,0);}glVertex3f(-size,-size*.45f,0);glVertex3f(size,-size*.45f,0);glVertex3f(-size,size*.45f,0);glVertex3f(size,size*.45f,0);glEnd();glLineWidth(1.0f);
        }else{
            glBegin(GL_QUADS);glVertex3f(-size,-size*.18f,0);glVertex3f(size,-size*.18f,0);glVertex3f(size,size*.18f,0);glVertex3f(-size,size*.18f,0);glEnd();
        }
        glDepthMask(GL_TRUE);glDisable(GL_BLEND);SetupSceneLighting();glPopMatrix();
    }
}

void DrawModularShip(const NativeBattlefieldRenderer::VisualAssets& assets,float x,float y,float z,
                     float yaw,float shipScale,bool player,const std::string& role,const Rgba& base,float alpha=1.0f,
                     float screenFraction=0.12f,std::uint32_t visualSeed=1u,bool inspectionOverlay=false,
                     const ProceduralShipVisualRecipe* overrideRecipe=nullptr,int selectedModuleIndex=-1,
                     const ShipAppearanceState* appearance=nullptr,
                     const ShipyardModuleRecord* selectedAuthoringRecord=nullptr,int selectedSocketIndex=-1,bool socketEdit=false,
                     const VisualModulePlacement* dragGhost=nullptr,const VisualModulePlacement* dragMirrorGhost=nullptr) {
    if(!assets.shipyardReady){
        // Fail closed visually. Never resurrect synthetic pre-Shipyard hulls,
        // engines or the old rectangle-mounted thruster presentation.
        return;
    }

    const std::string roleUpper=ToUpperAscii(role);
    const auto shipPresentation=ForwardSpacePresentationSystem{}.ForShip(role,player,screenFraction);
    const bool heavy=player || roleUpper.find("INDUSTR")!=std::string::npos || roleUpper.find("HAUL")!=std::string::npos ||
                     roleUpper.find("MIN")!=std::string::npos || roleUpper.find("SALV")!=std::string::npos || roleUpper.find("CARR")!=std::string::npos;
    const auto* recipe=ResolveShipRecipe(assets,player,role,visualSeed,overrideRecipe);

    const float renderedYaw=yaw+RecipeForwardVisualYawRadians(recipe);
    glPushMatrix();
    glTranslatef(x,y,z); glRotatef(renderedYaw*180.0f/kPi,0,0,1);
    const float recipeWidth=recipe?recipe->widthScale:1.0f;
    const float recipeLength=recipe?recipe->lengthScale:1.0f;
    glScalef(shipScale*shipPresentation.widthScale*recipeWidth,
             shipScale*shipPresentation.lengthScale*recipeLength,shipScale);

    const float accent=recipe?recipe->accentStrength:.35f;
    const auto rolePaint=ShipRolePaint(role,player,alpha);
    const auto paintColor=[&](const ShipPaintLayer& p){return Rgba{p.r,p.g,p.b,alpha*p.a};};
    const Rgba hull=appearance?paintColor(appearance->primary):Rgba{base.r*.30f+rolePaint.r*.70f,base.g*.30f+rolePaint.g*.70f,base.b*.30f+rolePaint.b*.70f,alpha};
    const Rgba dark=appearance?paintColor(appearance->secondary):Rgba{base.r*(.48f+.08f*(1.0f-accent)),base.g*(.51f+.08f*(1.0f-accent)),base.b*(.56f+.08f*(1.0f-accent)),alpha};
    const Rgba trim=appearance?paintColor(appearance->trim):Rgba{0.42f+accent*.18f,0.28f+accent*.12f,0.08f+accent*.08f,alpha};
    const Rgba machinery={dark.r*.62f+.07f,dark.g*.62f+.07f,dark.b*.62f+.08f,alpha};
    const Rgba utility=trim;
    const Rgba canopy=player?Rgba{0.07f,0.48f+accent*.14f,0.66f+accent*.18f,alpha}:Rgba{0.12f,0.27f+accent*.10f,0.34f+accent*.12f,alpha};

    if(recipe && !recipe->modules.empty()) {
        for(std::size_t placementIndex=0;placementIndex<recipe->modules.size();++placementIndex){
            const auto& placement=recipe->modules[placementIndex];
            Rgba color=hull;
            if(placement.material==SpaceMaterialKind::IndustrialHull) color=dark;
            else if(placement.material==SpaceMaterialKind::Canopy) color=canopy;
            else if(placement.material==SpaceMaterialKind::EngineHousing) color=machinery;
            else if(placement.material==SpaceMaterialKind::ThrusterCore) color={0.18f,0.64f,0.92f,alpha};
            if(placement.moduleId=="power_core") color=utility;
            DrawModulePlacement(assets,placement,color,alpha,appearance);
            if(selectedModuleIndex>=0&&placementIndex==static_cast<std::size_t>(selectedModuleIndex))
                DrawShipyardSelectionOverlay(assets,placement,selectedAuthoringRecord,selectedSocketIndex,socketEdit);
        }
        if(dragGhost){
            glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
            DrawModulePlacement(assets,*dragGhost,{.22f,.88f,.92f,.46f},.46f,appearance);
            if(dragMirrorGhost)DrawModulePlacement(assets,*dragMirrorGhost,{.30f,.78f,1.0f,.38f},.38f,appearance);
            glDepthMask(GL_TRUE);glDisable(GL_BLEND);SetupSceneLighting();
        }
    } else {
        // Shipyard-only authority: an unresolved recipe draws no synthetic ship.
    }

    if(recipe&&appearance)DrawAppearanceDecals(assets,*recipe,*appearance,alpha);

    if(recipe && shipPresentation.lod<=1){
        const Rgba accentColor=ManufacturerAccent(recipe,player?Rgba{0.78f,0.48f,0.12f,alpha}:Rgba{0.36f+accent*.34f,0.42f+accent*.20f,0.46f+accent*.12f,alpha},alpha);
        for(const auto& detail:recipe->details){
            const Rgba dc=DetailColor(detail,hull,dark,machinery,accentColor,alpha);
            DrawVisualDetail(detail,dc,alpha);
        }

        // Pass361-400 Refinement R1 inspection QA: while F6 inspection is
        // active, expose the generated structural attachment path directly on
        // the rendered ship. Gold translucent geometry is a mount/pylon path;
        // cyan markers are functional module anchors. This makes a genuinely
        // detached engine/thruster immediately visible from any camera angle.
        if(inspectionOverlay){
            DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            for(const auto& detail:recipe->details)if(detail.kind==VisualDetailKind::MountBridge){
                VisualDetailPlacement qa=detail;qa.sizeX*=1.06f;qa.sizeY*=1.06f;qa.sizeZ*=1.10f;
                DrawVisualDetail(qa,{1.0f,.62f,.10f,.42f},.42f);
            }
            for(const auto& module:recipe->modules){
                const bool functional=module.moduleId.find("engine")!=std::string::npos||module.moduleId.find("thruster")!=std::string::npos||module.moduleId=="cargo_bay"||module.moduleId=="weapon_mount"||module.moduleId=="sensor_array";
                if(functional)DrawSphere(module.x,module.y,module.z+.18f,.075f,{.16f,.90f,1.0f,.78f},8,4,SpaceMaterialKind::ThrusterCore);
            }
            SetupSceneLighting();
        }
    }

    // Close-range overlays remain deterministic presentation detail on top of
    // the generated authored-module recipe, not replacement geometry.
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    if(false && !recipe && shipPresentation.lod<=1){
        const float breakup=shipPresentation.platingBreakup;
        const float panelAlpha=0.10f+0.16f*breakup;
        // Thin longitudinal maintenance/armor seams support the silhouette
        // without painting a giant fixed rectangle across the assembled ship.
        for(int i=-2;i<=2;++i){
            const float py=static_cast<float>(i)*1.44f-.12f;
            FilledRect(-.98f,py,1.66f,1.96f,.032f,{.035f,.047f,.052f,panelAlpha});
        }
        FilledRect(-.18f,-.52f,1.70f,.36f,.68f,{.13f,.17f,.19f,.54f});
        const float nav=.46f*shipPresentation.navigationLights;
        FilledCircle(-1.48f,1.18f,1.78f,.065f,{.95f,.16f,.10f,nav},10);
        FilledCircle( 1.48f,1.18f,1.78f,.065f,{.18f,.95f,.42f,nav},10);
    }
    if(!recipe && shipPresentation.lod==0){
        FilledRect(-0.07f,2.05f,2.02f,0.14f,0.94f,{0.24f,0.29f,0.31f,0.76f});
        FilledCircle(0.0f,3.02f,2.05f,0.16f,{0.34f,0.52f,0.56f,0.74f},14);
    }
    glDisable(GL_BLEND); glEnable(GL_LIGHTING);
    glPopMatrix();
}

void DrawThrusterPlume(float x,float y,float z,float dx,float dy,float length,float width,float activity,float phase) {
    const float sideX=-dy,sideY=dx;
    const float pulse=0.92f+0.08f*std::sin(phase*19.0f+x*3.1f+y*2.7f);
    DisableShader();glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    // Wide low-alpha halo.
    Color({0.03f,0.24f,1.0f,0.09f*activity});glBegin(GL_TRIANGLES);
    glVertex3f(x+sideX*width*2.4f,y+sideY*width*2.4f,z);glVertex3f(x-sideX*width*2.4f,y-sideY*width*2.4f,z);glVertex3f(x+dx*length*1.30f*pulse,y+dy*length*1.30f*pulse,z-.05f);glEnd();
    // Saturated plasma body.
    Color({0.10f,0.58f,1.0f,0.46f*activity});glBegin(GL_TRIANGLES);
    glVertex3f(x+sideX*width*1.05f,y+sideY*width*1.05f,z+.005f);glVertex3f(x-sideX*width*1.05f,y-sideY*width*1.05f,z+.005f);glVertex3f(x+dx*length*.96f*pulse,y+dy*length*.96f*pulse,z+.005f);glEnd();
    // White-hot core/nozzle streak.
    Line(x,y,z+.025f,x+dx*length*.58f,y+dy*length*.58f,z+.025f,{.94f,.99f,1.0f,.92f*activity},std::max(1.2f,width*7.5f));
    FilledCircle(x,y,z+.03f,std::max(.035f,width*.72f),{.72f,.92f,1.0f,.75f*activity},12);
    // Sparse deterministic ion breakup near the tail.
    glPointSize(2.0f);glBegin(GL_POINTS);for(int i=0;i<8;++i){const float t=(i+1)/9.0f;const float wob=std::sin(phase*11.0f+i*2.17f)*width*(.2f+t*.6f);Color({.28f,.72f,1.0f,.35f*activity*(1.0f-t*.65f)});glVertex3f(x+dx*length*t+sideX*wob,y+dy*length*t+sideY*wob,z+.02f);}glEnd();
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);glEnable(GL_LIGHTING);
}

Vector3 TransformShipLocalPose(float x,float y,float yaw,const Vector3& local,const ShipRenderAxisScale& axis) {
    const float sy=std::sin(yaw),cy=std::cos(yaw);
    const float lx=local.x*axis.x,ly=local.y*axis.y;
    return {x+lx*cy-ly*sy,y+lx*sy+ly*cy,local.z*axis.z};
}

Vector3 TransformShipLocal(const PhysicsComponent& player,const Vector3& local,const ShipRenderAxisScale& axis) {
    return TransformShipLocalPose(player.position.x,player.position.y,player.rotation.z,local,axis);
}

float DirectionalAxisScale(const Vector3& direction,const ShipRenderAxisScale& axis){
    const float ax=std::fabs(direction.x),ay=std::fabs(direction.y),az=std::fabs(direction.z);
    const float total=std::max(.0001f,ax+ay+az);
    return (ax*axis.x+ay*axis.y+az*axis.z)/total;
}

Vector3 RotateShipDirection(const PhysicsComponent& player,const Vector3& local) {
    const float yaw=player.rotation.z,sy=std::sin(yaw),cy=std::cos(yaw);
    return {local.x*cy-local.y*sy,local.x*sy+local.y*cy,local.z};
}

// Legacy detached thruster modules/pylons intentionally removed in Pass429.
// Visible propulsion now comes only from authored Shipyard recipe modules;
// this renderer retains only the plume-effect transform helpers above.

void DrawAsteroid3D(const AsteroidData& asteroid,int index) {
    const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(asteroid.position);
    const float r=std::clamp(asteroid.size*0.019f,0.20f,0.82f);
    const Rgba rock={0.30f+0.02f*(index%3),0.285f,0.27f,1.0f};
    SetMaterial(rock,5.0f,0.0f,SpaceMaterialKind::AsteroidRock);
    const int slices=9,stacks=5;
    for(int stack=0;stack<stacks;++stack){
        const float lat0=-kPi*0.5f+static_cast<float>(stack)/stacks*kPi;
        const float lat1=-kPi*0.5f+static_cast<float>(stack+1)/stacks*kPi;
        glBegin(GL_QUAD_STRIP);
        for(int s=0;s<=slices;++s){
            const float lon=static_cast<float>(s%slices)/slices*2*kPi;
            for(float lat : {lat0,lat1}){
                const float wobble=0.78f+0.14f*std::sin(lon*3.0f+index*1.7f)+0.08f*std::cos(lat*5.0f+index);
                const float cl=std::cos(lat),sl=std::sin(lat),co=std::cos(lon),si=std::sin(lon);
                glNormal3f(co*cl,si*cl,sl);
                glVertex3f(p.x+r*wobble*co*cl,p.y+r*wobble*si*cl,0.18f+r*wobble*sl);
            }
        }
        glEnd();
    }
}

StationArchetype StationArchetypeFromText(const std::string& type) {
    const std::string t=ToUpperAscii(type);
    if(t.find("INDUSTR")!=std::string::npos||t.find("REFIN")!=std::string::npos)return StationArchetype::IndustrialRefinery;
    if(t.find("MIN")!=std::string::npos)return StationArchetype::MiningDepot;
    if(t.find("SHIPYARD")!=std::string::npos)return StationArchetype::Shipyard;
    if(t.find("MIL")!=std::string::npos||t.find("DEFEN")!=std::string::npos)return StationArchetype::Military;
    if(t.find("RESEARCH")!=std::string::npos||t.find("SCI")!=std::string::npos)return StationArchetype::Research;
    if(t.find("TETHER")!=std::string::npos||t.find("ELEVATOR")!=std::string::npos)return StationArchetype::TetherTerminal;
    if(t.find("FRONTIER")!=std::string::npos||t.find("OUTPOST")!=std::string::npos)return StationArchetype::FrontierOutpost;
    if(t.find("ASTEROID")!=std::string::npos)return StationArchetype::AsteroidStation;
    if(t.find("CORPORATE")!=std::string::npos||t.find("HQ")!=std::string::npos)return StationArchetype::CorporateHQ;
    return StationArchetype::TradeHub;
}

Rgba StationKitbashColor(StationArchetype archetype, SpaceMaterialKind material) {
    Rgba shell{0.28f,0.40f,0.46f,1.0f};
    switch(archetype){
        case StationArchetype::Military:shell={0.31f,0.33f,0.35f,1};break;
        case StationArchetype::IndustrialRefinery:case StationArchetype::MiningDepot:shell={0.37f,0.33f,0.27f,1};break;
        case StationArchetype::Research:shell={0.25f,0.42f,0.54f,1};break;
        case StationArchetype::TetherTerminal:shell={0.22f,0.48f,0.52f,1};break;
        case StationArchetype::CorporateHQ:shell={0.34f,0.42f,0.52f,1};break;
        case StationArchetype::Shipyard:shell={0.32f,0.36f,0.38f,1};break;
        default:break;
    }
    if(material==SpaceMaterialKind::IndustrialHull)return {shell.r*.72f,shell.g*.72f,shell.b*.68f,1};
    if(material==SpaceMaterialKind::Canopy)return {0.08f,0.42f,0.56f,0.92f};
    return shell;
}

bool DrawStationKitbash(const NativeBattlefieldRenderer::VisualAssets& assets,const Vector3& p,
                        StationArchetype archetype,std::uint64_t seed,bool asteroidEmbedded,
                        StationKitbashVisualRecipe* outRecipe=nullptr) {
    StationRuntimeQualitySystem quality;
    const auto certification=quality.Audit(assets.shipyardCatalog,archetype,seed,asteroidEmbedded);
    const auto recipe=StationKitbashVisualSystem::Build(assets.shipyardCatalog,archetype,seed,asteroidEmbedded);
    if(outRecipe)*outRecipe=recipe;
    if(!recipe.resolved)return false;
    // Pass523: if certified structural content exists, render the shared kitbash
    // recipe even when a secondary archetype-quality preference is missing.
    // Primitive fallback is reserved for truly missing structural content.
    if(!certification.certified && certification.primitiveFallbackAllowed)return false;
    glPushMatrix();
    glTranslatef(p.x,p.y,0.28f);
    for(const auto& placement:recipe.modules){
        DrawModulePlacement(assets,placement,StationKitbashColor(archetype,placement.material),1.0f,nullptr);
    }
    glPopMatrix();
    return true;
}

void DrawStationDockingReadability(const Vector3& p,const Vector3& apertureLocal) {
    // Pass563: docking readability follows the actual generated dock anchor.
    // It may no longer float at a hard-coded station-space Y offset.
    const float ax=p.x+apertureLocal.x, ay=p.y+apertureLocal.y, az=0.28f+apertureLocal.z;
    DrawBox(ax,ay,az,4.4f,2.20f,1.30f,{0.16f,0.20f,0.23f,1.0f});
    DrawBox(ax,ay-.90f,az+.02f,2.60f,0.18f,0.86f,{0.02f,0.08f,0.10f,1.0f});
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(int i=-3;i<=3;++i){const float lx=ax+i*0.34f;FilledCircle(lx,ay+3.23f,az+.28f,0.055f,{0.18f,0.78f,0.92f,0.76f},10);}
    FilledCircle(ax-1.15f,ay+3.73f,az+.24f,0.07f,{0.92f,0.18f,0.12f,0.80f},10);
    FilledCircle(ax+1.15f,ay+3.73f,az+.24f,0.07f,{0.18f,0.92f,0.42f,0.80f},10);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawStationActivity(const Vector3& p,StationArchetype archetype,std::uint64_t seed) {
    // Pass531: stations are active traffic nodes rather than static scenery.
    // The craft are deterministic presentation proxies for approach/departure
    // lanes; gameplay traffic identity remains owned by generated contacts.
    StationActivityPresentationSystem activity;
    const int population=700+static_cast<int>(seed%1800u);
    const auto profile=activity.Build(population,archetype,DockingExperienceStage::Undocked);
    const int craft=std::min(8,profile.inboundTraffic+profile.outboundTraffic+profile.securityCraft);
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(int i=0;i<craft;++i){
        const float lane=float(i%std::max(1,profile.approachLanes));
        const float dir=(i&1)?-1.0f:1.0f;
        const float phase=std::fmod(float(gRenderTime*.11)+float((seed>>((i%4)*8))&255u)/255.0f+float(i)*.137f,1.0f);
        const float radial=7.2f+lane*1.35f+phase*4.2f;
        const float angle=.35f*float(i)+dir*phase*1.2f;
        const float x=p.x+std::cos(angle)*radial;
        const float y=p.y+std::sin(angle)*radial;
        const Rgba c=i<profile.securityCraft?Rgba{.92f,.32f,.20f,.72f}:Rgba{.22f,.80f,.92f,.62f};
        DrawBox(x,y,1.0f,0.16f,0.32f,0.11f,c);
        Line(x,y,1.0f,p.x+std::cos(angle)*(radial-1.2f),p.y+std::sin(angle)*(radial-1.2f),1.0f,{c.r,c.g,c.b,.18f},.65f);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawStation3D(const NativeBattlefieldRenderer::VisualAssets& assets,const StationData& station) {
    const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(station.position);
    const auto archetype=StationArchetypeFromText(station.stationType);
    const auto seed=static_cast<std::uint64_t>(ProceduralVisualVariantSystem::StableSeed(station.stationId.empty()?station.name:station.stationId));
    StationKitbashVisualRecipe recipe;
    if(DrawStationKitbash(assets,p,archetype,seed,false,&recipe)){
        DrawStationDockingReadability(p,recipe.primaryDockLocal);
        DrawStationActivity(p,archetype,seed);
        return;
    }
    // Content-missing fallback only. Production stations should resolve the
    // shared Shipyard catalog above instead of living as permanent torus boxes.
    DrawTorus(p.x,p.y,1.25f,5.20f,0.32f,{0.25f,0.43f,0.52f,1.0f},36,8);
    DrawTorus(p.x,p.y,1.25f,3.10f,0.22f,{0.40f,0.48f,0.54f,1.0f},30,7);
    DrawBox(p.x,p.y,1.25f,1.20f,1.20f,4.80f,{0.28f,0.34f,0.39f,1.0f});
    DrawStationDockingReadability(p,{0.0f,-6.45f,0.50f});
    DrawStationActivity(p,archetype,seed);
}

void DrawRuntimeStation3D(const NativeBattlefieldRenderer::VisualAssets& assets,const RuntimeStationContact& station) {
    const Vector3 p=station.position;
    const auto stationSeed=station.id?station.id:static_cast<std::uint64_t>(ProceduralVisualVariantSystem::StableSeed(station.name));
    StationKitbashVisualRecipe recipe;
    if(DrawStationKitbash(assets,p,station.archetype,stationSeed,station.asteroidEmbedded,&recipe)){
        DrawStationDockingReadability(p,recipe.primaryDockLocal);
        DrawStationActivity(p,station.archetype,stationSeed);
        return;
    }
    if(station.asteroidEmbedded){
        SetMaterial({0.24f,0.225f,0.21f,1.0f},5.0f,0.0f,SpaceMaterialKind::AsteroidRock);
        DrawSphere(p.x,p.y,0.30f,3.4f,{0.25f,0.235f,0.22f,1.0f},22,12,SpaceMaterialKind::AsteroidRock);
        DrawBox(p.x,p.y-2.55f,0.42f,2.1f,1.45f,0.92f,{0.13f,0.18f,0.20f,1.0f});
        DrawBox(p.x,p.y-3.18f,0.44f,1.35f,0.18f,0.58f,{0.02f,0.07f,0.09f,1.0f});
        DrawBox(p.x-2.45f,p.y+0.45f,1.15f,0.34f,2.2f,0.34f,{0.30f,0.36f,0.38f,1.0f});
        DrawBox(p.x+2.45f,p.y+0.45f,1.15f,0.34f,2.2f,0.34f,{0.30f,0.36f,0.38f,1.0f});
    }else{
        Rgba shell{0.28f,0.40f,0.46f,1.0f};
        float ring=4.20f;
        switch(station.archetype){
            case StationArchetype::TetherTerminal:shell={0.24f,0.50f,0.54f,1.0f};ring=5.10f;break;
            case StationArchetype::Military:shell={0.34f,0.35f,0.38f,1.0f};ring=3.80f;break;
            case StationArchetype::IndustrialRefinery:case StationArchetype::MiningDepot:shell={0.38f,0.34f,0.28f,1.0f};ring=4.65f;break;
            case StationArchetype::Research:shell={0.28f,0.42f,0.54f,1.0f};ring=4.00f;break;
            default:break;
        }
        DrawTorus(p.x,p.y,1.10f,ring,0.28f,shell,32,7);
        DrawBox(p.x,p.y,1.10f,1.05f,1.05f,4.2f,{0.30f,0.35f,0.39f,1.0f});
        DrawBox(p.x,p.y-5.65f,0.68f,3.6f,1.65f,1.05f,{0.14f,0.20f,0.22f,1.0f});
        if(station.archetype==StationArchetype::TetherTerminal){
            DrawBox(p.x,p.y+2.05f,0.15f,0.22f,4.2f,0.22f,{0.38f,0.48f,0.50f,1.0f});
        }
    }
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    FilledCircle(p.x-0.56f,p.y-2.72f,1.05f,0.07f,{0.16f,0.88f,0.76f,0.82f},10);
    FilledCircle(p.x+0.56f,p.y-2.72f,1.05f,0.07f,{0.18f,0.72f,0.94f,0.82f},10);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
    DrawStationActivity(p,station.archetype,stationSeed);
}

void DrawOrbitalHub3D(const OrbitalHubData& hub) {
    const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(hub.position);
    DrawTorus(p.x,p.y,0.68f,1.42f,0.13f,{0.24f,0.58f,0.66f,1.0f},32,7);
    DrawBox(p.x,p.y,0.68f,0.42f,2.9f,0.42f,{0.42f,0.48f,0.52f,1.0f});
    DrawBox(p.x,p.y,0.68f,2.9f,0.34f,0.34f,{0.34f,0.40f,0.44f,1.0f});
}

bool DrawImportedTexturedPlanetSphere(const NativeBattlefieldRenderer::VisualAssets& assets,
                                     const std::string& textureKey,
                                     float cx,float cy,float cz,float radius,
                                     float rotationPhase,int slices,int stacks,float alpha=1.0f,bool cloud=false){
    const auto it=assets.planetTextures.find(textureKey);if(it==assets.planetTextures.end()||!it->second)return false;
    DisableShader();SetupSceneLighting();glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,static_cast<GLuint>(it->second));glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    if(cloud){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);}
    glColor4f(1.0f,1.0f,1.0f,alpha);
    for(int stack=0;stack<stacks;++stack){
        const float v0=float(stack)/stacks,v1=float(stack+1)/stacks,lat0=-kPi*.5f+v0*kPi,lat1=-kPi*.5f+v1*kPi;
        const float z0=std::sin(lat0),zr0=std::cos(lat0),z1=std::sin(lat1),zr1=std::cos(lat1);glBegin(GL_QUAD_STRIP);
        for(int slice=0;slice<=slices;++slice){
            const float f=float(slice)/slices,lon=f*2*kPi,x=std::cos(lon),y=std::sin(lon);const float u=f+rotationPhase;
            glNormal3f(x*zr0,y*zr0,z0);glTexCoord2f(u,1.0f-v0);glVertex3f(cx+radius*x*zr0,cy+radius*y*zr0,cz+radius*z0);
            glNormal3f(x*zr1,y*zr1,z1);glTexCoord2f(u,1.0f-v1);glVertex3f(cx+radius*x*zr1,cy+radius*y*zr1,cz+radius*z1);
        }glEnd();
    }
    glBindTexture(GL_TEXTURE_2D,0);glDisable(GL_TEXTURE_2D);if(cloud){glDepthMask(GL_TRUE);glDisable(GL_BLEND);}return true;
}

CelestialRenderTier PlanetRenderTier(const Vector3& center,float radius){
    const float dx=gCameraEye.x-center.x,dy=gCameraEye.y-center.y,dz=gCameraEye.z-center.z;
    const float distance=std::max(1.0f,std::sqrt(dx*dx+dy*dy+dz*dz));
    const float apparent=radius/distance;
    if(apparent>=.115f)return CelestialRenderTier::Near;
    if(apparent>=.028f)return CelestialRenderTier::Mid;
    return CelestialRenderTier::Far;
}

void DrawPlanet3D(const PlanetData& planet,std::size_t index,const NativeBattlefieldRenderer::VisualAssets& assets) {
    CelestialEnvironmentSystem celestial;const auto profile=celestial.ProfileFor(planet);const auto surface=PlanetSurfaceSystem::Build(planet);const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(planet.position);const float r=celestial.WorldRadius(planet);const float z=-std::max(3.0f,r*0.58f)-float(index%2)*1.3f;
    const Vector3 center3{p.x,p.y,z};const auto tier=PlanetRenderTier(center3,r);
    if(tier==CelestialRenderTier::Near)++gCelestialFrameTelemetry.nearBodies;else if(tier==CelestialRenderTier::Mid)++gCelestialFrameTelemetry.midBodies;else ++gCelestialFrameTelemetry.farBodies;
    const int surfaceSlices=tier==CelestialRenderTier::Near?80:(tier==CelestialRenderTier::Mid?56:32);
    const int surfaceStacks=tier==CelestialRenderTier::Near?40:(tier==CelestialRenderTier::Mid?28:16);
    const int cloudSlices=tier==CelestialRenderTier::Near?72:(tier==CelestialRenderTier::Mid?48:28);
    const int cloudStacks=tier==CelestialRenderTier::Near?36:(tier==CelestialRenderTier::Mid?24:14);
    const int atmosphereSlices=tier==CelestialRenderTier::Near?56:(tier==CelestialRenderTier::Mid?36:24);
    const int atmosphereStacks=tier==CelestialRenderTier::Near?28:(tier==CelestialRenderTier::Mid?18:12);

    // Imported Various Planets artwork is the visual authority. Runtime adds
    // lighting/orbit/weather motion, but it no longer buries the source cloud
    // texture under an equally opaque procedural weather sheet.
    const auto imported=ImportedPlanetVisualSystem::ProfileFor(planet.type);
    const float surfaceSpin=PlanetPresentationSystem::SurfaceRotationPhase(gRenderTime*imported.surfaceRotationRate,surface.surfaceSeed);
    const bool importedSurface=assets.importedPlanetPackReady && DrawImportedTexturedPlanetSphere(assets,imported.surfaceTexture,p.x,p.y,z,r,surfaceSpin,surfaceSlices,surfaceStacks,1.0f,false);
    if(!importedSurface)DrawDedicatedPlanetSurface(planet,surface,p.x,p.y,z,r,tier==CelestialRenderTier::Near?72:(tier==CelestialRenderTier::Mid?48:32),tier==CelestialRenderTier::Near?36:(tier==CelestialRenderTier::Mid?24:16));

    bool importedCloud=false;
    if(assets.importedPlanetPackReady&&!imported.cloudTextures.empty()){
        for(std::size_t ci=0;ci<imported.cloudTextures.size();++ci){
            if(tier==CelestialRenderTier::Far&&ci>0)break; // distant bodies never pay for multiple alpha shells
            const float cloudRate=ci==0?imported.primaryCloudRotationRate:imported.secondaryCloudRotationRate;
            const float cloudSpin=PlanetPresentationSystem::CloudRotationPhase(gRenderTime*std::fabs(cloudRate),static_cast<int>(ci),surface.surfaceSeed)*(cloudRate<0.0f?-1.0f:1.0f);
            const float importedAlpha=ci==0?1.0f:.82f; // preserve source cloud contrast; PNG/JPG alpha remains authoritative
            if(DrawImportedTexturedPlanetSphere(assets,imported.cloudTextures[ci],p.x,p.y,z,r*(imported.cloudRadiusMultiplier+.004f*float(ci)),cloudSpin,cloudSlices,cloudStacks,importedAlpha,true)){
                importedCloud=true;++gCelestialFrameTelemetry.importedCloudLayers;
            }
        }
    }

    const bool wantsClouds=profile.hasCloudLayer||!imported.cloudTextures.empty();
    if(wantsClouds){
        // Source cloud art should dominate whenever it exists. Procedural weather
        // remains visible reinforcement, but only as a subtle near-body breakup layer
        // instead of a detail-destroying overlay.
        if(importedCloud){
            if(tier==CelestialRenderTier::Near){
                const float reinforcement=std::clamp(std::max(surface.cloudOpacity,profile.cloudOpacity)*.22f,.045f,.13f);
                const Rgba clouds{surface.cloudColor[0],surface.cloudColor[1],surface.cloudColor[2],reinforcement};
                DrawLitPlanetClouds(p.x,p.y,z,r*(PlanetAtmospherePresentationSystem::Default().cloudRadiusMultiplier+.006f),clouds,surface.surfaceSeed+47.0f,.72f,58,29);
                ++gCelestialFrameTelemetry.proceduralCloudLayers;
            }
        }else if(tier!=CelestialRenderTier::Far){
            const float visibleCloudAlpha=std::clamp(std::max(surface.cloudOpacity,profile.cloudOpacity),.22f,.58f);
            const Rgba clouds{surface.cloudColor[0],surface.cloudColor[1],surface.cloudColor[2],visibleCloudAlpha};
            DrawLitPlanetClouds(p.x,p.y,z,r*PlanetAtmospherePresentationSystem::Default().cloudRadiusMultiplier,clouds,surface.surfaceSeed,1.0f,tier==CelestialRenderTier::Near?80:52,tier==CelestialRenderTier::Near?40:26);
            ++gCelestialFrameTelemetry.proceduralCloudLayers;
            if(tier==CelestialRenderTier::Near&&surface.stormStrength>.20f){DrawLitPlanetClouds(p.x,p.y,z,r*(PlanetAtmospherePresentationSystem::Default().cloudRadiusMultiplier+.008f),{clouds.r*.92f,clouds.g*.94f,clouds.b*.98f,clouds.a*(.22f+.18f*surface.stormStrength)},surface.surfaceSeed+193,-.62f,58,29);++gCelestialFrameTelemetry.proceduralCloudLayers;}
        }
    }

    // Pass489-491: weather is an evolving state rather than a rigidly rotating
    // decal. Near worlds expose localized storm cells; gas giants additionally
    // carry counter-rotating band drift and a larger persistent vortex budget.
    if(tier==CelestialRenderTier::Near){
        auto weather=PlanetWeatherSystem::Initialize(planet,static_cast<std::uint32_t>(surface.surfaceSeed*997.0f)+31u);
        PlanetWeatherSystem::Advance(weather,planet,gRenderTime);
        DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        int shown=0;for(const auto& storm:weather.storms){if(shown++>=4)break;const float lon=storm.longitude,lat=storm.latitude;const float cl=std::cos(lat),sl=std::sin(lat);const float shell=r*(PlanetAtmospherePresentationSystem::Default().cloudRadiusMultiplier+.010f);const float sx=p.x+std::cos(lon)*cl*shell,sy=p.y+std::sin(lon)*cl*shell,sz=z+sl*shell;const float sr=std::max(.08f,r*storm.radius*.42f);const Rgba sc=planet.type==PlanetType::GasGiant?Rgba{.94f,.84f,.70f,.12f+.18f*storm.intensity}:Rgba{.92f,.96f,1.0f,.10f+.20f*storm.intensity};Ring(sx,sy,sz,sr,sc,1.1f+storm.intensity*1.6f,24);Ring(sx,sy,sz,sr*.62f,{sc.r,sc.g,sc.b,sc.a*.72f},1.0f,20);}
        if(planet.type==PlanetType::GasGiant){const auto giant=GasGiantWeatherSystem::Build(planet,static_cast<std::uint32_t>(surface.surfaceSeed*173.0f)+7u);if(giant.volatileAtmosphere&&gRenderTime>0){const float pulse=.5f+.5f*std::sin(gRenderTime*2.4f+surface.surfaceSeed);if(pulse>.82f)Ring(p.x-r*.23f,p.y+r*.16f,z+r*.42f,r*.11f,{1.0f,.84f,.58f,.25f},2.0f,28);}}
        glDisable(GL_BLEND);SetupSceneLighting();
    }

    const auto bands=celestial.RingBands(planet);if(!bands.empty()){SetupSceneLighting();glPushMatrix();glTranslatef(p.x,p.y,z);glRotatef(profile.ringTiltDegrees,1,0,0);glRotatef(11,0,1,0);const int ringSegments=tier==CelestialRenderTier::Near?72:(tier==CelestialRenderTier::Mid?48:32);for(std::size_t i=0;i<bands.size();++i){const auto& b=bands[i];float mid=(b.innerRadiusMultiplier+b.outerRadiusMultiplier)*.5f*r,width=std::max(r*.018f,(b.outerRadiusMultiplier-b.innerRadiusMultiplier)*r*.12f),shade=.90f-float(i)*.08f;DrawTorus(0,0,0,mid,width,{.68f*shade,.62f*shade,.50f*shade,b.opacity},ringSegments,tier==CelestialRenderTier::Far?5:7);}glPopMatrix();}
    const Rgba atmosphere{surface.atmosphereColor[0],surface.atmosphereColor[1],surface.atmosphereColor[2],std::min(.34f,surface.atmosphereOpacity*1.18f)};
    DrawAtmosphereLimb(p.x,p.y,z,r*imported.atmosphereRadiusMultiplier,atmosphere,atmosphereSlices,atmosphereStacks);SetupSceneLighting();
}

void DrawSun3D(const StarData& star) {
    const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(star.position);
    const auto visual=SolarPresentationSystem::VisualFor(star);
    const float r=visual.worldRadius;
    const float z=-std::max(18.0f,r*.32f);
    const Rgba core{star.colorR,star.colorG,star.colorB,1.0f};
    gSurfaceOverride.active=true;
    gSurfaceOverride.detailR=std::min(1.0f,star.colorR*1.08f);
    gSurfaceOverride.detailG=std::min(1.0f,star.colorG*.94f+.08f);
    gSurfaceOverride.detailB=std::min(1.0f,star.colorB*.82f+.06f);
    gSurfaceOverride.variation=visual.photosphereVariation;
    gSurfaceOverride.detailScale=8.0f;
    gSurfaceOverride.bandStrength=visual.prominenceStrength;
    gSurfaceOverride.surfaceSeed=static_cast<float>((std::hash<std::string>{}(star.starId)%10000u)+1u);
    DrawSphere(p.x,p.y,z,r,core,64,32,SpaceMaterialKind::Sun);
    gSurfaceOverride.active=false;
    DisableSceneLighting();
    glDepthMask(GL_FALSE); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    DrawUnlitSphere(p.x,p.y,z,r*visual.coronaInnerMultiplier,{star.colorR,star.colorG,star.colorB,0.13f},42,18);
    DrawUnlitSphere(p.x,p.y,z,r*visual.coronaOuterMultiplier,{star.colorR,star.colorG*.92f,star.colorB*.72f,0.038f},34,14);
    // Sparse animated prominences keep the star from reading as a static ball.
    for(int i=0;i<7;++i){
        const float a=gRenderTime*(.045f+.008f*i)+static_cast<float>(i)*.897f;
        const float rr=r*(1.05f+.07f*std::sin(gRenderTime*.31f+i));
        FilledCircle(p.x+std::cos(a)*rr,p.y+std::sin(a)*rr,z+r*.02f,r*(.018f+.006f*(i%3)),
                     {star.colorR,star.colorG*.72f,star.colorB*.48f,.12f+visual.prominenceStrength*.10f},12);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);glDepthMask(GL_TRUE);
    SetupSceneLighting();
}

Rgba SiteColor(SectorSiteType type) {
    switch(type){
        case SectorSiteType::MiningField:return {0.20f,0.72f,0.70f,0.70f};
        case SectorSiteType::SalvageSite:return {0.88f,0.58f,0.18f,0.72f};
        case SectorSiteType::DerelictYard:return {0.72f,0.42f,0.20f,0.70f};
        case SectorSiteType::OrbitalRelay:return {0.28f,0.66f,0.88f,0.72f};
        case SectorSiteType::ResearchOutpost:return {0.50f,0.72f,0.92f,0.72f};
        case SectorSiteType::TradeLane:return {0.30f,0.80f,0.56f,0.62f};
        case SectorSiteType::DistressWreck:return {0.94f,0.28f,0.18f,0.78f};
        case SectorSiteType::AnomalyBeacon:return {0.64f,0.34f,0.92f,0.72f};
        case SectorSiteType::IndustrialDepot:return {0.76f,0.68f,0.30f,0.70f};
    }
    return {0.5f,0.7f,0.8f,0.7f};
}

void DrawSectorSites(const NativeBattlefieldFrame& frame) {
    DisableSceneLighting(); glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    for(std::size_t i=0;i<frame.sector->pointsOfInterest.size();++i){
        const auto& site=frame.sector->pointsOfInterest[i];
        const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(site.position);
        const float r=std::clamp(site.radius*kSectorPositionScale,0.55f,2.6f);
        Rgba c=SiteColor(site.type);
        Ring(p.x,p.y,0.03f,r,c,1.0f,44);
        Line(p.x,p.y,0.03f,p.x,p.y,0.62f,{c.r,c.g,c.b,0.42f},1.2f);
        if(frame.selection.kind==NativeContactKind::Site&&frame.selection.index==i)
            Ring(p.x,p.y,0.05f,r*1.18f,{1.0f,0.70f,0.20f,0.88f},1.8f,44);
    }
    glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawMissilesAndMiningEffects(const NativeBattlefieldFrame& frame) {
    if(frame.missileSystem){
        for(const auto& m:frame.missileSystem->GetProjectiles()){
            const float yaw=std::atan2(-m.velocity.x,m.velocity.y);
            glPushMatrix();glTranslatef(m.position.x,m.position.y,0.42f);glRotatef(yaw*180.0f/kPi,0,0,1);
            DrawBox(0,0,0,0.10f,0.38f,0.10f,m.payload==MissilePayloadType::MiningFracture?Rgba{0.76f,0.58f,0.18f,1}:Rgba{0.50f,0.54f,0.58f,1},SpaceMaterialKind::MissileBody);
            glPopMatrix();
            Vector3 dir=m.velocity.length()>0.001f?m.velocity.normalized():Vector3{0,1,0};
            DrawThrusterPlume(m.position.x,m.position.y,0.42f,-dir.x,-dir.y,0.72f,0.055f,0.82f,frame.elapsedSeconds+static_cast<float>(m.id)*0.13f);
        }
        DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        for(const auto& d:frame.missileSystem->GetDetonations()){
            const float t=std::clamp(d.age/std::max(0.01f,d.lifetime),0.0f,1.0f);
            const float r=d.radius*(0.24f+0.95f*t);
            const Rgba c=d.payload==MissilePayloadType::MiningFracture?Rgba{0.92f,0.64f,0.24f,0.55f*(1-t)}:Rgba{1.0f,0.36f,0.12f,0.62f*(1-t)};
            Ring(d.position.x,d.position.y,0.28f,r,c,2.0f,48);
            FilledCircle(d.position.x,d.position.y,0.30f,r*0.24f,{c.r,c.g,c.b,0.24f*(1-t)},28);
            const int burstCount=std::clamp(d.particleBudget/4,12,48);
            glPointSize(2.0f+3.0f*(1.0f-t)); glBegin(GL_POINTS);
            for(int i=0;i<burstCount;++i){
                std::uint32_t h=static_cast<std::uint32_t>(d.missileId*747796405u)+static_cast<std::uint32_t>(i*2891336453u);
                h^=h>>16; h*=2246822519u; h^=h>>13;
                const float a=static_cast<float>(h&0xffffu)/65535.0f*2.0f*kPi;
                const float rr=r*(0.20f+0.92f*static_cast<float>((h>>16)&0xffu)/255.0f);
                Color({c.r,c.g,c.b,0.46f*(1.0f-t)});
                glVertex3f(d.position.x+std::cos(a)*rr,d.position.y+std::sin(a)*rr,0.30f+0.35f*std::sin(a*3.0f));
            }
            glEnd();
        }
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
    }

    if(frame.fractureSystem){
        for(const auto& f:frame.fractureSystem->GetFragments()){
            if(f.recovered)continue;
            DrawBox(f.position.x,f.position.y,0.18f+f.position.z,f.size,f.size*1.15f,f.size*0.72f,
                    {0.38f,0.34f,0.29f,1.0f},SpaceMaterialKind::Debris);
        }
        DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        for(const auto& cloud:frame.fractureSystem->GetDustClouds()){
            const float t=std::clamp(cloud.age/std::max(0.01f,cloud.lifetime),0.0f,1.0f);
            const float r=cloud.baseRadius*(0.55f+1.65f*t);
            const int count=28;
            glPointSize(2.0f+2.0f*(1-t));glBegin(GL_POINTS);
            for(int i=0;i<count;++i){
                std::uint32_t h=cloud.seed+static_cast<std::uint32_t>(i*2654435761u);h^=h>>16;h*=2246822519u;h^=h>>13;
                const float a=static_cast<float>(h&0xffffu)/65535.0f*2*kPi;
                const float rr=r*(0.18f+0.82f*static_cast<float>((h>>16)&0xffu)/255.0f);
                Color({0.46f,0.40f,0.34f,cloud.density*0.22f*(1-t)});glVertex3f(cloud.position.x+std::cos(a)*rr,cloud.position.y+std::sin(a)*rr,0.25f+0.30f*std::sin(a*2.0f));
            }
            glEnd();
        }
        glDisable(GL_BLEND);SetupSceneLighting();
    }
}

void DrawDerelict3D(const NativeBattlefieldRenderer::VisualAssets& assets,const DerelictData& d) {
    const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(d.position);
    DrawModularShip(assets,p.x,p.y,0.10f,d.orientation,0.115f,false,"SALVAGE",{0.30f,0.27f,0.24f,1.0f},0.72f,0.028f,ProceduralVisualVariantSystem::StableSeed(d.derelictId));
    DisableSceneLighting();
    Line(p.x-0.8f,p.y-0.5f,0.62f,p.x+0.8f,p.y+0.5f,0.62f,{0.92f,0.30f,0.12f,0.55f},2.0f);
    SetupSceneLighting();
}

void DrawDebrisFields(const NativeBattlefieldFrame& frame) {
    for(std::size_t fieldIndex=0;fieldIndex<frame.sector->debrisFields.size();++fieldIndex){
        const auto& field=frame.sector->debrisFields[fieldIndex];
        const Vector3 c=NativeBattlefieldRenderer::SectorToWorld(field.position);
        const float r=std::clamp(field.radius*kSectorPositionScale,1.2f,5.5f);
        const int count=12+static_cast<int>(field.density*24.0f);
        for(int i=0;i<count;++i){
            const float a=static_cast<float>(i)*2.399963f+static_cast<float>(fieldIndex)*0.71f;
            const float d=r*(0.18f+0.80f*static_cast<float>((i*37)%97)/96.0f);
            const float z=0.10f+0.65f*std::sin(a*1.7f+i*0.2f);
            const float s=0.06f+0.09f*static_cast<float>((i*13)%11)/10.0f;
            DrawBox(c.x+std::cos(a)*d,c.y+std::sin(a)*d,z,s,s*1.6f,s*0.8f,{0.30f,0.29f,0.28f,1.0f});
        }
    }
}

void DrawAnomalies(const NativeBattlefieldFrame& frame) {
    DisableSceneLighting();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(std::size_t i=0;i<frame.sector->anomalies.size();++i){
        const auto& a=frame.sector->anomalies[i];
        const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(a.position);
        const float r=std::clamp(a.radius*kSectorPositionScale,0.8f,4.5f);
        Rgba c{0.24f,0.22f,0.55f,0.09f};
        if(a.type==AnomalyType::IonStorm)c={0.12f,0.48f,0.72f,0.10f};
        else if(a.type==AnomalyType::RadiationZone)c={0.58f,0.42f,0.10f,0.08f};
        else if(a.type==AnomalyType::BlackHole)c={0.35f,0.12f,0.50f,0.11f};
        FilledCircle(p.x,p.y,-1.0f,r,c,48);
        Ring(p.x,p.y,-0.95f,r*0.72f,{c.r*1.4f,c.g*1.4f,c.b*1.4f,0.22f},1.0f,48);
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_BLEND); SetupSceneLighting();
}

struct ShieldRipplePulse{
    Vector3 localPoint{};
    float started=-100.0f;
    float strength=0.0f;
    std::uint64_t sourceId=0;
    bool active=false;
};

struct ShieldSurfaceState{
    float lastShieldFraction=1.0f;
    bool initialized=false;
    std::uint64_t lastMissileId=~std::uint64_t{0};
    std::size_t nextPulse=0;
    std::array<ShieldRipplePulse,4> pulses{};
};

void DrawShipProfileShield(const NativeBattlefieldRenderer::VisualAssets& assets,const PhysicsComponent& p,
                           const ProceduralShipVisualRecipe* recipe,const ShipRenderAxisScale& axis,
                           float shieldFraction,float timeSeconds,const NativeBattlefieldFrame& frame) {
    shieldFraction=std::clamp(shieldFraction,0.0f,1.0f);if(shieldFraction<=0.005f||!recipe||recipe->modules.empty())return;
    constexpr float kShieldGapWorld=0.3048f; // one foot from the rendered hull surface
    constexpr float kShieldCalmWaveHeightWorld=0.0060f; // nearly-still pond surface
    constexpr float kShieldImpactWaveHeightWorld=0.065f; // localized strike ripple crest
    constexpr float kShieldRippleLifetime=2.35f;
    const auto& triangles=CachedShieldTriangles(assets,*recipe);if(triangles.empty())return;
    const float rootYaw=p.rotation.z+RecipeForwardVisualYawRadians(recipe);

    // Keep a tiny, bounded visual state per shielded physics body.  Nearby
    // missile detonations provide an exact impact locus when available; a
    // measured shield-energy drop provides a generic fallback for beams or
    // other damage sources that do not currently publish a renderer hit point.
    static std::unordered_map<const PhysicsComponent*,ShieldSurfaceState> surfaceStates;
    if(surfaceStates.size()>32)surfaceStates.clear();
    auto& surface=surfaceStates[&p];
    bool seededExactImpact=false;

    const auto recipeBounds=BuildRecipeLocalBounds(assets,*recipe);
    float shieldRadiusWorld=4.0f;
    if(recipeBounds.valid){
        const Vector3 half=(recipeBounds.max-recipeBounds.min)*.5f;
        shieldRadiusWorld=std::max(1.0f,std::sqrt((half.x*axis.x)*(half.x*axis.x)+(half.y*axis.y)*(half.y*axis.y)+(half.z*axis.z)*(half.z*axis.z))+kShieldGapWorld);
    }

    auto worldToShieldLocal=[&](const Vector3& world){
        const float dx=world.x-p.position.x,dy=world.y-p.position.y,dz=world.z-0.30f;
        const float c=std::cos(rootYaw),sn=std::sin(rootYaw);
        return Vector3{(c*dx+sn*dy)/std::max(.0001f,std::fabs(axis.x)),
                       (-sn*dx+c*dy)/std::max(.0001f,std::fabs(axis.y)),
                       dz/std::max(.0001f,std::fabs(axis.z))};
    };
    auto seedRipple=[&](const Vector3& localPoint,float strength,std::uint64_t sourceId,float started){
        auto& pulse=surface.pulses[surface.nextPulse%surface.pulses.size()];
        pulse.localPoint=localPoint;pulse.started=started;pulse.strength=std::clamp(strength,.20f,1.0f);pulse.sourceId=sourceId;pulse.active=true;
        surface.nextPulse=(surface.nextPulse+1)%surface.pulses.size();
    };

    if(frame.missileSystem){
        for(const auto& d:frame.missileSystem->GetDetonations()){
            if(d.missileId==surface.lastMissileId)continue;
            const float age=std::max(0.0f,d.age);if(age>std::min(.30f,d.lifetime))continue;
            if(Distance2D(d.position,p.position)>shieldRadiusWorld+std::max(.75f,d.radius*1.15f))continue;
            const float life=std::max(.01f,d.lifetime);const float strength=1.0f-std::clamp(age/life,0.0f,1.0f);
            seedRipple(worldToShieldLocal(d.position),.60f+.40f*strength,static_cast<std::uint64_t>(d.missileId),timeSeconds-age);
            surface.lastMissileId=static_cast<std::uint64_t>(d.missileId);seededExactImpact=true;
            break;
        }
    }

    if(!surface.initialized){surface.lastShieldFraction=shieldFraction;surface.initialized=true;}
    if(shieldFraction<surface.lastShieldFraction-.0025f&&!seededExactImpact){
        // Unknown-locus damage still gets a physical surface reaction instead
        // of flashing the entire bubble. Pick a deterministic perimeter point
        // from time so repeated hits do not stack at the same location.
        const float a=timeSeconds*1.6180339f;
        const Vector3 center=recipeBounds.valid?(recipeBounds.min+recipeBounds.max)*.5f:Vector3{};
        const Vector3 half=recipeBounds.valid?(recipeBounds.max-recipeBounds.min)*.5f:Vector3{1.0f,1.0f,1.0f};
        const Vector3 fallback{center.x+std::cos(a)*std::max(.25f,half.x),center.y+std::sin(a)*std::max(.25f,half.y),center.z+std::sin(a*.73f)*std::max(.10f,half.z)};
        const float loss=std::clamp(surface.lastShieldFraction-shieldFraction,0.0f,1.0f);
        seedRipple(fallback,.38f+.62f*std::min(1.0f,loss*5.0f),0,timeSeconds);
    }
    surface.lastShieldFraction=shieldFraction;

    // Calm state should read like a still pond: almost no breathing/pulsing,
    // just a very slow surface drift. Impacts inject concentric traveling waves
    // whose displacement, opacity and slight cyan/white color lift damp out.
    struct ActiveShieldRipple{Vector3 localPoint{};float age=0.0f;float temporalStrength=0.0f;};
    std::array<ActiveShieldRipple,4> activeRipples{};std::size_t activeRippleCount=0;
    for(auto& pulse:surface.pulses){
        if(!pulse.active)continue;const float age=timeSeconds-pulse.started;
        if(age<0.0f||age>kShieldRippleLifetime){if(age>kShieldRippleLifetime)pulse.active=false;continue;}
        activeRipples[activeRippleCount++]={pulse.localPoint,age,std::exp(-age*1.55f)*pulse.strength};
    }

    const float baseAlpha=.016f+.040f*shieldFraction;
    DisableShader();SetShipBaseTexture(0);glDisable(GL_LIGHTING);glEnable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
    glPushMatrix();glTranslatef(p.position.x,p.position.y,0.30f);glRotatef(rootYaw*180.0f/kPi,0,0,1);glScalef(axis.x,axis.y,axis.z);
    glBegin(GL_TRIANGLES);
    for(const auto& tri:triangles){
        const Vector3 center=tri.center;
        float rippleHeight=0.0f,impactEnergy=0.0f;
        for(std::size_t ri=0;ri<activeRippleCount;++ri){
            const auto& pulse=activeRipples[ri];const Vector3 d=center-pulse.localPoint;
            const float worldDistance=std::sqrt((d.x*axis.x)*(d.x*axis.x)+(d.y*axis.y)*(d.y*axis.y)+(d.z*axis.z)*(d.z*axis.z));
            const float spatial=std::exp(-worldDistance*.11f),wave=std::sin(worldDistance*5.7f-pulse.age*14.0f);
            const float envelope=pulse.temporalStrength*spatial;
            rippleHeight+=wave*envelope*kShieldImpactWaveHeightWorld;
            impactEnergy=std::max(impactEnergy,std::fabs(wave)*envelope);
        }

        const float calmPhase=timeSeconds*.62f+tri.phase*.22f+center.x*.13f+center.y*.09f;
        const float calmWave=(std::sin(calmPhase)+.45f*std::sin(calmPhase*.63f+1.7f))*kShieldCalmWaveHeightWorld;
        Vector3 wn{tri.n.x/std::max(.0001f,std::fabs(axis.x)),tri.n.y/std::max(.0001f,std::fabs(axis.y)),tri.n.z/std::max(.0001f,std::fabs(axis.z))};
        wn=wn.normalized();
        const float offsetWorld=kShieldGapWorld+calmWave+rippleHeight;
        const Vector3 off{wn.x/std::max(.0001f,std::fabs(axis.x))*offsetWorld,
                          wn.y/std::max(.0001f,std::fabs(axis.y))*offsetWorld,
                          wn.z/std::max(.0001f,std::fabs(axis.z))*offsetWorld};

        const float glassShimmer=.5f+.5f*std::sin(timeSeconds*.78f+tri.phase*.31f);
        const float colorLift=std::clamp(impactEnergy,0.0f,1.0f);
        Color({.035f+.025f*glassShimmer+.105f*colorLift,
               .43f+.10f*shieldFraction+.20f*colorLift,
               .76f+.10f*shieldFraction+.10f*colorLift,
               baseAlpha*(.90f+.10f*glassShimmer)+.060f*colorLift});
        const Vector3 a=tri.a+off,b=tri.b+off,c=tri.c+off;glVertex3f(a.x,a.y,a.z);glVertex3f(b.x,b.y,b.z);glVertex3f(c.x,c.y,c.z);
    }
    glEnd();glPopMatrix();glDepthMask(GL_TRUE);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawPlayableInterior(const NativeBattlefieldFrame& frame) {
    if(!frame.playerPhysics)return;
    const auto& p=frame.playerPhysics->position;
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    // One compact starter deck; later ship construction metadata can replace
    // these room bounds without changing the embodiment/camera authority.
    DrawBox(p.x,p.y,0.02f,4.2f,5.8f,0.10f,{0.045f,0.065f,0.074f,1.0f});
    DrawBox(p.x,p.y+2.82f,0.34f,4.2f,0.12f,0.62f,{0.18f,0.23f,0.25f,1.0f});
    DrawBox(p.x,p.y-2.82f,0.34f,4.2f,0.12f,0.62f,{0.18f,0.23f,0.25f,1.0f});
    DrawBox(p.x-2.04f,p.y,0.34f,0.12f,5.7f,0.62f,{0.18f,0.23f,0.25f,1.0f});
    DrawBox(p.x+2.04f,p.y,0.34f,0.12f,5.7f,0.62f,{0.18f,0.23f,0.25f,1.0f});
    DrawBox(p.x,p.y+1.55f,0.16f,2.8f,1.12f,0.20f,{0.10f,0.34f,0.43f,1.0f}); // cockpit
    DrawBox(p.x-1.05f,p.y-0.20f,0.17f,1.35f,1.55f,0.22f,{0.42f,0.27f,0.10f,1.0f}); // furnace/engineering
    DrawBox(p.x+1.05f,p.y-0.20f,0.17f,1.35f,1.55f,0.22f,{0.15f,0.37f,0.29f,1.0f}); // cargo/fabrication
    DrawBox(p.x,p.y-1.85f,0.17f,2.9f,0.82f,0.22f,{0.28f,0.31f,0.34f,1.0f});
    const Vector3 av{p.x+frame.interiorAvatar.localPosition.x*0.72f,p.y+frame.interiorAvatar.localPosition.y*0.72f,0.50f};
    DrawSphere(av.x,av.y,av.z,0.18f,{0.88f,0.66f,0.22f,1.0f},16,8,SpaceMaterialKind::ShipHull);
    glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawHangarBay(const NativeBattlefieldFrame& frame) {
    if(!frame.playerPhysics)return;const auto& p=frame.playerPhysics->position;
    DisableSceneLighting();
    DrawBox(p.x,p.y,-0.20f,42.0f,32.0f,0.28f,{0.035f,0.045f,0.052f,1.0f});
    DrawBox(p.x-20.5f,p.y,3.2f,0.8f,32.0f,6.4f,{0.12f,0.15f,0.17f,1.0f});
    DrawBox(p.x+20.5f,p.y,3.2f,0.8f,32.0f,6.4f,{0.12f,0.15f,0.17f,1.0f});
    for(int i=-4;i<=4;++i){
        Line(p.x-18.0f,p.y+i*3.0f,-0.04f,p.x+18.0f,p.y+i*3.0f,-0.04f,{0.18f,0.42f,0.44f,0.20f},1.0f);
        DrawBox(p.x+(i%2?17.8f:-17.8f),p.y+i*2.7f,0.18f,0.22f,0.55f,0.14f,{0.92f,0.62f,0.16f,1.0f});
    }
    SetupSceneLighting();
}

void DrawInteriorCutaway(const PhysicsComponent& p,float yaw) {
    glPushMatrix();
    glTranslatef(p.position.x,p.position.y,0.56f); glRotatef(yaw*180.0f/kPi,0,0,1);
    DrawBox(0.0f,0.0f,0.0f,2.25f,3.75f,0.10f,{0.08f,0.13f,0.16f,0.95f});
    DrawBox(0.0f,1.18f,0.22f,1.62f,1.05f,0.34f,{0.16f,0.46f,0.58f,0.88f});
    DrawBox(-0.62f,-0.35f,0.22f,0.92f,1.34f,0.34f,{0.70f,0.42f,0.14f,0.88f});
    DrawBox(0.62f,-0.35f,0.22f,0.92f,1.34f,0.34f,{0.24f,0.50f,0.35f,0.88f});
    DrawBox(0.0f,-1.42f,0.22f,1.78f,0.70f,0.34f,{0.37f,0.41f,0.44f,0.88f});
    DisableSceneLighting();
    Ring(0,0,0.45f,2.0f,{0.18f,0.78f,0.90f,0.46f},1.0f,44);
    SetupSceneLighting();
    glPopMatrix();
}

// 5x7 uppercase raster font.  Text is geometry emitted by the native OpenGL
// backend, so HUD labels no longer depend on C#, GDI text ownership, or an
// external font file merely to be readable during gameplay.
std::array<std::uint8_t,7> Glyph5x7(char c){
#define G(a,b,c_,d,e,f,g) return {{a,b,c_,d,e,f,g}}
    switch(c){
        case 'A': G(14,17,17,31,17,17,17); case 'B': G(30,17,17,30,17,17,30);
        case 'C': G(14,17,16,16,16,17,14); case 'D': G(30,17,17,17,17,17,30);
        case 'E': G(31,16,16,30,16,16,31); case 'F': G(31,16,16,30,16,16,16);
        case 'G': G(14,17,16,23,17,17,15); case 'H': G(17,17,17,31,17,17,17);
        case 'I': G(14,4,4,4,4,4,14); case 'J': G(7,2,2,2,18,18,12);
        case 'K': G(17,18,20,24,20,18,17); case 'L': G(16,16,16,16,16,16,31);
        case 'M': G(17,27,21,21,17,17,17); case 'N': G(17,25,21,19,17,17,17);
        case 'O': G(14,17,17,17,17,17,14); case 'P': G(30,17,17,30,16,16,16);
        case 'Q': G(14,17,17,17,21,18,13); case 'R': G(30,17,17,30,20,18,17);
        case 'S': G(15,16,16,14,1,1,30); case 'T': G(31,4,4,4,4,4,4);
        case 'U': G(17,17,17,17,17,17,14); case 'V': G(17,17,17,17,17,10,4);
        case 'W': G(17,17,17,21,21,21,10); case 'X': G(17,17,10,4,10,17,17);
        case 'Y': G(17,17,10,4,4,4,4); case 'Z': G(31,1,2,4,8,16,31);
        case '0': G(14,17,19,21,25,17,14); case '1': G(4,12,4,4,4,4,14);
        case '2': G(14,17,1,2,4,8,31); case '3': G(30,1,1,14,1,1,30);
        case '4': G(2,6,10,18,31,2,2); case '5': G(31,16,16,30,1,1,30);
        case '6': G(14,16,16,30,17,17,14); case '7': G(31,1,2,4,8,8,8);
        case '8': G(14,17,17,14,17,17,14); case '9': G(14,17,17,15,1,1,14);
        case '-': G(0,0,0,31,0,0,0); case '.': G(0,0,0,0,0,12,12);
        case ':': G(0,12,12,0,12,12,0); case '/': G(1,2,2,4,8,8,16);
        case '+': G(0,4,4,31,4,4,0); case '[': G(14,8,8,8,8,8,14);
        case ']': G(14,2,2,2,2,2,14); case '_': G(0,0,0,0,0,0,31);
        default: G(0,0,0,0,0,0,0);
    }
#undef G
}

void DrawText5x7(const std::string& raw,float x,float y,float pixel,const Rgba& c){
    // Pass417: all gameplay text has a global readability floor. Legacy callers
    // may still request muted colors/scales, but the renderer never emits tiny,
    // low-alpha text against the starfield anymore.
    Rgba readable=c;
    readable.a=std::max(readable.a,0.84f);
    const float luminance=readable.r*.2126f+readable.g*.7152f+readable.b*.0722f;
    if(luminance<.58f){
        const float t=std::clamp((.58f-luminance)/.58f,0.0f,.58f);
        readable.r=readable.r+(1.0f-readable.r)*t;
        readable.g=readable.g+(1.0f-readable.g)*t;
        readable.b=readable.b+(1.0f-readable.b)*t;
    }
    pixel=std::max(pixel,.62f); // native font maps this to the 14 px floor.
    if(DrawNativeUiText(raw,x,y,pixel,readable.r,readable.g,readable.b,readable.a))return;
    const std::string text=ToUpperAscii(raw);
    pixel=std::max(pixel,1.25f);
    float cursor=x;
    for(char ch:text){
        if(ch==' '){cursor+=pixel*4.0f;continue;}
        const auto rows=Glyph5x7(ch);
        for(int row=0;row<7;++row){
            for(int col=0;col<5;++col){
                if(rows[row]&(1u<<(4-col))) FilledRect(cursor+col*pixel,y+row*pixel,0,pixel,pixel,readable);
            }
        }
        cursor+=pixel*6.0f;
    }
}

std::string ContactKindName(NativeContactKind kind){
    switch(kind){
        case NativeContactKind::Ship:return "SHIP"; case NativeContactKind::Planet:return "PLANET";
        case NativeContactKind::Station:return "STATION"; case NativeContactKind::Derelict:return "DERELICT";
        case NativeContactKind::OrbitalHub:return "ORBITAL HUB"; case NativeContactKind::Asteroid:return "ASTEROID";
        case NativeContactKind::Site:return "POINT OF INTEREST"; default:return "CONTACT";
    }
}

Vector3 ContactWorld(const NativeBattlefieldFrame& frame,const NativeContactSelection& s){
    if(!frame.sector)return {};
    switch(s.kind){
        case NativeContactKind::Ship: if(s.index<frame.sector->ships.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->ships[s.index].position); break;
        case NativeContactKind::Planet: if(s.index<frame.sector->planets.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->planets[s.index].position); break;
        case NativeContactKind::Station:
            if(s.index==0 && frame.sector->hasStation)return NativeBattlefieldRenderer::SectorToWorld(frame.sector->station.position);
            if(frame.stationContacts && s.index>0 && s.index-1<frame.stationContacts->size())return (*frame.stationContacts)[s.index-1].position;
            break;
        case NativeContactKind::Derelict: if(s.index<frame.sector->derelicts.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->derelicts[s.index].position); break;
        case NativeContactKind::OrbitalHub: if(s.index<frame.sector->orbitalHubs.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->orbitalHubs[s.index].position); break;
        case NativeContactKind::Asteroid: if(s.index<frame.sector->asteroids.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->asteroids[s.index].position); break;
        case NativeContactKind::Site: if(s.index<frame.sector->pointsOfInterest.size())return NativeBattlefieldRenderer::SectorToWorld(frame.sector->pointsOfInterest[s.index].position); break;
        default: break;
    }
    return {};
}

void DrawScannerContact(float mx,float my,const Vector3& player,const Vector3& contact,float range,const Rgba& color,float size){
    const float dx=contact.x-player.x,dy=contact.y-player.y;
    const float d=std::sqrt(dx*dx+dy*dy);
    if(d>range)return;
    const float scale=82.0f/range;
    FilledCircle(mx+dx*scale,my-dy*scale,0,size,color,12);
}

void DrawWorldLabels(const NativeBattlefieldFrame& frame){
    DisableShader();
    SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    if(frame.camera->GetZoom()>=0.55f){
        for(const auto& planet:frame.sector->planets){
            const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(planet.position);
            auto sp=NativeBattlefieldRenderer::WorldToScreen({p.x,p.y,0.0f},frame.viewportWidth,frame.viewportHeight,*frame.camera);
            if(sp.visible) DrawText5x7(planet.name,sp.x+10.0f,sp.y-12.0f,1.35f,{0.62f,0.78f,0.84f,0.65f});
        }
        if(frame.sector->hasStar){
            const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(frame.sector->star.position);
            auto sp=NativeBattlefieldRenderer::WorldToScreen({p.x,p.y,-5.0f},frame.viewportWidth,frame.viewportHeight,*frame.camera);
            if(sp.visible)DrawText5x7(frame.sector->star.name,sp.x+14.0f,sp.y-14.0f,1.35f,{0.94f,0.78f,0.42f,0.66f});
        }
        for(const auto& site:frame.sector->pointsOfInterest){
            const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(site.position);
            auto sp=NativeBattlefieldRenderer::WorldToScreen({p.x,p.y,0.42f},frame.viewportWidth,frame.viewportHeight,*frame.camera);
            if(sp.visible)DrawText5x7(site.name,sp.x+9.0f,sp.y-8.0f,0.95f,{0.50f,0.72f,0.74f,0.50f});
        }
        if(frame.sector->hasStation){
            const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(frame.sector->station.position);
            auto sp=NativeBattlefieldRenderer::WorldToScreen({p.x,p.y,1.0f},frame.viewportWidth,frame.viewportHeight,*frame.camera);
            if(sp.visible) DrawText5x7(frame.sector->station.name,sp.x+12.0f,sp.y-6.0f,1.25f,{0.42f,0.82f,0.88f,0.72f});
        }
        if(frame.stationContacts){
            for(const auto& station:*frame.stationContacts){
                auto sp=NativeBattlefieldRenderer::WorldToScreen({station.position.x,station.position.y,1.0f},frame.viewportWidth,frame.viewportHeight,*frame.camera);
                if(sp.visible)DrawText5x7(station.name,sp.x+10.0f,sp.y-6.0f,1.02f,station.asteroidEmbedded?Rgba{0.82f,0.62f,0.34f,0.72f}:Rgba{0.38f,0.84f,0.78f,0.72f});
            }
        }
    }
    glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST);
}

void DrawQueuedUI(const NativeBattlefieldFrame& frame){
    if(!frame.uiRenderer)return;
    DisableShader();
    SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    for(const auto& cmd:frame.uiRenderer->GetCommands()){
        const Rgba c{cmd.color.r,cmd.color.g,cmd.color.b,cmd.color.a};
        switch(cmd.type){
            case DrawCommandType::FilledRect: FilledRect(cmd.rect.x,cmd.rect.y,0,cmd.rect.width,cmd.rect.height,c); break;
            case DrawCommandType::OutlineRect:
                glLineWidth(cmd.lineWidth);Color(c);glBegin(GL_LINE_LOOP);glVertex2f(cmd.rect.x,cmd.rect.y);glVertex2f(cmd.rect.x+cmd.rect.width,cmd.rect.y);glVertex2f(cmd.rect.x+cmd.rect.width,cmd.rect.y+cmd.rect.height);glVertex2f(cmd.rect.x,cmd.rect.y+cmd.rect.height);glEnd();break;
            case DrawCommandType::Line: Line(cmd.p1.x,cmd.p1.y,0,cmd.p2.x,cmd.p2.y,0,c,cmd.lineWidth);break;
            case DrawCommandType::Circle: Ring(cmd.p1.x,cmd.p1.y,0,cmd.p2.x,c,cmd.lineWidth,40);break;
            case DrawCommandType::FilledCircle: FilledCircle(cmd.p1.x,cmd.p1.y,0,cmd.p2.x,c,40);break;
            case DrawCommandType::Text: DrawText5x7(cmd.text,cmd.p1.x,cmd.p1.y,std::max(1.0f,cmd.fontSize/9.0f),c);break;
        }
    }
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

std::string StarterCareerName(StarterCareer career){
    switch(career){
        case StarterCareer::Prospector:return "PROSPECTOR";
        case StarterCareer::Scrapper:return "SCRAPPER";
        case StarterCareer::Pathfinder:return "PATHFINDER";
        case StarterCareer::Defender:return "DEFENDER";
        case StarterCareer::Custom:return "CUSTOM";
    }
    return "PROSPECTOR";
}

void DrawFrontend(const NativeBattlefieldFrame& frame){
    DisableShader();
    SetupScreenProjection(frame.viewportWidth,frame.viewportHeight);
    glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight);
    FilledRect(0,0,0,w,h,{0.006f,0.012f,0.022f,1.0f});
    for(int i=0;i<48;++i){float x=std::fmod(73.0f*i+41.0f,w);float y=std::fmod(47.0f*i+19.0f,h);FilledCircle(x,y,0,(i%5==0)?1.4f:.8f,{0.34f,0.48f,0.60f,0.28f},8);}

    DrawText5x7("SUBSPACE",w*.5f-125,h*.14f,4.5f,{0.72f,0.86f,0.92f,0.96f});
    DrawText5x7("PVE CO-OP INDUSTRIAL SPACE SANDBOX",w*.5f-205,h*.14f+48,1.35f,{0.42f,0.62f,0.70f,0.72f});

    const float centerX=w*.5f;
    const auto controls=BuildFrontendControls(frame.frontendScreen,frame.viewportWidth,frame.viewportHeight);
    const auto drawControl=[&](const FrontendControl& control){
        const bool hovered=control.command==frame.frontendHovered;
        const bool selected=control.command==frame.frontendSelected;
        const auto& r=control.bounds;
        Rgba fill={0.018f,0.045f,0.058f,0.90f};
        Rgba border={0.12f,0.34f,0.40f,0.54f};
        Rgba text={0.66f,0.80f,0.84f,0.92f};
        if(control.primary){fill={0.045f,0.095f,0.110f,0.94f};text={0.92f,0.78f,0.38f,0.98f};}
        if(selected){fill={0.075f,0.20f,0.24f,0.98f};border={0.22f,0.72f,0.80f,0.88f};text={0.94f,0.98f,0.98f,1.0f};}
        if(hovered){fill={0.10f,0.25f,0.29f,0.99f};border={0.34f,0.86f,0.90f,0.96f};text={1.0f,0.86f,0.46f,1.0f};}
        FilledRect(r.x,r.y,0,r.width,r.height,fill);
        Line(r.x,r.y,0,r.x+r.width,r.y,0,border,1.0f);
        Line(r.x,r.y+r.height,0,r.x+r.width,r.y+r.height,0,border,1.0f);
        Line(r.x,r.y,0,r.x,r.y+r.height,0,border,1.0f);
        Line(r.x+r.width,r.y,0,r.x+r.width,r.y+r.height,0,border,1.0f);
        DrawText5x7(control.label,r.x+16.0f,r.y+12.0f,1.05f,text);
    };

    switch(frame.frontendScreen){
        case FrontendScreen::MainMenu:
            DrawText5x7("MAIN MENU",centerX-72,h*.31f,1.55f,{0.70f,0.86f,0.90f,0.92f});
            break;
        case FrontendScreen::NewSandbox: {
            DrawText5x7("NEW SANDBOX",centerX-96,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7("COMMANDER",centerX-220,h*.37f,1.0f,{0.46f,0.64f,0.70f,0.72f});
            DrawText5x7(frame.frontendConfig.commanderName,centerX-78,h*.37f,1.08f,{0.76f,0.88f,0.90f,0.94f});
            DrawText5x7("CORPORATION",centerX-220,h*.405f,1.0f,{0.46f,0.64f,0.70f,0.72f});
            DrawText5x7(frame.frontendConfig.corporationName,centerX-78,h*.405f,1.0f,{0.76f,0.88f,0.90f,0.94f});
            break;
        }
        case FrontendScreen::StartingShip:
            DrawText5x7("STARTING SHIP",centerX-100,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7(StarterCareerName(frame.starterCareer),centerX-108,h*.41f,2.0f,{0.74f,0.90f,0.92f,0.98f});
            DrawText5x7("SELECT A CERTIFIED PREBUILT STARTER",centerX-196,h*.445f,1.0f,{0.50f,0.70f,0.76f,0.78f});
            break;
        case FrontendScreen::StationHangar:
            DrawText5x7("STATION HANGAR",centerX-110,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7("STARTER SHIP CERTIFIED",centerX-130,h*.41f,1.25f,{0.66f,0.84f,0.78f,0.92f});
            DrawText5x7("FITTING  CARGO  MARKET  PERSONNEL  SHIPYARD",centerX-232,h*.455f,.90f,{0.52f,0.68f,0.72f,0.72f});
            break;
        case FrontendScreen::LoadSandbox:
            DrawText5x7("LOAD SANDBOX",centerX-102,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7("NO SAVE BROWSER IS WIRED TO THE NATIVE FRONTEND YET",centerX-260,h*.46f,.92f,{0.62f,0.74f,0.78f,0.82f});
            DrawText5x7("THIS SCREEN NOW REMAINS INTERACTIVE INSTEAD OF FALLING THROUGH",centerX-300,h*.50f,.78f,{0.42f,0.60f,0.66f,0.68f});
            break;
        case FrontendScreen::Settings:
            DrawText5x7("SETTINGS",centerX-70,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7("NATIVE RUNTIME SETTINGS PANEL",centerX-164,h*.46f,1.0f,{0.62f,0.76f,0.80f,0.84f});
            DrawText5x7("GRAPHICS / AUDIO / GAMEPLAY CONTROLS ARE THE NEXT UI SURFACE",centerX-292,h*.50f,.78f,{0.42f,0.60f,0.66f,0.68f});
            break;
        case FrontendScreen::Credits:
            DrawText5x7("CREDITS",centerX-64,h*.31f,1.75f,{0.96f,0.68f,0.22f,0.98f});
            DrawText5x7("CODENAME SUBSPACE",centerX-112,h*.44f,1.25f,{0.72f,0.86f,0.90f,0.90f});
            DrawText5x7("THIRD-PARTY ASSET ATTRIBUTION IS RETAINED IN PROJECT PROVENANCE",centerX-310,h*.49f,.78f,{0.46f,0.64f,0.70f,0.70f});
            break;
        default:
            break;
    }

    for(const auto& control:controls) drawControl(control);

    if(frame.frontendScreen==FrontendScreen::NewSandbox){
        const float left=w*.5f-220.0f;
        const float y=h*.47f;
        DrawText5x7(frame.frontendConfig.storyEnabled?"ENABLED":"DISABLED",left+310.0f,y+12.0f,1.0f,frame.frontendConfig.storyEnabled?Rgba{0.42f,0.88f,0.70f,0.98f}:Rgba{0.88f,0.46f,0.38f,0.92f});
        DrawText5x7(frame.frontendConfig.coopEnabled?"ENABLED":"DISABLED",left+310.0f,y+60.0f,1.0f,frame.frontendConfig.coopEnabled?Rgba{0.42f,0.88f,0.70f,0.98f}:Rgba{0.88f,0.46f,0.38f,0.92f});
        FilledRect(left+94.0f,y+96.0f,0,252.0f,38.0f,{0.010f,0.032f,0.044f,0.94f});
        DrawText5x7("GALAXY SEED",left+112.0f,y+108.0f,.90f,{0.52f,0.70f,0.74f,0.80f});
        DrawText5x7(std::to_string(frame.frontendConfig.galaxySeed),left+234.0f,y+108.0f,.90f,{0.90f,0.94f,0.94f,0.96f});
    }

    DrawText5x7("MOUSE  /  UP DOWN  /  ENTER SELECT  /  ESC BACK",centerX-242,h-34,.82f,{0.46f,0.66f,0.72f,0.72f});
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

void DrawContextMenuOverlay(const NativeBattlefieldFrame& frame){
    if(!frame.contextMenu.open||frame.contextMenu.actions.empty())return;
    const int w=frame.viewportWidth,h=frame.viewportHeight;
    DisableShader();SetupScreenProjection(w,h);glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    PlayerFacingIntegrationSystem uiAuthority;
    const auto menu=uiAuthority.LayoutContextMenu(frame.contextMenu,w,h);
    FilledRect(menu.x,menu.y,0,menu.width,menu.height,{0.004f,0.018f,0.028f,0.985f});
    Line(menu.x,menu.y,0,menu.x+menu.width,menu.y,0,{0.18f,0.70f,0.76f,0.90f},2.0f);
    DrawText5x7(frame.contextMenu.systemMapTarget?"DESTINATION ACTIONS":"CONTEXT ACTIONS",menu.x+16,menu.y+12,menu.textScale*.88f,{0.62f,0.82f,0.86f,0.94f});
    for(std::size_t i=0;i<frame.contextMenu.actions.size();++i){
        const float rowY=menu.y+menu.headerHeight+i*menu.rowHeight;
        if(i==frame.contextMenu.selected)FilledRect(menu.x+7,rowY+3,0,menu.width-14,menu.rowHeight-6,{0.10f,0.28f,0.32f,0.94f});
        const auto& a=frame.contextMenu.actions[i];
        DrawText5x7(a.label,menu.x+18,rowY+9,menu.textScale,a.enabled?Rgba{0.82f,0.94f,0.95f,0.98f}:Rgba{0.45f,0.50f,0.52f,0.68f});
    }
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

void DrawDockingGuidance(const NativeBattlefieldFrame& frame){
    if(!frame.dockingState||frame.dockingStage==DockingExperienceStage::Undocked||frame.dockingStage==DockingExperienceStage::Docked)return;
    const auto& d=frame.dockingState->geometry;if(d.stationId==0)return;
    const auto lights=StationNavigationLightSystem{}.Build(d,frame.dockingState->trafficClearance,false);
    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    Ring(d.stationWorld.x,d.stationWorld.y,.12f,d.dockingEnvelopeRadius,{.08f,.58f,.70f,.12f},1.2f,72);
    for(const auto& wp:d.corridor)Ring(wp.position.x,wp.position.y,.18f,wp.radius,{.18f,.72f,.86f,.18f},1.1f,32);
    for(const auto& l:lights){float pulse=StationNavigationLightSystem::Pulse(l,frame.elapsedSeconds);Rgba c{.16f,.88f,.96f,.35f+.55f*pulse};if(l.kind==DockNavLightKind::Hold)c={1.0f,.62f,.12f,.35f+.55f*pulse};else if(l.kind==DockNavLightKind::Closed)c={1.0f,.16f,.10f,.45f+.45f*pulse};else if(l.kind==DockNavLightKind::Aperture)c={.36f,1.0f,.62f,.45f+.50f*pulse};FilledCircle(l.position.x,l.position.y,.52f,.12f,c,12);}
    Line(d.captureWorld.x,d.captureWorld.y,.24f,d.apertureWorld.x,d.apertureWorld.y,.24f,{.42f,1.0f,.68f,.70f},2.0f);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawObservableWarpEvidence(const NativeBattlefieldFrame& frame){
    if(!frame.observableWarpEvents)return;DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    for(const auto& e:*frame.observableWarpEvents){const float life=1.0f-std::clamp(e.ageSeconds/std::max(.01f,e.lifetimeSeconds),0.0f,1.0f);const float a=e.intensity*life;const Vector3 end=e.origin+e.direction*(4.0f+22.0f*(1.0f-life));switch(e.kind){case WarpEvidenceKind::DepartureStreak:case WarpEvidenceKind::TransitWake:Line(e.origin.x,e.origin.y,.50f,end.x,end.y,.50f,{.18f,.72f,1.0f,.62f*a},2.0f+5.0f*a);break;case WarpEvidenceKind::CollapseRing:Ring(e.origin.x,e.origin.y,.42f,2.0f+8.0f*(1.0f-life),{.22f,.62f,1.0f,.48f*a},2.0f,48);break;case WarpEvidenceKind::ArrivalFlare:FilledCircle(e.origin.x,e.origin.y,.52f,.35f+2.2f*(1.0f-life),{.58f,.88f,1.0f,.40f*a},28);break;default:Ring(e.origin.x,e.origin.y,.42f,1.0f+2.0f*(1.0f-life),{.32f,.76f,1.0f,.28f*a},1.4f,36);break;}}
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDisable(GL_BLEND);SetupSceneLighting();
}

void DrawHud(const NativeBattlefieldFrame& frame,const NativeBattlefieldRenderer::VisualAssets* assets){
    const int w=frame.viewportWidth,h=frame.viewportHeight;
    DisableShader();
    SetupScreenProjection(w,h);
    glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const Rgba cyan{0.18f,0.78f,0.88f,0.85f};
    const Rgba text{0.70f,0.84f,0.88f,0.90f};
    const float cx=w*0.5f,cy=h*0.5f;

    // Pass311-315: one coherent forward-facing HUD language. Interior and
    // hangar states intentionally do not inherit the flight telemetry layout.
    FilledRect(0,0,0,static_cast<float>(w),54,{0.004f,0.014f,0.024f,0.90f});
    Line(0,54,0,static_cast<float>(w),54,0,{0.12f,0.48f,0.56f,0.42f},1.0f);
    DrawText5x7(frame.flightHud.modeLabel.empty()?frame.productionHud.modeLabel:frame.flightHud.modeLabel,24,18,1.35f,{0.82f,0.92f,0.95f,0.98f});
    // Pass418: original Subspace Command Rail. Functionally inspired by compact
    // space-sim launch rails, but uses Subspace naming, layout and visual tokens.
    const auto& rail=frame.commandRail;
    FilledRect(0,54,0,rail.width,static_cast<float>(h)-54,{0.004f,0.014f,0.022f,0.94f});
    Line(rail.width,54,0,rail.width,static_cast<float>(h),0,{0.12f,0.44f,0.52f,0.58f},1.0f);
    for(std::size_t i=0;i<rail.items.size();++i){
        const float ry=rail.top+static_cast<float>(i)*rail.rowHeight;const auto& item=rail.items[i];
        if(item.active)FilledRect(6,ry+4,0,rail.width-12,rail.rowHeight-8,{0.08f,0.24f,0.30f,0.92f});
        else FilledRect(8,ry+6,0,rail.width-16,rail.rowHeight-12,{0.018f,0.045f,0.058f,0.78f});
        DrawText5x7(item.shortLabel.empty()?item.label:item.shortLabel,18,ry+15,.78f,item.active?Rgba{0.92f,0.98f,0.98f,1.0f}:Rgba{0.70f,0.84f,0.86f,0.96f});
    }
    if(frame.dockingStage!=DockingExperienceStage::Docked){
        DrawText5x7(frame.strategicFlightMode?"STRATEGIC FLIGHT":"MANUAL FLIGHT",w-302,20,1.05f,frame.strategicFlightMode?Rgba{0.30f,0.82f,0.72f,0.90f}:Rgba{0.90f,0.64f,0.24f,0.90f});
        DrawText5x7("TAB MODE   SHIFT BOOST",w-302,40,.76f,{0.42f,0.62f,0.68f,0.66f});
    }
    if(frame.embodimentMode==ShipEmbodimentMode::InteriorOnFoot){
        FilledRect(cx-176,72,0,352,56,{.006f,.026f,.036f,.90f});
        DrawText5x7("SHIP INTERIOR / ON FOOT",cx-154,84,.92f,{.72f,.90f,.94f,.96f});
        DrawText5x7("WASD MOVE   I RETURN TO CONTROLS",cx-154,106,.68f,{.92f,.64f,.18f,.92f});
    }
    if(frame.dockingState && frame.dockingStage!=DockingExperienceStage::Undocked && frame.dockingStage!=DockingExperienceStage::Docked){
        const auto& d=*frame.dockingState;
        FilledRect(cx-235,136,0,470,66,{.006f,.026f,.036f,.90f});
        DrawText5x7(d.guidanceCue.empty()?frame.dockingStatus:d.guidanceCue,cx-214,148,.82f,d.alignment<.58f?Rgba{1.0f,.52f,.16f,.96f}:Rgba{.46f,.94f,.82f,.94f});
        std::ostringstream dg;dg<<"GATE "<<(d.corridorWaypoint+1)<<" / "<<std::max<std::size_t>(1,d.geometry.corridor.size())<<"   RANGE "<<static_cast<int>(d.distanceToGuidance)<<" U   LIMIT "<<static_cast<int>(d.assignedSpeedLimit);
        DrawText5x7(dg.str(),cx-214,172,.68f,{.62f,.80f,.84f,.90f});
    }
    if(frame.embodimentMode==ShipEmbodimentMode::DockedHangar||frame.dockingStage==DockingExperienceStage::Docked){
        FilledRect(94,82,0,286,h-132,{0.006f,0.022f,0.034f,0.88f});
        DrawText5x7(frame.hangarRuntime.title.empty()?"STATION SERVICES":frame.hangarRuntime.title,112,104,1.20f,{0.72f,0.88f,0.92f,0.94f});
        DrawText5x7(frame.hangarRuntime.hangarProfile,112,126,.76f,{0.42f,0.66f,0.72f,0.70f});
        if(!frame.hangarRuntime.serviceActions.empty()){
            int i=0;for(const auto& service:frame.hangarRuntime.serviceActions){if(i>=8)break;DrawText5x7(service,114,154+i*32,1.0f,i==0?Rgba{0.94f,0.64f,0.20f,0.92f}:Rgba{0.52f,0.70f,0.74f,0.78f});++i;}
        }else if(!frame.hangarRuntime.services.empty()){
            int i=0;for(const auto& service:frame.hangarRuntime.services){if(i>=8)break;DrawText5x7(service,114,154+i*32,1.0f,i==0?Rgba{0.94f,0.64f,0.20f,0.92f}:Rgba{0.52f,0.70f,0.74f,0.78f});++i;}
        }else{
            const char* left[]={"MARKET","CONTRACTS","INDUSTRY","SHIPYARD","RESEARCH","REPAIR / REFUEL","STORAGE"};
            for(int i=0;i<7;++i)DrawText5x7(left[i],114,154+i*32,1.0f,i==0?Rgba{0.94f,0.64f,0.20f,0.92f}:Rgba{0.52f,0.70f,0.74f,0.78f});
        }
        FilledRect(w-292,82,0,270,h-132,{0.006f,0.022f,0.034f,0.88f});
        DrawText5x7("SHIP OPERATIONS",w-270,104,1.35f,{0.72f,0.88f,0.92f,0.94f});
        const char* right[]={"FITTING","CARGO","DRONES","CREW","ENGINEERING","SHIP BUILDER","FLEET","SHIP INFO"};
        for(int i=0;i<8;++i)DrawText5x7(right[i],w-268,144+i*34,1.02f,i==0?Rgba{0.94f,0.64f,0.20f,0.92f}:Rgba{0.52f,0.70f,0.74f,0.78f});
        if(frame.workspaceMode==SandboxWorkspaceMode::HangarFitting&&assets){
            const auto* recipe=assets->shipyardReady
                ? ProceduralVisualVariantSystem::SelectSourceFamily(assets->generatedVisualCatalog,"INDUSTRIAL","SHIPYARD_V07_CC0",1u)
                : nullptr;
            if(!recipe) recipe=ProceduralVisualVariantSystem::Select(assets->generatedVisualCatalog,"INDUSTRIAL",1u);
            const float fy=450.0f;DrawText5x7("HARDPOINTS",w-270,fy,1.08f,{0.86f,0.94f,0.95f,0.98f});
            DrawText5x7("Actual generated hull / live sockets",w-270,fy+24,.76f,{0.68f,0.80f,0.83f,0.94f});
            if(recipe){int hi=0;for(const auto& hp:recipe->hardpoints){if(hi>=6)break;const char* sz=hp.size==FittingHardpointSize::Small?"S":hp.size==FittingHardpointSize::Medium?"M":hp.size==FittingHardpointSize::Large?"L":hp.size==FittingHardpointSize::Capital?"XL":"U";DrawText5x7(hp.id+"  ["+sz+"]  EMPTY",w-268,fy+52+hi*28,.90f,{0.78f,0.88f,0.90f,0.96f});++hi;}}
        }
        FilledRect(w*.5f-188,h-70,0,376,42,{0.06f,0.12f,0.15f,0.94f});
        DrawText5x7("Q SELECT   W MOVE   E ROTATE   F FRAME   DEL REMOVE",w*.5f-174,h-62,.74f,{0.92f,0.78f,0.36f,0.98f});
        DrawText5x7("RMB ORBIT   MMB VIEW-PAN   LMB SELECT/DRAG   WHEEL ZOOM   ENTER UNDOCK",w*.5f-174,h-45,.68f,{0.72f,0.84f,0.86f,0.90f});
        glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);return;
    }
    Line(cx-20,cy,0,cx-7,cy,0,cyan,1.2f);Line(cx+7,cy,0,cx+20,cy,0,cyan,1.2f);
    Line(cx,cy-20,0,cx,cy-7,0,cyan,1.2f);Line(cx,cy+7,0,cx,cy+20,0,cyan,1.2f);

    const float speed=frame.playerPhysics?std::sqrt(frame.playerPhysics->velocity.x*frame.playerPhysics->velocity.x+frame.playerPhysics->velocity.y*frame.playerPhysics->velocity.y):0.0f;
    const float speedPct=std::clamp(speed/30.0f,0.0f,1.0f);
    FilledRect(94,h-224,0,310,92,{0.008f,0.026f,0.040f,0.76f});
    Line(22,h-224,0,332,h-224,0,{0.15f,0.58f,0.66f,0.60f});
    DrawText5x7("FLIGHT TELEMETRY",110,h-208,1.35f,text);
    DrawText5x7("SPEED",110,h-178,1.15f,{0.58f,0.72f,0.76f,0.88f});
    FilledRect(168,h-179,0,190,10,{0.04f,0.09f,0.12f,0.95f});FilledRect(168,h-179,0,190*speedPct,10,{0.86f,0.56f,0.16f,0.96f});
    std::ostringstream speedText; speedText << static_cast<int>(speed) << " U/S";
    DrawText5x7(speedText.str(),110,h-154,1.12f,text);
    DrawText5x7(frame.inertialDampening?"DAMP ON":"DAMP OFF",222,h-154,1.12f,frame.inertialDampening?Rgba{0.20f,0.78f,0.68f,0.92f}:Rgba{0.82f,0.32f,0.24f,0.92f});
    DrawText5x7(frame.boostActive?"BOOST":"CRUISE",314,h-154,1.12f,frame.boostActive?Rgba{0.96f,0.62f,0.14f,0.96f}:text);

    const float mx=w-128.0f,my=116.0f;
    Ring(mx,my,0,88,{0.10f,0.50f,0.58f,0.55f},1.2f,64);Ring(mx,my,0,44,{0.10f,0.42f,0.50f,0.28f},1.0f,64);
    Line(mx-88,my,0,mx+88,my,0,{0.10f,0.42f,0.50f,0.22f});Line(mx,my-88,0,mx,my+88,0,{0.10f,0.42f,0.50f,0.22f});
    DrawText5x7("TACTICAL",w-205,20,1.20f,{0.48f,0.72f,0.78f,0.70f});
    if(frame.playerPhysics){
        const Vector3 pp=frame.playerPhysics->position;
        for(const auto& ship:frame.sector->ships)DrawScannerContact(mx,my,pp,NativeBattlefieldRenderer::SectorToWorld(ship.position),32.0f,ship.hostile?Rgba{0.94f,0.28f,0.18f,0.88f}:Rgba{0.32f,0.72f,0.82f,0.78f},2.2f);
        for(const auto& planet:frame.sector->planets)DrawScannerContact(mx,my,pp,NativeBattlefieldRenderer::SectorToWorld(planet.position),32.0f,{0.54f,0.64f,0.68f,0.72f},3.0f);
        if(frame.sector->hasStation)DrawScannerContact(mx,my,pp,NativeBattlefieldRenderer::SectorToWorld(frame.sector->station.position),32.0f,{0.24f,0.86f,0.72f,0.90f},3.0f);
        if(frame.stationContacts)for(const auto& station:*frame.stationContacts)DrawScannerContact(mx,my,pp,station.position,32.0f,station.asteroidEmbedded?Rgba{0.82f,0.58f,0.26f,0.88f}:Rgba{0.24f,0.82f,0.72f,0.86f},3.0f);
        for(const auto& site:frame.sector->pointsOfInterest)DrawScannerContact(mx,my,pp,NativeBattlefieldRenderer::SectorToWorld(site.position),32.0f,SiteColor(site.type),2.4f);
        FilledCircle(mx,my,0,3.2f,{0.92f,0.76f,0.30f,0.95f},12);
    }

    // Pass492-495: Contacts is the persistent EVE-familiar tactical object list.
    // The renderer consumes the same layout authority used by LMB/RMB hit tests,
    // so rows are real tactical controls instead of an inert painted overview.
    if(frame.workspaceMode==SandboxWorkspaceMode::Flight){
        const int visible=std::min<int>(frame.tacticalContacts.maxVisibleRows,static_cast<int>(frame.tacticalContacts.rows.size()));
        const auto cl=TacticalContactsSystem::Layout(w,h,visible);const float px=cl.x,py=cl.y,pw=cl.width,ph=cl.Height(visible);
        FilledRect(px,py,0,pw,ph,{.006f,.024f,.036f,.90f});Line(px,py,0,px+pw,py,0,{.12f,.58f,.68f,.74f},1.4f);
        DrawText5x7("CONTACTS / "+std::string(TacticalContactsSystem::PresetName(frame.tacticalContacts.preset)),px+14,py+9,.92f,{.72f,.90f,.94f,.96f});
        const float tabW=pw/7.0f;for(int ti=0;ti<7;++ti){const auto preset=TacticalContactsSystem::PresetFromIndex(ti);const bool active=preset==frame.tacticalContacts.preset;const float tx=px+ti*tabW;FilledRect(tx,py+cl.titleHeight,0,tabW-1,cl.tabHeight,active?Rgba{.07f,.27f,.33f,.94f}:Rgba{.018f,.055f,.070f,.90f});DrawText5x7(TacticalContactsSystem::PresetShortName(preset),tx+8,py+cl.titleHeight+6,.60f,active?Rgba{.88f,.97f,.98f,.98f}:Rgba{.48f,.68f,.72f,.82f});}
        const float columnsY=py+cl.titleHeight+cl.tabHeight+6;DrawText5x7("NAME / TYPE              RANGE",px+14,columnsY,.66f,{.42f,.66f,.72f,.80f});
        for(int i=0;i<visible;++i){const auto& row=frame.tacticalContacts.rows[static_cast<std::size_t>(i)];const float y=cl.RowsY()+i*cl.rowHeight;if(row.selected)FilledRect(px+6,y,0,pw-12,cl.rowHeight-2,{.10f,.34f,.42f,.38f});const Rgba rc=row.threat>=3?Rgba{1.0f,.30f,.18f,.96f}:row.locked?Rgba{1.0f,.68f,.16f,.96f}:Rgba{.66f,.82f,.85f,.88f};std::ostringstream rr;rr<<(row.locked?"L ":"  ")<<row.name.substr(0,19)<<"  "<<static_cast<int>(row.range)<<" U";DrawText5x7(rr.str(),px+12,y+7,.72f,rc);}
        if(visible==0)DrawText5x7("NO CONTACTS IN CURRENT FILTER",px+14,cl.RowsY()+10,.66f,{.44f,.62f,.66f,.72f});
    }

    // EVE-familiar locked target cards: selection and target lock remain distinct.
    if(!frame.targeting.targets.empty()){float tx=cx-((float)frame.targeting.targets.size()*118.0f)*.5f;for(const auto& t:frame.targeting.targets){FilledRect(tx,32,0,108,58,{.008f,.026f,.038f,.88f});Line(tx,32,0,tx+108,32,0,t.locked?Rgba{1.0f,.60f,.14f,.90f}:Rgba{.18f,.58f,.70f,.70f},1.6f);DrawText5x7(t.contact.id.substr(0,14),tx+8,44,.72f,{.78f,.90f,.92f,.94f});const float prog=std::clamp(t.lockProgress,0.0f,1.0f);FilledRect(tx+8,68,0,92,6,{.04f,.08f,.10f,.96f});FilledRect(tx+8,68,0,92*prog,6,t.locked?Rgba{.92f,.58f,.14f,.96f}:Rgba{.18f,.68f,.80f,.90f});tx+=118.0f;}}

    // Command Board replaces the old text-only module strip with a compact ship
    // state/power/speed/module surface.
    {
        const auto& ch=frame.commandHud;const float bx=cx-300.0f,by=h-142.0f;
        FilledRect(bx,by,0,600,112,{.005f,.020f,.032f,.91f});
        DrawText5x7("SHIP COMMAND",bx+18,by+12,.88f,{.66f,.86f,.90f,.92f});
        const Rgba integrity=ch.critical?Rgba{1.0f,.24f,.14f,.98f}:Rgba{.42f,.78f,.72f,.90f};
        DrawText5x7(ch.integrityStatus,bx+112,by+12,.68f,integrity);
        const float coreX=cx,coreY=by+48;
        Ring(coreX,coreY,0,24,{.08f,.28f,.34f,.70f},3.0f,40);
        Ring(coreX,coreY,0,20*std::clamp(ch.power,.0f,1.0f),ch.power<.20f?Rgba{1.0f,.30f,.14f,.92f}:Rgba{.16f,.70f,.78f,.78f},3.0f,40);
        FilledCircle(coreX,coreY,0,14,{.05f,.16f,.20f,.94f},28);DrawText5x7("PWR",coreX-13,coreY-6,.65f,{.80f,.92f,.94f,.94f});
        const float gauges[3]={ch.shield,ch.armor,ch.hull};const char* labels[3]={"S","A","H"};
        for(int i=0;i<3;++i){const float gx=coreX-92+i*92;const float v=std::clamp(gauges[i],0.0f,1.0f);const Rgba gc=v<.25f?Rgba{1.0f,.24f,.14f,.96f}:(v<.50f?Rgba{1.0f,.62f,.14f,.94f}:Rgba{.18f,.72f,.78f,.90f});DrawText5x7(labels[i],gx,by+24,.65f,gc);FilledRect(gx+12,by+26,0,60,5,{.03f,.07f,.09f,.96f});FilledRect(gx+12,by+26,0,60*v,5,gc);}
        const float speedPct=std::clamp(ch.speed/std::max(1.0f,ch.maximumSpeed),0.0f,1.0f);FilledRect(bx+18,by+89,0,150,7,{.03f,.07f,.09f,.96f});FilledRect(bx+18,by+89,0,150*speedPct,7,{.92f,.58f,.16f,.92f});std::ostringstream sv;sv<<static_cast<int>(ch.speed)<<" U/S";DrawText5x7(sv.str(),bx+18,by+68,.72f,{.74f,.86f,.88f,.92f});
        int mi=0;for(const auto& m:ch.modules){if(mi>=6)break;const float mx=coreX+48+mi*52;FilledRect(mx,by+40,0,44,36,m.active?Rgba{.12f,.40f,.46f,.94f}:Rgba{.035f,.075f,.09f,.96f});DrawText5x7(m.shortcut,mx+6,by+47,.60f,{.92f,.64f,.18f,.94f});++mi;}
    }

    if(frame.miningTelemetry){
        FilledRect(94,h-292,0,380,54,{0.008f,0.026f,0.040f,0.68f});
        std::ostringstream mineText;mineText<<"MINING  FRACTURES "<<frame.miningTelemetry->asteroidsFractured<<"  ORE "<<static_cast<int>(frame.miningTelemetry->oreRecovered)<<"  SALV "<<static_cast<int>(frame.miningTelemetry->salvageRecovered);
        DrawText5x7(mineText.str(),110,h-276,0.92f,{0.58f,0.78f,0.70f,0.84f});
        DrawText5x7("F MINING MISSILE   SPACE COMBAT MISSILE",110,h-254,0.84f,{0.46f,0.62f,0.66f,0.66f});
    }

    if(frame.selection.IsValid()){
        const Vector3 target=ContactWorld(frame,frame.selection);
        const float range=frame.playerPhysics?Distance2D(frame.playerPhysics->position,target):0.0f;
        const float selectedY=212.0f;FilledRect(w-382,selectedY,0,350,132,{0.008f,0.030f,0.042f,0.88f});
        Line(w-382,selectedY,0,w-32,selectedY,0,{0.92f,0.58f,0.16f,0.82f},2.0f);
        DrawText5x7("SELECTED OBJECT / "+std::string(ContactKindName(frame.selection.kind)),w-368,selectedY+13,.78f,{0.94f,0.64f,0.20f,0.96f});
        DrawText5x7(frame.selection.id,w-368,selectedY+37,1.02f,text);
        std::ostringstream rangeText;rangeText<<"RANGE "<<static_cast<int>(range)<<" U";
        DrawText5x7(rangeText.str(),w-368,selectedY+63,.86f,{0.56f,0.74f,0.78f,0.84f});
        float actionX=w-368.0f;int actionCount=0;for(const auto& action:frame.productionHud.contextActions){if(actionCount>=4)break;const float aw=78.0f;FilledRect(actionX,selectedY+88,0,aw-4,26,{.025f,.075f,.090f,.94f});DrawText5x7(action.substr(0,11),actionX+6,selectedY+96,.58f,{.72f,.86f,.88f,.90f});actionX+=aw;++actionCount;}

        auto sp=NativeBattlefieldRenderer::WorldToScreen({target.x,target.y,0.55f},w,h,*frame.camera);
        if(sp.visible){
            const float b=18.0f;
            Line(sp.x-b,sp.y-b,0,sp.x-5,sp.y-b,0,{1.0f,0.65f,0.18f,0.9f},2);Line(sp.x-b,sp.y-b,0,sp.x-b,sp.y-5,0,{1.0f,0.65f,0.18f,0.9f},2);
            Line(sp.x+b,sp.y-b,0,sp.x+5,sp.y-b,0,{1.0f,0.65f,0.18f,0.9f},2);Line(sp.x+b,sp.y-b,0,sp.x+b,sp.y-5,0,{1.0f,0.65f,0.18f,0.9f},2);
            Line(sp.x-b,sp.y+b,0,sp.x-5,sp.y+b,0,{1.0f,0.65f,0.18f,0.9f},2);Line(sp.x-b,sp.y+b,0,sp.x-b,sp.y+5,0,{1.0f,0.65f,0.18f,0.9f},2);
            Line(sp.x+b,sp.y+b,0,sp.x+5,sp.y+b,0,{1.0f,0.65f,0.18f,0.9f},2);Line(sp.x+b,sp.y+b,0,sp.x+b,sp.y+5,0,{1.0f,0.65f,0.18f,0.9f},2);
        }
    }
    // Context actions replace most permanent hotkey noise while shortcuts remain.
    if(!frame.selection.IsValid()&&!frame.productionHud.contextTitle.empty()){
        float panelH=52.0f+static_cast<float>(frame.productionHud.contextActions.size())*24.0f;
        FilledRect(w-382,352,0,350,panelH,{0.006f,0.026f,0.038f,0.88f});
        DrawText5x7(frame.productionHud.contextTitle,w-360,370,1.10f,{0.90f,0.66f,0.24f,0.94f});
        float ay=400;for(const auto& action:frame.productionHud.contextActions){DrawText5x7(action,w-356,ay,0.90f,{0.58f,0.76f,0.80f,0.84f});ay+=24;}
    }
    if(false && (frame.productionHud.showModuleRack||!frame.flightHud.moduleSlots.empty())){
        const int slots=std::max(1,static_cast<int>(frame.flightHud.moduleSlots.size()));
        const float rackW=std::min(620.0f,30.0f+slots*82.0f),rackX=(w-rackW)*0.5f;
        FilledRect(rackX,h-78,0,rackW,50,{0.006f,0.022f,0.034f,0.88f});
        for(int i=0;i<slots&&i<8;++i){const float x=rackX+14+i*82;FilledRect(x,h-68,0,70,30,{0.035f,0.075f,0.092f,0.96f});DrawText5x7(std::to_string(i+1),x+6,h-58,0.72f,{0.90f,0.66f,0.22f,0.84f});if(i<static_cast<int>(frame.flightHud.moduleSlots.size()))DrawText5x7(frame.flightHud.moduleSlots[i],x+20,h-58,0.62f,{0.62f,0.78f,0.82f,0.80f});}
    }
    // Pass401: chat stays anchored bottom-left instead of competing with center flight content.
    if(frame.flightHud.chatBottomLeft){
        FilledRect(94,h-116,0,350,84,{0.006f,0.020f,0.030f,0.82f});
        DrawText5x7("COMMS  LOCAL",110,h-101,.92f,{0.50f,0.78f,0.84f,0.86f});
        DrawText5x7("SYSTEM  Flight channel ready",110,h-78,.76f,{0.46f,0.64f,0.68f,0.72f});
        DrawText5x7("FLEET   Wing telemetry linked",110,h-57,.76f,{0.46f,0.64f,0.68f,0.72f});
    }
    float sy=18.0f;for(const auto& line:frame.productionHud.statusLines){DrawText5x7(line,210,sy,0.80f,{0.42f,0.62f,0.68f,0.62f});sy+=16.0f;}
    if(frame.workspaceMode==SandboxWorkspaceMode::Flight)DrawContextMenuOverlay(frame);

    // Pass356 cinematic arrival: the destination title lives in the world
    // transition rather than popping into a corner UI after the camera resets.
    if(!frame.arrivalTitle.empty()&&frame.arrivalTitleAlpha>0.001f){
        const float pixel=2.35f;
        const float approxW=static_cast<float>(frame.arrivalTitle.size())*6.0f*pixel;
        const float tx=std::max(24.0f,cx-approxW*.5f);
        DrawText5x7(frame.arrivalTitle,tx,h*.43f,pixel,{0.82f,0.92f,0.96f,frame.arrivalTitleAlpha*.94f});
        const std::string subtitle="LOCAL SPACE";
        DrawText5x7(subtitle,cx-66.0f,h*.43f+38.0f,1.05f,{0.42f,0.68f,0.76f,frame.arrivalTitleAlpha*.68f});
    }
    if(frame.shipInspection){
        DrawText5x7("SHIP QA OVERLAY",cx-82.0f,72.0f,1.25f,{0.92f,0.72f,0.30f,0.96f});
        DrawText5x7("F6 TOGGLE   RMB ORBIT   MMB PAN   WHEEL HULL-CLOSE ZOOM",cx-255.0f,96.0f,.86f,{0.66f,0.82f,0.84f,0.94f});
        DrawText5x7("GOLD = MOUNT PATH   CYAN = FUNCTIONAL ANCHOR",cx-210.0f,118.0f,.76f,{0.68f,0.82f,0.82f,0.90f});
    }
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}

Rgba SystemMapNodeColor(SystemMapNodeKind kind) {
    switch(kind){
        case SystemMapNodeKind::Star:return {1.0f,0.72f,0.24f,0.96f};
        case SystemMapNodeKind::Planet:return {0.40f,0.72f,0.92f,0.92f};
        case SystemMapNodeKind::Moon:return {0.66f,0.74f,0.78f,0.90f};
        case SystemMapNodeKind::Station:return {0.22f,0.86f,0.68f,0.92f};
        case SystemMapNodeKind::OrbitalHub:return {0.36f,0.88f,0.82f,0.92f};
        case SystemMapNodeKind::Belt:return {0.68f,0.66f,0.54f,0.84f};
        case SystemMapNodeKind::Salvage:return {0.88f,0.48f,0.20f,0.86f};
        case SystemMapNodeKind::Signature:return {0.66f,0.38f,0.94f,0.88f};
        case SystemMapNodeKind::TradeLane:return {0.30f,0.82f,0.56f,0.82f};
        default:return {0.50f,0.62f,0.66f,0.72f};
    }
}

Rgba OrbitalBodyColor(OrbitalBodyKind kind) {
    switch(kind){
        case OrbitalBodyKind::Star:return {1.0f,0.72f,0.24f,0.98f};
        case OrbitalBodyKind::Planet:return {0.40f,0.72f,0.92f,0.94f};
        case OrbitalBodyKind::Moon:return {0.64f,0.72f,0.76f,0.86f};
        case OrbitalBodyKind::Station:return {0.22f,0.88f,0.70f,0.96f};
        case OrbitalBodyKind::AsteroidStation:return {0.78f,0.58f,0.30f,0.92f};
        case OrbitalBodyKind::BeltObject:return {0.68f,0.66f,0.54f,0.82f};
        default:return {0.52f,0.62f,0.68f,0.74f};
    }
}

SystemMapNodeKind MapKindForOrbital(OrbitalBodyKind kind){
    switch(kind){
        case OrbitalBodyKind::Star:return SystemMapNodeKind::Star;
        case OrbitalBodyKind::Planet:return SystemMapNodeKind::Planet;
        case OrbitalBodyKind::Moon:return SystemMapNodeKind::Moon;
        case OrbitalBodyKind::Station:case OrbitalBodyKind::AsteroidStation:return SystemMapNodeKind::Station;
        case OrbitalBodyKind::BeltObject:return SystemMapNodeKind::Belt;
        default:return SystemMapNodeKind::DeepSpace;
    }
}

void DrawSystemMapMarker(SystemMapNodeKind kind,float x,float y,const Rgba& color,bool selected,bool hovered){
    const float emphasis=selected?1.0f:(hovered?.82f:.62f);
    Rgba c=color;c.a=std::max(c.a,emphasis);
    switch(kind){
        case SystemMapNodeKind::Star:
            FilledCircle(x,y,0,7.2f,c,24);Ring(x,y,0,11.2f,{c.r,c.g,c.b,c.a*.55f},1.1f,32);
            Line(x-15,y,0,x-10,y,0,c,1.5f);Line(x+10,y,0,x+15,y,0,c,1.5f);Line(x,y-15,0,x,y-10,0,c,1.5f);Line(x,y+10,0,x,y+15,0,c,1.5f);break;
        case SystemMapNodeKind::Planet:
            FilledCircle(x,y,0,5.3f,c,20);Ring(x,y,0,8.3f,{c.r,c.g,c.b,c.a*.42f},1.0f,28);break;
        case SystemMapNodeKind::Moon:
            Ring(x,y,0,4.1f,c,1.5f,20);FilledCircle(x-1.0f,y-.5f,0,1.7f,{c.r,c.g,c.b,c.a*.78f},12);break;
        case SystemMapNodeKind::Station:
            Line(x,y-6,0,x+6,y,0,c,1.8f);Line(x+6,y,0,x,y+6,0,c,1.8f);Line(x,y+6,0,x-6,y,0,c,1.8f);Line(x-6,y,0,x,y-6,0,c,1.8f);FilledCircle(x,y,0,1.8f,c,10);break;
        case SystemMapNodeKind::OrbitalHub:
            Ring(x,y,0,6.0f,c,1.6f,24);Line(x-7,y,0,x+7,y,0,c,1.2f);Line(x,y-7,0,x,y+7,0,c,1.2f);break;
        case SystemMapNodeKind::Belt:
            Ring(x,y,0,9.0f,c,1.4f,32);FilledCircle(x-5,y+1,0,1.2f,c,8);FilledCircle(x+1,y-4,0,1.0f,c,8);FilledCircle(x+5,y+3,0,1.3f,c,8);break;
        case SystemMapNodeKind::Salvage:
            Line(x-5,y-5,0,x+5,y+5,0,c,1.8f);Line(x+5,y-5,0,x-5,y+5,0,c,1.8f);Ring(x,y,0,7.0f,{c.r,c.g,c.b,c.a*.45f},1.0f,20);break;
        case SystemMapNodeKind::Signature:
            Line(x,y-6,0,x+5,y,0,c,1.5f);Line(x+5,y,0,x,y+6,0,c,1.5f);Line(x,y+6,0,x-5,y,0,c,1.5f);Line(x-5,y,0,x,y-6,0,c,1.5f);break;
        case SystemMapNodeKind::TradeLane:
            Line(x-7,y-3,0,x+7,y-3,0,c,1.3f);Line(x-7,y+3,0,x+7,y+3,0,c,1.3f);break;
        default:Ring(x,y,0,3.5f,c,1.2f,16);break;
    }
    if(selected)Ring(x,y,0,16.0f,{1.0f,.70f,.20f,.94f},2.0f,36);
    else if(hovered)Ring(x,y,0,13.0f,{.48f,.88f,.92f,.82f},1.5f,32);
}

void DrawLocalRingEnvironment(const NativeBattlefieldFrame& frame) {
    if(!frame.playerPhysics||!frame.sector)return;
    CelestialEnvironmentSystem celestial;
    const PlanetData* nearest=nullptr;Vector3 nearestWorld{};float nearestDistance=1e30f;
    for(const auto& planet:frame.sector->planets){
        if(!planet.hasRings&&planet.type!=PlanetType::GasGiant)continue;
        const Vector3 p=NativeBattlefieldRenderer::SectorToWorld(planet.position);
        const float d=Distance2D(frame.playerPhysics->position,p);
        if(d<nearestDistance){nearestDistance=d;nearest=&planet;nearestWorld=p;}
    }
    if(!nearest)return;
    const auto profile=celestial.ProfileFor(*nearest);const float r=celestial.WorldRadius(*nearest);
    if(nearestDistance<r*profile.ringInnerMultiplier*.82f||nearestDistance>r*profile.ringOuterMultiplier*1.12f)return;

    DisableSceneLighting();glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    const Vector3 center=frame.playerPhysics->position;
    const int count=180;
    glPointSize(2.0f);glBegin(GL_POINTS);
    for(int i=0;i<count;++i){
        std::uint32_t h=static_cast<std::uint32_t>(i*2654435761u+0x9e3779b9u);h^=h>>16;h*=2246822519u;h^=h>>13;
        const float a=static_cast<float>(h&0xffffu)/65535.0f*2.0f*kPi;
        const float d=8.0f+static_cast<float>((h>>16)&0xffu)/255.0f*78.0f;
        const float z=-0.10f+static_cast<float>((h>>24)&0xffu)/255.0f*1.3f;
        Color({0.72f,0.68f,0.58f,0.10f});glVertex3f(center.x+std::cos(a)*d,center.y+std::sin(a)*d,z);
    }
    glEnd();glDisable(GL_BLEND);SetupSceneLighting();
}


void DrawShipyardModuleThumbnail(const ObjMeshData& mesh,float x,float y,float width,float height,const Rgba& tint,EditorThumbnailViewPreset preset=EditorThumbnailViewPreset::ThreeQuarter){
    if(mesh.positions.empty()||mesh.triangles.empty()||width<8.0f||height<8.0f)return;
    struct PreviewPoint{float x=0,y=0,z=0;};
    struct CachedTri{PreviewPoint p[3];float light=1.0f;};
    struct CachedPreview{std::vector<CachedTri> triangles;float spanX=.001f,spanY=.001f,cx=0,cy=0;};
    static std::unordered_map<std::uintptr_t,CachedPreview> cache;
    const std::uintptr_t key=(reinterpret_cast<std::uintptr_t>(&mesh)>>3)^(static_cast<std::uintptr_t>(preset)+1u)*0x9e3779b9u;
    auto it=cache.find(key);
    if(it==cache.end()){
        if(cache.size()>512)cache.clear();
        std::vector<PreviewPoint> projected;projected.reserve(mesh.positions.size());float minX=1e30f,maxX=-1e30f,minY=1e30f,maxY=-1e30f;
        for(const auto& v:mesh.positions){PreviewPoint p;if(preset==EditorThumbnailViewPreset::Axial){p.x=v.x*.8660254f-v.z*.50f;p.y=-(v.y*.92f-(v.x+v.z)*.28f);p.z=(v.x*.35f+v.y*.80f+v.z*.35f);}else if(preset==EditorThumbnailViewPreset::Vertical){p.x=v.x*.88f-v.y*.22f;p.y=-(v.z*.90f+v.y*.32f-v.x*.12f);p.z=(v.x*.30f+v.y*.42f+v.z*.74f);}else{p.x=v.x*.7071067f-v.z*.7071067f;p.y=-(v.y*.8164966f-(v.x+v.z)*.4082483f);p.z=(v.x+v.y+v.z)*.5773502f;}minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);projected.push_back(p);}
        std::vector<std::size_t> order;order.reserve(mesh.triangles.size());const std::size_t stride=std::max<std::size_t>(1,mesh.triangles.size()/1600u);for(std::size_t i=0;i<mesh.triangles.size();i+=stride)order.push_back(i);
        std::sort(order.begin(),order.end(),[&](std::size_t a,std::size_t b){const auto& ta=mesh.triangles[a];const auto& tb=mesh.triangles[b];auto dz=[&](const auto& t){float z=0;int n=0;for(int k=0;k<3;++k){const int pi=t.position[k];if(pi>=0&&pi<static_cast<int>(projected.size())){z+=projected[pi].z;++n;}}return n?z/static_cast<float>(n):0.0f;};return dz(ta)<dz(tb);});
        CachedPreview built;built.spanX=std::max(.001f,maxX-minX);built.spanY=std::max(.001f,maxY-minY);built.cx=(minX+maxX)*.5f;built.cy=(minY+maxY)*.5f;built.triangles.reserve(order.size());for(const auto index:order){const auto& t=mesh.triangles[index];if(t.position[0]<0||t.position[1]<0||t.position[2]<0||t.position[0]>=static_cast<int>(projected.size())||t.position[1]>=static_cast<int>(projected.size())||t.position[2]>=static_cast<int>(projected.size()))continue;const auto& a=mesh.positions[t.position[0]];const auto& b=mesh.positions[t.position[1]];const auto& c=mesh.positions[t.position[2]];const float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z,vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;const float nl=std::sqrt(nx*nx+ny*ny+nz*nz);if(nl>1e-5f){nx/=nl;ny/=nl;nz/=nl;}const float light=std::clamp(.42f+.25f*std::fabs(nx)+.22f*std::fabs(ny)+.28f*std::fabs(nz),.34f,1.0f);built.triangles.push_back({{projected[t.position[0]],projected[t.position[1]],projected[t.position[2]]},light});}
        it=cache.emplace(key,std::move(built)).first;
    }
    const float scale=std::min(width*.82f/it->second.spanX,height*.82f/it->second.spanY);glBegin(GL_TRIANGLES);for(const auto& t:it->second.triangles){Color({tint.r*t.light,tint.g*t.light,tint.b*t.light,tint.a});for(const auto& q:t.p)glVertex3f(x+width*.5f+(q.x-it->second.cx)*scale,y+height*.5f+(q.y-it->second.cy)*scale,0);}glEnd();
}

void DrawShipBuilderOverlay(const NativeBattlefieldFrame& frame,const NativeBattlefieldRenderer::VisualAssets* assets){
    if(!frame.shipBuilder)return;
    const auto& m=*frame.shipBuilder;
    const auto layout=ShipyardBuilderSystem::Layout(frame.viewportWidth,frame.viewportHeight);
    if(!layout.valid)return;

    const float w=static_cast<float>(frame.viewportWidth),h=static_cast<float>(frame.viewportHeight);
    const float left=layout.left,top=layout.top,libraryW=layout.leftWidth,rightW=layout.rightWidth,right=layout.right;
    const auto uiTheme=SubspaceUiTheme::Dark();
    const Rgba panel={uiTheme.panel.r,uiTheme.panel.g,uiTheme.panel.b,0.965f};
    const Rgba card={uiTheme.raised.r*.78f,uiTheme.raised.g*.78f,uiTheme.raised.b*.78f,0.94f};
    const Rgba cardSoft={uiTheme.raised.r,uiTheme.raised.g,uiTheme.raised.b,0.90f};
    const Rgba cyan={uiTheme.accent.r,uiTheme.accent.g,uiTheme.accent.b,0.95f};
    const Rgba text={uiTheme.textPrimary.r,uiTheme.textPrimary.g,uiTheme.textPrimary.b,0.99f};
    const Rgba muted={uiTheme.textSecondary.r,uiTheme.textSecondary.g,uiTheme.textSecondary.b,0.82f};
    const Rgba amber={uiTheme.warning.r,uiTheme.warning.g,uiTheme.warning.b,0.96f};
    const float s=layout.uiScale;
    auto ShipyardText=[&](const std::string& value,float x,float y,float scale,const Rgba& color){DrawText5x7(value,x,y,scale*s,color);};

    auto shortText=[](std::string value,std::size_t maxChars){
        if(value.size()<=maxChars)return value;
        if(maxChars<4)return value.substr(0,maxChars);
        return value.substr(0,maxChars-3)+"...";
    };
    auto cardBox=[&](float x,float y,float cw,float ch){
        FilledRect(x,y,0,cw,ch,card);
        Line(x,y,0,x+cw,y,0,{.08f,.30f,.36f,.60f},1.0f);
        Line(x,y+ch,0,x+cw,y+ch,0,{.03f,.12f,.16f,.52f},1.0f);
    };
    auto section=[&](const std::string& label,float x,float y,float width){
        ShipyardText(label,x,y,.66f,{.54f,.78f,.81f,.90f});
        Line(x+112.0f,y+6.0f,0,x+width,y+6.0f,0,{.07f,.25f,.30f,.60f},1.0f);
    };

    // Pane shells.  They deliberately stop above the status bar, making the
    // editor read as two stable inspectors around the central 3D canvas.
    FilledRect(left,top,0,libraryW,layout.statusY-top-12.0f,panel);
    FilledRect(right,top,0,rightW,layout.statusY-top-12.0f,panel);
    Line(left,top,0,left+libraryW,top,0,cyan,1.8f);
    Line(right,top,0,right+rightW,top,0,cyan,1.8f);

    std::size_t filteredCount=0;
    for(const auto& r:m.catalog)if(r.moduleClass==m.selectedClass)++filteredCount;

    // Left library hierarchy.
    ShipyardText("SHIPYARD",left+12,top+14,1.12f,text);
    ShipyardText(frame.standaloneShipyard?"BLUEPRINT DESIGN":"LIVE REFIT",left+12,top+35,.72f,muted);
    ShipyardText(std::string("PARTS  /  ")+ShipyardModuleSystem::ClassName(m.selectedClass)+"  /  "+std::to_string(filteredCount),
        left+12,top+52,.62f,{.48f,.70f,.74f,.82f});

    // Permanent canonical-forward authority. This marker is screen-space so it
    // remains visible regardless of selected part, depth occlusion or camera
    // distance, while its arrow direction is derived from the actual world
    // projection of ship-local +Y. It intentionally ignores the source-family
    // visual yaw: +Y is gameplay ship-forward, not OBJ-forward.
    {
        const float canvasLeft=left+libraryW+12.0f,canvasRight=right-12.0f;
        const float markerX=(canvasLeft+canvasRight)*.5f,markerY=top+42.0f;
        float dx=0.0f,dy=-1.0f;
        if(frame.camera){
            const float yaw=frame.playerPhysics?frame.playerPhysics->rotation.z:0.0f;
            const Vector3 worldForward{-std::sin(yaw),std::cos(yaw),0.0f};
            const auto basis=StrategicViewProjection::Build(*frame.camera,static_cast<float>(frame.viewportWidth),static_cast<float>(frame.viewportHeight));
            const float rx=worldForward.x*basis.right.x+worldForward.y*basis.right.y+worldForward.z*basis.right.z;
            const float uy=worldForward.x*basis.up.x+worldForward.y*basis.up.y+worldForward.z*basis.up.z;
            const float len=std::sqrt(rx*rx+uy*uy);if(len>1.0e-5f){dx=rx/len;dy=-uy/len;}
        }
        const float arrow=38.0f,head=9.0f;const Rgba forwardColor={.30f,.96f,.52f,.98f};
        Line(markerX,markerY,0,markerX+dx*arrow,markerY+dy*arrow,0,forwardColor,3.0f);
        const float px=-dy,py=dx;
        Line(markerX+dx*arrow,markerY+dy*arrow,0,markerX+dx*(arrow-head)+px*head*.65f,markerY+dy*(arrow-head)+py*head*.65f,0,forwardColor,2.4f);
        Line(markerX+dx*arrow,markerY+dy*arrow,0,markerX+dx*(arrow-head)-px*head*.65f,markerY+dy*(arrow-head)-py*head*.65f,0,forwardColor,2.4f);
        ShipyardText("SHIP FORWARD +Y",markerX-48.0f,markerY+15.0f,.66f,forwardColor);
    }

    // Pass746: screen-aligned construction frame follows the actual transformed
    // authored assembly bounds. The frame center follows the actual projected
    // construction envelope and adds a true ten-foot clear margin around every
    // side, regardless of symmetry-plane orientation.
    if(frame.camera&&m.symmetryFrame.live&&assets){
        constexpr float kConstructionMarginWorld=3.048f; // ten feet
        Vector3 base{};float shipYaw=0.0f;if(frame.playerPhysics){base=frame.playerPhysics->position;shipYaw=frame.playerPhysics->rotation.z;}
        const std::string role=m.recipe.role.empty()?m.role:m.recipe.role;const auto axis=ResolveShipAxisScale(.24f,role,true,.22f,&m.recipe);
        const float rootYaw=shipYaw+RecipeForwardVisualYawRadians(&m.recipe),cy=std::cos(rootYaw),sy=std::sin(rootYaw);
        auto toWorld=[&](Vector3 q){q={q.x*axis.x,q.y*axis.y,q.z*axis.z};return base+Vector3{q.x*cy-q.y*sy,q.x*sy+q.y*cy,q.z+.30f};};
        const auto bounds=BuildRecipeLocalBounds(*assets,m.recipe);const Vector3 o=m.symmetryFrame.origin;const auto originWorld=toWorld(o);
        const auto originScreen=StrategicViewProjection::WorldToScreen(originWorld,w,h,*frame.camera);
        if(bounds.valid&&originScreen.depth>0.0f){
            const Vector3 mn=bounds.min,mx=bounds.max;
            const Vector3 corners[8]={{mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mx.y,mn.z},{mn.x,mx.y,mn.z},{mn.x,mn.y,mx.z},{mx.x,mn.y,mx.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z}};
            float minX=1.0e30f,minY=1.0e30f,maxX=-1.0e30f,maxY=-1.0e30f;bool projected=false;
            for(const auto& corner:corners){const auto q=StrategicViewProjection::WorldToScreen(toWorld(corner),w,h,*frame.camera);if(q.depth<=0.0f)continue;minX=std::min(minX,q.x);minY=std::min(minY,q.y);maxX=std::max(maxX,q.x);maxY=std::max(maxY,q.y);projected=true;}
            if(projected){
                const auto basis=StrategicViewProjection::Build(*frame.camera,w,h);
                const auto rightMargin=StrategicViewProjection::WorldToScreen(originWorld+basis.right*kConstructionMarginWorld,w,h,*frame.camera);
                const auto upMargin=StrategicViewProjection::WorldToScreen(originWorld+basis.up*kConstructionMarginWorld,w,h,*frame.camera);
                const float marginX=std::max(10.0f,std::fabs(rightMargin.x-originScreen.x));
                const float marginY=std::max(10.0f,std::fabs(upMargin.y-originScreen.y));
                // Frame the real visible assembly with a true ten-foot gap on
                // every side.  The vertical authority line follows the visual
                // center of the complete construction, not an arbitrary module
                // origin, so ships/stations stay centered as the recipe grows.
                const float leftX=minX-marginX,rightX=maxX+marginX;
                const float topY=minY-marginY,bottomY=maxY+marginY;
                const float frameCenterX=(leftX+rightX)*.5f;
                const float frameCenterY=(topY+bottomY)*.5f;
                const Rgba sym={.64f,.40f,.94f,.58f},axisColor={.76f,.52f,1.0f,.76f};
                Line(leftX,topY,0,rightX,topY,0,sym,1.2f);Line(rightX,topY,0,rightX,bottomY,0,sym,1.2f);Line(rightX,bottomY,0,leftX,bottomY,0,sym,1.2f);Line(leftX,bottomY,0,leftX,topY,0,sym,1.2f);
                Line(frameCenterX,topY,0,frameCenterX,bottomY,0,axisColor,1.6f);
                Line(frameCenterX-8*s,frameCenterY,0,frameCenterX+8*s,frameCenterY,0,axisColor,1.4f);Line(frameCenterX,frameCenterY-8*s,0,frameCenterX,frameCenterY+8*s,0,axisColor,1.4f);
                // Keep the editable symmetry origin visible as a smaller marker
                // without letting it drag the visual framing off-center.
                Line(originScreen.x-5*s,originScreen.y,0,originScreen.x+5*s,originScreen.y,0,sym,1.0f);Line(originScreen.x,originScreen.y-5*s,0,originScreen.x,originScreen.y+5*s,0,sym,1.0f);
                ShipyardText(ConstructionSymmetrySystem::AxisName(m.symmetryFrame.axis),frameCenterX+10*s,frameCenterY+8*s,.44f,sym);
            }
        }
    }

    // Right inspector hierarchy.
    ShipyardText("SHIPYARD INSPECTOR",right+12,top+14,1.08f,text);
    ShipyardText(m.role+"  /  SEED "+std::to_string(m.seed)+"  /  "+std::to_string(m.recipe.modules.size())+" MODULES",
        right+12,top+35,.68f,muted);

    const auto* selectedCatalog=[&]()->const ShipyardModuleRecord*{
        std::vector<std::size_t> f;
        for(std::size_t i=0;i<m.catalog.size();++i)if(m.catalog[i].moduleClass==m.selectedClass)f.push_back(i);
        if(f.empty())return nullptr;
        return &m.catalog[f[std::min(m.selectedFilteredModule,f.size()-1)]];
    }();
    const auto* selectedPlacedRecord=[&]()->const ShipyardModuleRecord*{
        if(m.recipe.modules.empty())return nullptr;
        const auto index=std::min(m.selectedPlacedModule,m.recipe.modules.size()-1);
        const auto& id=m.recipe.modules[index].moduleId;
        for(const auto& r:m.catalog)if(r.source.moduleId==id)return &r;
        return nullptr;
    }();

    std::vector<const ShipyardModuleRecord*> filteredCatalog;
    filteredCatalog.reserve(m.catalog.size());
    for(const auto& record:m.catalog)if(record.moduleClass==m.selectedClass)filteredCatalog.push_back(&record);

    const auto controls=ShipyardBuilderSystem::BuildControls(m,frame.viewportWidth,frame.viewportHeight);
    const ShipyardBuilderControl* hovered=nullptr;
    for(const auto& c:controls){
        const bool hover=frame.pointerX>=c.x&&frame.pointerX<=c.x+c.width&&frame.pointerY>=c.y&&frame.pointerY<=c.y+c.height;
        if(hover)hovered=&c;

        // Real geometry asset cards. The previous implementation only changed
        // the editor architecture while still drawing the same text-list UI;
        // this is the visible inventory treatment requested for Shipyard V2.
        if(c.command==ShipyardBuilderCommand::SelectModule){
            const ShipyardModuleRecord* record=(c.value>=0&&static_cast<std::size_t>(c.value)<filteredCatalog.size())?filteredCatalog[static_cast<std::size_t>(c.value)]:nullptr;
            Rgba bg=c.active?Rgba{.035f,.145f,.175f,.99f}:Rgba{.010f,.040f,.052f,.97f};
            Rgba border=c.active?Rgba{.30f,.88f,.92f,.96f}:Rgba{.08f,.27f,.33f,.72f};
            if(hover){bg={.025f,.090f,.112f,.99f};border={.42f,.91f,.94f,.98f};}
            FilledRect(c.x,c.y,0,c.width,c.height,bg);
            Line(c.x,c.y,0,c.x+c.width,c.y,0,border,1.2f*s);
            Line(c.x,c.y+c.height,0,c.x+c.width,c.y+c.height,0,{border.r*.7f,border.g*.7f,border.b*.7f,border.a},1.0f*s);
            const float previewW=std::min(c.height*1.28f,112.0f*s);
            FilledRect(c.x+6.0f*s,c.y+6.0f*s,0,previewW-12.0f*s,c.height-12.0f*s,{.004f,.014f,.020f,.98f});
            if(record){
                const auto thumb=EditorAssetThumbnailSystem::Build(*record);
                if(assets){const auto mi=assets->shipModules.find(record->source.moduleId);
                if(mi!=assets->shipModules.end())DrawShipyardModuleThumbnail(mi->second,c.x+6.0f*s,c.y+6.0f*s,previewW-12.0f*s,c.height-12.0f*s,c.active?Rgba{.72f,.92f,.95f,.98f}:Rgba{.58f,.70f,.73f,.96f},thumb.viewPreset);}
                const float tx=c.x+previewW+8.0f*s;
                ShipyardText(shortText(ShipyardModuleSystem::SemanticName(record->semantic),30),tx,c.y+10.0f*s,.70f,c.active?Rgba{1.0f,.86f,.48f,.99f}:text);
                ShipyardText(shortText((record->placementRole.empty()?std::string(ShipyardModuleSystem::ClassName(record->moduleClass)):record->placementRole)+"  /  "+ShipyardModuleSystem::ClassName(record->moduleClass),34),tx,c.y+31.0f*s,.53f,record->placementRole=="REVIEW_REQUIRED"?Rgba{.98f,.50f,.26f,.96f}:muted);
                const Rgba cert=thumb.reviewState==EditorThumbnailReviewState::Certified?Rgba{.36f,.88f,.60f,.92f}:(thumb.reviewState==EditorThumbnailReviewState::ManualOnly?Rgba{.78f,.63f,.34f,.92f}:Rgba{.96f,.52f,.30f,.94f});
                const std::string detail=thumb.propulsion?("COMPAT "+thumb.sizeBadge+"   T "+thumb.thrustLabel+"  E "+thumb.exhaustLabel+"   "+thumb.certificationBadge):("COMPAT "+thumb.sizeBadge+"   SOCKETS "+std::to_string(record->sockets.size())+"   "+thumb.certificationBadge);
                ShipyardText(detail,tx,c.y+49.0f*s,.48f,cert);
                ShipyardText(thumb.mirrorSupported?"MIR":"",c.x+c.width-54.0f*s,c.y+10.0f*s,.42f,{.45f,.74f,.78f,.78f});
            }else ShipyardText(c.label,c.x+previewW+8.0f*s,c.y+18.0f*s,.62f,text);
            continue;
        }

        const bool tab=c.command==ShipyardBuilderCommand::WorkspaceBuild||
                       c.command==ShipyardBuilderCommand::WorkspaceAppearance||
                       c.command==ShipyardBuilderCommand::WorkspaceSystems||
                       c.command==ShipyardBuilderCommand::WorkspaceAuthoring||
                       c.command==ShipyardBuilderCommand::InspectorSockets||
                       c.command==ShipyardBuilderCommand::InspectorAuthoring;
        const bool danger=c.command==ShipyardBuilderCommand::RemoveModule||
                          c.command==ShipyardBuilderCommand::RemoveSocket||
                          c.command==ShipyardBuilderCommand::Reset||
                          c.command==ShipyardBuilderCommand::RemoveDecal;
        const bool primary=c.command==ShipyardBuilderCommand::AddModule||
                           c.command==ShipyardBuilderCommand::ReplaceModule||
                           c.command==ShipyardBuilderCommand::GenerateVariant||
                           c.command==ShipyardBuilderCommand::SaveBlueprint||
                           c.command==ShipyardBuilderCommand::SaveSocketOverrides||
                           c.command==ShipyardBuilderCommand::SaveDefinitionOverrides||
                           c.command==ShipyardBuilderCommand::Apply;
        const bool validate=c.command==ShipyardBuilderCommand::Validate;

        Rgba bg={.018f,.052f,.066f,.95f};
        Rgba border={.09f,.26f,.31f,.62f};
        Rgba fg={.71f,.85f,.87f,.96f};
        if(tab){bg={.010f,.032f,.042f,.96f};border={.08f,.23f,.28f,.70f};}
        if(primary){bg={.030f,.112f,.136f,.97f};border={.16f,.50f,.58f,.76f};}
        if(danger){bg={.090f,.040f,.035f,.93f};border={.42f,.18f,.14f,.68f};fg={.94f,.72f,.66f,.94f};}
        if(validate){
            bg=m.validation.valid?Rgba{.025f,.112f,.075f,.96f}:Rgba{.112f,.072f,.025f,.96f};
            border=m.validation.valid?Rgba{.16f,.60f,.40f,.76f}:Rgba{.62f,.40f,.12f,.76f};
        }
        if(c.active){bg={.055f,.235f,.285f,.99f};border={.32f,.88f,.92f,.94f};fg={.96f,1.0f,1.0f,1.0f};}
        if(hover&&c.enabled){bg={std::min(1.0f,bg.r+.050f),std::min(1.0f,bg.g+.070f),std::min(1.0f,bg.b+.080f),.99f};border={.40f,.90f,.94f,.98f};fg={1.0f,.94f,.70f,1.0f};}
        if(!c.enabled){bg={.009f,.020f,.027f,.68f};border={.05f,.11f,.14f,.48f};fg={.28f,.38f,.40f,.55f};}

        FilledRect(c.x,c.y,0,c.width,c.height,bg);
        Line(c.x,c.y,0,c.x+c.width,c.y,0,border,1.0f);
        Line(c.x,c.y+c.height,0,c.x+c.width,c.y+c.height,0,{border.r*.62f,border.g*.62f,border.b*.62f,border.a*.78f},1.0f);
        if(tab&&c.active)FilledRect(c.x,c.y+c.height-3.0f,0,c.width,3.0f,cyan);
        ShipyardText(c.label,c.x+8,c.y+10,.68f,fg);
    }

    // Selected library-part card: larger internal spacing and reduced raw
    // metadata keep this useful without reading like a diagnostics dump.
    const float infoY=layout.leftInfoY;
    const float infoH=std::max(112.0f,std::min(142.0f,layout.statusY-infoY-22.0f));
    cardBox(left+10,infoY-8,libraryW-20,infoH);
    ShipyardText("SELECTED PART  /  DRAG INTO VIEWPORT TO PREVIEW",left+18,infoY+2,.56f,{.50f,.75f,.78f,.88f});
    if(selectedCatalog){
        ShipyardText(ShipyardModuleSystem::SemanticName(selectedCatalog->semantic),left+18,infoY+25,.82f,amber);
        ShipyardText(std::string("CLASS  ")+ShipyardModuleSystem::ClassName(selectedCatalog->moduleClass)+
            "    SIZE  "+ShipyardModuleSystem::SizeName(selectedCatalog->size),left+18,infoY+49,.64f,{.64f,.81f,.84f,.90f});
        ShipyardText("SOCKETS  "+std::to_string(selectedCatalog->sockets.size()),left+18,infoY+70,.62f,{.50f,.69f,.72f,.82f});
        ShipyardText("ROOT  "+(selectedCatalog->preferredMountFace.empty()?"AUTO":selectedCatalog->preferredMountFace),
            left+18,infoY+90,.60f,{.78f,.52f,.38f,.86f});
        if(infoH>128.0f)ShipyardText("RED MARKER = MOUNT FACE",left+18,infoY+110,.56f,{.72f,.42f,.34f,.80f});
    }else{
        ShipyardText("NO PART IN THIS CATEGORY",left+18,infoY+27,.70f,muted);
    }

    // Persistent selected-module summary directly beneath the inspector tabs.
    const float summaryY=layout.selectedSummaryY;
    cardBox(right+10,summaryY-6,rightW-20,54.0f);
    if(!m.recipe.modules.empty()){
        const auto index=std::min(m.selectedPlacedModule,m.recipe.modules.size()-1);
        const auto& p=m.recipe.modules[index];
        const std::string partName=selectedPlacedRecord?ShipyardModuleSystem::SemanticName(selectedPlacedRecord->semantic):"MODULE";
        ShipyardText("SELECTED MODULE  "+std::to_string(index+1)+" / "+std::to_string(m.recipe.modules.size())+"   "+partName,
            right+18,summaryY+5,.70f,amber);
        std::ostringstream xform;xform.setf(std::ios::fixed);xform.precision(1);
        xform<<"P "<<p.pitchDegrees<<"   Y "<<p.yawDegrees<<"   R "<<p.rollDegrees
             <<"      POS "<<p.x<<", "<<p.y<<", "<<p.z
             <<"      S "<<p.scaleX<<"/"<<p.scaleY<<"/"<<p.scaleZ;
        ShipyardText(xform.str(),right+18,summaryY+27,.60f,{.54f,.72f,.75f,.86f});
    }else{
        ShipyardText("NO MODULE SELECTED",right+18,summaryY+14,.68f,muted);
    }

    // Page-specific visual hierarchy.  Only the active workflow is shown,
    // eliminating the wall-of-controls look from R4.
    if(m.inspectorTab==ShipyardInspectorTab::Transform){
        section("BUILD TOOL",right+14,layout.editLabelY,rightW-28);
        section("FOCUS",right+14,layout.focusLabelY,rightW-28);
        if(m.transformTool==ShipyardTransformTool::Move)section("MOVE  /  ARROWS FOLLOW ACTIVE SPACE  /  SHIFT = PRECISION",right+14,layout.moveLabelY,rightW-28);
        else if(m.transformTool==ShipyardTransformTool::Rotate)section("ROTATE  /  SHIFT = PRECISION",right+14,layout.moveLabelY,rightW-28);
        else if(m.transformTool==ShipyardTransformTool::Scale)section("SCALE  /  PART OR WHOLE ASSEMBLY",right+14,layout.moveLabelY,rightW-28);
        else section("SELECT A MODULE OR CHOOSE A BUILD TOOL",right+14,layout.moveLabelY,rightW-28);
    }else if(m.inspectorTab==ShipyardInspectorTab::Sockets){
        section("SOCKET AUTHORING  /  EDITS REUSABLE MODULE DEFINITION",right+14,layout.editLabelY,rightW-28);
        section("SOCKET TRANSFORM",right+14,layout.focusLabelY,rightW-28);
        if(selectedPlacedRecord&&!selectedPlacedRecord->sockets.empty()){
            const auto si=std::min(m.selectedSocket,selectedPlacedRecord->sockets.size()-1);
            const auto& socket=selectedPlacedRecord->sockets[si];
            std::ostringstream ss;ss.setf(std::ios::fixed);ss.precision(2);
            ss<<"ACTIVE  "<<socket.name<<"  /  "<<socket.type<<"   POS "<<socket.x<<", "<<socket.y<<", "<<socket.z
              <<"   DIR "<<socket.dirX<<", "<<socket.dirY<<", "<<socket.dirZ;
            ShipyardText(shortText(ss.str(),74),right+18,layout.editLabelY-17.0f,.52f,{.92f,.78f,.32f,.92f});
        }else ShipyardText("SELECT A PLACED MODULE; ADD SOCKET IF NONE EXIST",right+18,layout.editLabelY-17.0f,.52f,muted);
        section(m.transformTool==ShipyardTransformTool::Rotate?"ROTATE SOCKET FRAME / DIRECTION":"MOVE SOCKET ORIGIN",right+14,layout.moveLabelY,rightW-28);
        section("SOCKET DEFINITION / HISTORY",right+14,layout.rotateLabelY,rightW-28);
    }else if(m.inspectorTab==ShipyardInspectorTab::Authoring){
        section("TEACH PCG / REUSABLE MODULE DEFINITION",right+14.0f*s,layout.editLabelY,rightW-28.0f*s);
        if(selectedPlacedRecord){
            ShipyardText(std::string("SEMANTIC  ")+ShipyardModuleSystem::SemanticName(selectedPlacedRecord->semantic),right+18.0f*s,layout.focusLabelY-16.0f*s,.58f,amber);
            ShipyardText(std::string("GENERATOR  ")+(selectedPlacedRecord->generatorEligible?"ENABLED":"QUARANTINED")+"    PAIRING  "+(selectedPlacedRecord->pairedPlacement?"PAIRED":"SINGLE"),right+18.0f*s,layout.moveLabelY-16.0f*s,.52f,muted);
            ShipyardText("MOUNT FACE  "+(selectedPlacedRecord->preferredMountFace.empty()?std::string("AUTO"):selectedPlacedRecord->preferredMountFace),right+18.0f*s,layout.moveLabelY+4.0f*s,.52f,{.62f,.76f,.78f,.88f});
        }else ShipyardText("SELECT A PLACED MODULE TO TEACH PCG",right+18.0f*s,layout.focusLabelY-16.0f*s,.54f,muted);
        section("DEFINITION OVERRIDE / PERSISTENCE",right+14.0f*s,layout.rotateLabelY,rightW-28.0f*s);
        ShipyardText("Socket geometry is edited from the dedicated SOCKETS tab.",right+18.0f*s,layout.rotateLabelY+24.0f*s,.48f,{.52f,.70f,.73f,.84f});
    }else if(m.inspectorTab==ShipyardInspectorTab::Assembly){
        section("ASSEMBLED MODULES / SYSTEMS",right+14.0f*s,layout.placedListY-20.0f*s,rightW-28.0f*s);
        section("BLUEPRINT / GENERATOR",right+14.0f*s,layout.blueprintLabelY,rightW-28.0f*s);
    }else if(m.inspectorTab==ShipyardInspectorTab::Appearance){
        section("PAINT & LIVERY",right+14.0f*s,layout.liveryLabelY,rightW-28.0f*s);
        ShipyardText("PRESET  "+m.liveryName,right+18,layout.liveryLabelY+20,.60f,amber);
        const auto& paint=m.appearance;
        const float swY=layout.decalRowY+48.0f;
        cardBox(right+12,swY,rightW-24,84.0f);
        const float swW=(rightW-52.0f)/3.0f;
        FilledRect(right+20,swY+34,0,swW-8,30,{paint.primary.r,paint.primary.g,paint.primary.b,1.0f});
        FilledRect(right+20+swW,swY+34,0,swW-8,30,{paint.secondary.r,paint.secondary.g,paint.secondary.b,1.0f});
        FilledRect(right+20+swW*2,swY+34,0,swW-8,30,{paint.trim.r,paint.trim.g,paint.trim.b,1.0f});
        ShipyardText("PRIMARY  "+m.primaryPaintName,right+20,swY+10,.56f,text);
        ShipyardText("SECONDARY  "+m.secondaryPaintName,right+20+swW,swY+10,.53f,text);
        ShipyardText("ACCENT  "+m.trimPaintName,right+20+swW*2,swY+10,.56f,text);
        ShipyardText("Changes preview immediately on Flat / Flat.001 paint zones.",right+20,swY+69,.51f,muted);
    }else{
        section("SHIPYARD",right+14.0f*s,layout.editLabelY,rightW-28.0f*s);
        ShipyardText("No inspector workflow is active.",right+18.0f*s,layout.editLabelY+24.0f*s,.54f,muted);
    }

    // Validation card is intentionally compact and calm when valid.  It only
    // expands into error text when something needs action.
    const float validationTop=layout.validationY;
    const float validationH=std::max(58.0f,layout.statusY-validationTop-14.0f);
    cardBox(right+10,validationTop-6,rightW-20,validationH);
    ShipyardText(m.validation.valid?"GENERATOR CERTIFIED":"AUTHORING DRAFT / SAVE ALLOWED",
        right+18,validationTop+4,.72f,m.validation.valid?Rgba{.32f,.86f,.58f,.95f}:Rgba{1.0f,.66f,.24f,.95f});
    float vy=validationTop+28.0f;
    if(m.validation.valid){
        ShipyardText("Eligible for generator certification and docked apply.",right+18,vy,.57f,{.48f,.68f,.70f,.78f});
    }else{
        ShipyardText("Custom drafts still save + export for generator refinement review.",right+18,vy,.54f,{.72f,.66f,.48f,.82f});
        vy+=18.0f;
        int shown=0;
        for(const auto& e:m.validation.errors){
            if(shown++>=3||vy>layout.statusY-24.0f)break;
            ShipyardText("! "+shortText(e,58),right+18,vy,.58f,{.96f,.44f,.28f,.91f});vy+=18.0f;
        }
        shown=0;
        for(const auto& warn:m.validation.warnings){
            if(shown++>=2||vy>layout.statusY-24.0f)break;
            ShipyardText("? "+shortText(warn,58),right+18,vy,.56f,{.92f,.70f,.28f,.84f});vy+=17.0f;
        }
    }

    // Full-width status bar with two deliberate text zones; long status text
    // is clipped before it can collide with contextual help.
    FilledRect(left,layout.statusY-8,0,w-left-14.0f,36,{.004f,.018f,.026f,.98f});
    std::string help="RMB orbit   MMB truck/pedestal   Wheel dolly   Alt+WASD free cam   Alt+RMB look   Q/E roll   Home frame";
    if(hovered){
        switch(hovered->command){
            case ShipyardBuilderCommand::SelectModule:{
                if(hovered->value>=0&&static_cast<std::size_t>(hovered->value)<filteredCatalog.size()){
                    const auto* hoveredRecord=filteredCatalog[static_cast<std::size_t>(hovered->value)];
                    const auto t=EditorAssetThumbnailSystem::Build(*hoveredRecord);
                    const std::string functionalRole=hoveredRecord->placementRole.empty()?std::string(ShipyardModuleSystem::ClassName(hoveredRecord->moduleClass)):hoveredRecord->placementRole;
                    help=std::string("Kitbash module: ")+ShipyardModuleSystem::SemanticName(hoveredRecord->semantic)+" / "+functionalRole+" / "+ShipyardModuleSystem::ClassName(hoveredRecord->moduleClass)+" / compatibility "+t.sizeBadge+" / "+t.certificationBadge+(t.propulsion?(" / thrust "+t.thrustLabel+" exhaust "+t.exhaustLabel):"")+". Drag the real geometry into the viewport; live symmetry previews its exact reflected partner.";
                }
                else help="Select a kitbash module to preview and drag into the construction viewport.";break;}
            case ShipyardBuilderCommand::WorkspaceBuild:help="Build: browse kitbash parts, drag to preview, snap to compatible mounts, then refine transforms.";break;
            case ShipyardBuilderCommand::WorkspaceAppearance:help="Appearance: paint, livery, patterns, camouflage, decals, and normalized surface sections.";break;
            case ShipyardBuilderCommand::WorkspaceSystems:help="Systems: inspect functional modules, ship role, blueprint validation, and fitting authority.";break;
            case ShipyardBuilderCommand::WorkspaceAuthoring:help="Authoring: advanced sockets, Teach PCG, material mapping, DesignDNA, and certification.";break;
            case ShipyardBuilderCommand::InspectorTransform:help="Transform: position, rotate, flip, and frame the selected module.";break;
            case ShipyardBuilderCommand::InspectorSockets:help="Sockets: edit reusable mating points, directions, types, and persistent authoring overrides.";break;
            case ShipyardBuilderCommand::InspectorAuthoring:help="Teach PCG: correct reusable semantic, generator eligibility, pairing, and mount-face intent.";break;
            case ShipyardBuilderCommand::InspectorAssembly:help="Assembly: select placed modules, generate variants, save, and apply.";break;
            case ShipyardBuilderCommand::InspectorAppearance:help="Appearance: live ship paint, secondary color, accent, and decals.";break;
            case ShipyardBuilderCommand::PreviousLiveryPreset:
            case ShipyardBuilderCommand::NextLiveryPreset:help="Paint Preset: recolor primary, secondary, and accent zones immediately.";break;
            case ShipyardBuilderCommand::PreviousPrimaryPaint:
            case ShipyardBuilderCommand::NextPrimaryPaint:help="Primary Paint: cycle the main Flat hull color.";break;
            case ShipyardBuilderCommand::PreviousSecondaryPaint:
            case ShipyardBuilderCommand::NextSecondaryPaint:help="Secondary Paint: cycle Flat.001 and secondary regions.";break;
            case ShipyardBuilderCommand::PreviousTrimPaint:
            case ShipyardBuilderCommand::NextTrimPaint:help="Accent Paint: cycle trim/decal accent color.";break;
            case ShipyardBuilderCommand::FrameSelected:help="Frame Part: focus closely on the selected module.";break;
            case ShipyardBuilderCommand::FrameShip:help="Frame Ship: restore a useful whole-ship view.";break;
            case ShipyardBuilderCommand::ToggleTransformSpace:help="Transform Space: cycle CAMERA, SHIP, and LOCAL movement axes.";break;
            case ShipyardBuilderCommand::ToggleTransformSnap:help="Snap: toggle snapped transform increments.";break;
            case ShipyardBuilderCommand::CycleRotationStep:help="Rotation Step: cycle 15, 5, and 1 degree increments; Shift gives 0.1x precision.";break;
            case ShipyardBuilderCommand::ToolScale:help="Scale: drag in the viewport or use precise part/assembly scale controls.";break;
            case ShipyardBuilderCommand::ScaleUniformNegative:
            case ShipyardBuilderCommand::ScaleUniformPositive:help="Part Scale: uniform selected-module scaling. Shift precision is available during drag.";break;
            case ShipyardBuilderCommand::ScaleAssemblyDown:
            case ShipyardBuilderCommand::ScaleAssemblyUp:help="Assembly Scale: resize the whole ship around its assembly center.";break;
            case ShipyardBuilderCommand::PreviousSocket:
            case ShipyardBuilderCommand::NextSocket:help="Socket Select: cycle the selected module's reusable attachment sockets.";break;
            case ShipyardBuilderCommand::AddSocket:help="Add Socket: create a new manual detail-mount socket on this module definition.";break;
            case ShipyardBuilderCommand::RemoveSocket:help="Remove Socket: blocked while the current assembly actively uses that socket.";break;
            case ShipyardBuilderCommand::MirrorSocketX:help="Mirror Socket: reflect the active socket across the currently selected construction symmetry plane.";break;
            case ShipyardBuilderCommand::SymmetryAxisPortStarboard:help="Port/Starboard symmetry: exact left/right reflection across the editable ship centerline.";break;
            case ShipyardBuilderCommand::SymmetryAxisForeAft:help="Fore/Aft symmetry: exact front/back reflection across the editable Y plane.";break;
            case ShipyardBuilderCommand::SymmetryAxisDorsalVentral:help="Dorsal/Ventral symmetry: exact top/bottom reflection across the editable Z plane.";break;
            case ShipyardBuilderCommand::ToggleLiveSymmetry:help="Live Symmetry: drag/place one module and preview/commit an exact reflected partner.";break;
            case ShipyardBuilderCommand::SymmetryPlaneNegative:
            case ShipyardBuilderCommand::SymmetryPlanePositive:help="Move the active symmetry plane without moving the ship or existing source module.";break;
            case ShipyardBuilderCommand::ResetSymmetryFrame:help="Center: reset the symmetry plane origin to the current assembly center.";break;
            case ShipyardBuilderCommand::MirrorSelectedAcrossSymmetry:help="Mirror Copy: create an exact opposite-handed partner of the selected module across the active plane.";break;
            case ShipyardBuilderCommand::BreakSymmetryPair:help="Break Pair: unlink the selected mirrored pair so each side can be edited independently.";break;
            case ShipyardBuilderCommand::CycleSocketType:help="Socket Type: cycle the mating/functional type; current assembly is revalidated immediately.";break;
            case ShipyardBuilderCommand::ResetSocket:help="Reset Socket: restore the inferred/certified baseline definition for this named socket.";break;
            case ShipyardBuilderCommand::UndoSocketEdit:help="Undo the most recent reusable socket-definition edit.";break;
            case ShipyardBuilderCommand::RedoSocketEdit:help="Redo the next reusable socket-definition edit.";break;
            case ShipyardBuilderCommand::SaveSocketOverrides:help="Save only manual socket-definition differences as a persistent authoring override file.";break;
            case ShipyardBuilderCommand::PreviousSemantic:
            case ShipyardBuilderCommand::NextSemantic:help="Teach PCG Semantic: correct what this reusable kitbash module actually is.";break;
            case ShipyardBuilderCommand::ToggleGeneratorEligible:help="PCG Eligibility: allow or quarantine this reusable module from procedural generation.";break;
            case ShipyardBuilderCommand::TogglePairedPlacement:help="Paired Placement: mark the module as normally mirrored/paired during generation.";break;
            case ShipyardBuilderCommand::CyclePreferredMountFace:help="Mount Face: review the preferred structural side used by placement scoring.";break;
            case ShipyardBuilderCommand::ResetDefinitionOverride:help="Reset Teach PCG definition metadata to the certified/inferred baseline.";break;
            case ShipyardBuilderCommand::SaveDefinitionOverrides:help="Persist only reviewed Teach PCG definition differences; source assets remain untouched.";break;
            case ShipyardBuilderCommand::ReplaceModule:help="Replace only the selected placed module with the selected library part.";break;
            case ShipyardBuilderCommand::Apply:help="Apply the validated refit while docked.";break;
            default:help=hovered->label;break;
        }
        if(!hovered->enabled)help+="  (Unavailable in the current state.)";
    }
    // Floating contextual tooltip: help is visible where the pointer is, not
    // only in a distant status strip.  This is the first production consumer
    // of the normalized project-wide tooltip behavior.
    if(hovered&&hovered->width>0.0f&&!hovered->label.empty()){
        const float tipW=std::min(430.0f*s,w*.34f),tipH=82.0f*s;
        float tx=frame.pointerX+18.0f*s,ty=frame.pointerY+18.0f*s;
        if(tx+tipW>w-14.0f*s)tx=frame.pointerX-tipW-18.0f*s;
        if(ty+tipH>h-14.0f*s)ty=frame.pointerY-tipH-18.0f*s;
        FilledRect(tx,ty,0,tipW,tipH,{.006f,.022f,.030f,.985f});
        Line(tx,ty,0,tx+tipW,ty,0,{.26f,.78f,.84f,.92f},1.4f*s);
        ShipyardText(shortText(hovered->label,42),tx+12.0f*s,ty+10.0f*s,.64f,{.96f,.88f,.58f,.98f});
        std::string line1=help,line2;
        const std::size_t wrap=58;
        if(line1.size()>wrap){std::size_t cut=line1.rfind(' ',wrap);if(cut==std::string::npos)cut=wrap;line2=line1.substr(cut+1);line1=line1.substr(0,cut);}
        ShipyardText(shortText(line1,64),tx+12.0f*s,ty+34.0f*s,.50f,{.70f,.84f,.86f,.94f});
        if(!line2.empty())ShipyardText(shortText(line2,64),tx+12.0f*s,ty+54.0f*s,.50f,{.58f,.73f,.76f,.88f});
    }

    ShipyardText(shortText(m.status,48),left+12.0f*s,layout.statusY+2.0f*s,.62f,
        m.validation.valid?Rgba{.40f,.84f,.66f,.92f}:Rgba{.92f,.64f,.24f,.92f});
    ShipyardText(shortText(help,82),std::max(left+370.0f*s,w*.45f),layout.statusY+2.0f*s,.58f,{.50f,.69f,.72f,.80f});
}


void DrawWorkspaceOverlay(const NativeBattlefieldFrame& frame,const NativeBattlefieldRenderer::VisualAssets* assets) {
    const bool workspaceOpen=frame.workspaceMode!=SandboxWorkspaceMode::Flight;
    const bool travelVisible=frame.vectorTravelStage==VectorTravelStage::Aligning||frame.vectorTravelStage==VectorTravelStage::Charging||frame.vectorTravelStage==VectorTravelStage::Cruise||frame.vectorTravelStage==VectorTravelStage::Decelerating||frame.vectorTravelStage==VectorTravelStage::Complete;
    if(!workspaceOpen&&!travelVisible)return;
    const int w=frame.viewportWidth,h=frame.viewportHeight;
    DisableShader();SetupScreenProjection(w,h);glDisable(GL_DEPTH_TEST);glDisable(GL_LIGHTING);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    if(travelVisible){
        const float barW=420.0f,barX=(w-barW)*0.5f;
        FilledRect(barX,58,0,barW,50,{0.006f,0.020f,0.032f,0.88f});
        FilledRect(barX+12,90,0,barW-24,7,{0.04f,0.09f,0.12f,0.96f});
        FilledRect(barX+12,90,0,(barW-24)*static_cast<float>(std::clamp(frame.vectorTravelProgress,0.0,1.0)),7,{0.20f,0.74f,0.86f,0.96f});
        DrawText5x7(frame.vectorTravelStatus.empty()?"VECTOR DRIVE":frame.vectorTravelStatus,barX+18,68,1.10f,{0.64f,0.84f,0.90f,0.94f});
    }

    if(workspaceOpen){
        SandboxWorkspaceSystem workspace;workspace.Open(frame.workspaceMode);
        if(frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilder){
            DrawShipBuilderOverlay(frame,assets);
        }else if(frame.workspaceMode==SandboxWorkspaceMode::GalaxyMap&&frame.galaxyRuntime&&frame.galaxyRuntime->initialized){
            const float left=44,top=58,right=w-44,bottom=h-58;FilledRect(left,top,0,right-left,bottom-top,{0.003f,0.012f,0.022f,0.96f});
            DrawText5x7("GALAXY MAP",left+22,top+20,1.55f,{0.72f,0.88f,0.92f,0.96f});
            DrawText5x7("10K DETERMINISTIC SYSTEM CATALOG",left+22,top+48,.86f,{0.44f,0.66f,0.72f,0.74f});
            const auto& g=*frame.galaxyRuntime;const float yaw=g.camera.yaw*0.0174532925f,pitch=g.camera.pitch*0.0174532925f;const float cyaw=std::cos(yaw),syaw=std::sin(yaw),cp=std::cos(pitch),sp=std::sin(pitch);const float scale=std::max(.015f,2400.0f/std::max(300.0f,g.camera.distance));
            const float gcx=(left+right)*.5f,gcy=(top+bottom)*.54f;const std::size_t step=std::max<std::size_t>(1,g.catalog.size()/1400);
            for(std::size_t i=0;i<g.catalog.size();i+=step){const auto&r=g.catalog[i];const float rx=r.x-g.camera.focusX,ry=r.y-g.camera.focusY,rz=r.z-g.camera.focusZ;const float x1=rx*cyaw-rz*syaw,z1=rx*syaw+rz*cyaw;const float y1=ry*cp-z1*sp;const float sxp=gcx+x1*scale,syp=gcy-y1*scale;if(sxp<left+8||sxp>right-8||syp<top+68||syp>bottom-34)continue;const bool sel=r.id==g.selectedSystem;FilledCircle(sxp,syp,0,sel?3.8f:1.35f,sel?Rgba{1.0f,.66f,.18f,.96f}:Rgba{.38f,.64f,.78f,.54f},sel?16:8);}
            DrawText5x7("RMB DRAG ORBIT   WHEEL ZOOM   UP/DOWN SELECT   ENTER ROUTE",left+22,bottom-30,.82f,{.46f,.66f,.70f,.74f});
        }else if(frame.workspaceMode==SandboxWorkspaceMode::PlanetaryManufacturing&&frame.planetIndustryRuntime){
            const float left=64,top=62,right=w-64,bottom=h-58;FilledRect(left,top,0,right-left,bottom-top,{0.004f,0.016f,0.026f,0.95f});DrawText5x7("PLANETARY INDUSTRY - "+frame.planetIndustryRuntime->planetName,left+22,top+20,1.35f,{.72f,.88f,.92f,.96f});
            const float hx=(left+right)*.5f,hy=(top+bottom)*.52f,hs=28.0f;for(const auto&kv:frame.planetIndustryRuntime->industry.hexes){const auto&hdata=kv.second;const float x=hx+hs*1.5f*hdata.coord.q;const float y=hy+hs*0.8660254f*(2*hdata.coord.r+hdata.coord.q);const bool selected=hdata.coord.q==frame.planetIndustryRuntime->selectedHex.q&&hdata.coord.r==frame.planetIndustryRuntime->selectedHex.r;Ring(x,y,0,hs*.82f,selected?Rgba{1.0f,.68f,.18f,.94f}:Rgba{.22f,.58f,.62f,.60f},selected?2.2f:1.0f,6);if(hdata.surveyed)FilledCircle(x,y,0,3.0f,{.30f,.78f,.58f,.80f},10);}
            std::ostringstream pi;pi<<"TETHER STORAGE "<<static_cast<int>(frame.planetIndustryRuntime->tetherStored)<<"   INSTALLATIONS "<<frame.planetIndustryRuntime->industry.installations.size();DrawText5x7(pi.str(),left+22,bottom-32,.90f,{.52f,.74f,.78f,.82f});
        }else if(frame.workspaceMode==SandboxWorkspaceMode::SystemMap&&frame.systemMap){
            PlayerFacingIntegrationSystem mapUi;
            const auto layout=mapUi.LayoutSystemMap(*frame.systemMap,frame.universeSystemMap,w,h,frame.systemMapZoom,frame.systemMapPan);
            const float left=layout.left,top=layout.top,right=layout.right,bottom=layout.bottom;
            FilledRect(left,top,0,right-left,bottom-top,{0.004f,0.016f,0.026f,0.96f});
            Line(left,top,0,right,top,0,{0.16f,0.62f,0.70f,0.76f},1.5f);
            DrawText5x7("SYSTEM MAP / LIVE EPHEMERIS",left+24,top+20,1.55f,{0.72f,0.88f,0.92f,0.96f});
            DrawText5x7(frame.systemMap->systemName,left+24,top+50,1.05f,{0.46f,0.66f,0.72f,0.84f});
            DrawText5x7("LMB SELECT   RMB ACTIONS   MMB PAN   WHEEL ZOOM   ENTER VECTOR",left+24,bottom-28,.78f,{.50f,.72f,.76f,.86f});
            Line(layout.plotRight+8,top+66,0,layout.plotRight+8,bottom-16,0,{.10f,.34f,.40f,.56f},1.0f);

            const bool live=frame.universeSystemMap&&!frame.universeSystemMap->nodes.empty();
            std::string selectedName;
            if(frame.systemMap->selected<frame.systemMap->nodes.size())selectedName=frame.systemMap->nodes[frame.systemMap->selected].label;

            if(live){
                for(const auto& n:frame.universeSystemMap->nodes){
                    if(n.orbitTrack.size()<2)continue;
                    const auto c=OrbitalBodyColor(n.kind);Rgba track{c.r,c.g,c.b,n.name==selectedName?.42f:.14f};
                    for(std::size_t j=1;j<=n.orbitTrack.size();++j){
                        const auto&a=n.orbitTrack[j-1];const auto&b=n.orbitTrack[j%n.orbitTrack.size()];
                        Line(layout.centerX+a.x*layout.scale,layout.centerY-a.y*layout.scale,0,
                             layout.centerX+b.x*layout.scale,layout.centerY-b.y*layout.scale,0,track,n.name==selectedName?1.5f:.8f);
                    }
                }
                for(const auto& n:frame.universeSystemMap->nodes){
                    const auto it=std::find_if(frame.systemMap->nodes.begin(),frame.systemMap->nodes.end(),[&](const SystemMapNode& m){return m.label==n.name;});
                    const int index=it==frame.systemMap->nodes.end()?-1:static_cast<int>(std::distance(frame.systemMap->nodes.begin(),it));
                    const bool selected=index>=0&&frame.systemMap->selected==static_cast<std::size_t>(index);
                    const bool hovered=index>=0&&frame.systemMapHoveredNode==index;
                    const float x=layout.centerX+n.currentPosition.x*layout.scale,y=layout.centerY-n.currentPosition.y*layout.scale;
                    const auto kind=MapKindForOrbital(n.kind);const auto c=SystemMapNodeColor(kind);
                    DrawSystemMapMarker(kind,x,y,c,selected,hovered);
                    const bool major=kind==SystemMapNodeKind::Star||kind==SystemMapNodeKind::Planet||kind==SystemMapNodeKind::Moon||kind==SystemMapNodeKind::Station;
                    if(major||selected||hovered){
                        DrawText5x7(n.name,x+15,y-8,major?.76f:.82f,selected?Rgba{1.0f,.84f,.48f,.98f}:Rgba{.72f,.84f,.86f,.90f});
                        if(selected||hovered)DrawText5x7(SystemMapSystem::KindName(kind),x+15,y+9,.62f,{c.r,c.g,c.b,.82f});
                    }
                }
            }

            // Non-orbital sites and fallback nodes remain visible over the live hierarchy.
            for(std::size_t i=0;i<frame.systemMap->nodes.size();++i){
                const auto&n=frame.systemMap->nodes[i];if(!n.known)continue;
                if(live&&(n.kind==SystemMapNodeKind::Star||n.kind==SystemMapNodeKind::Planet||n.kind==SystemMapNodeKind::Moon||n.kind==SystemMapNodeKind::Station||n.kind==SystemMapNodeKind::OrbitalHub))continue;
                const float x=layout.centerX+n.position.x*layout.scale,y=layout.centerY-n.position.y*layout.scale;const auto c=SystemMapNodeColor(n.kind);
                const bool selected=frame.systemMap->selected==i,hovered=frame.systemMapHoveredNode==static_cast<int>(i);
                DrawSystemMapMarker(n.kind,x,y,c,selected,hovered);
                if(selected||hovered||n.kind==SystemMapNodeKind::Belt){
                    DrawText5x7(n.label,x+15,y-8,.76f,selected?Rgba{1.0f,.84f,.48f,.98f}:Rgba{.70f,.82f,.84f,.88f});
                    if(selected||hovered)DrawText5x7(SystemMapSystem::KindName(n.kind),x+15,y+9,.62f,{c.r,c.g,c.b,.82f});
                }
            }

            // Right-side selected-object authority: what it is, why it matters,
            // and whether the player can immediately travel there.
            const float infoX=layout.infoLeft+12.0f;float infoY=top+82.0f;
            DrawText5x7("SELECTED OBJECT",infoX,infoY,1.02f,{.68f,.88f,.92f,.96f});infoY+=30;
            const SystemMapNode* selected=frame.systemMap->selected<frame.systemMap->nodes.size()?&frame.systemMap->nodes[frame.systemMap->selected]:nullptr;
            if(selected){
                const auto c=SystemMapNodeColor(selected->kind);
                DrawText5x7(selected->label,infoX,infoY,1.05f,{.92f,.96f,.96f,.98f});infoY+=28;
                DrawText5x7(SystemMapSystem::KindName(selected->kind),infoX,infoY,.86f,{c.r,c.g,c.b,.94f});infoY+=23;
                DrawText5x7(selected->regionLabel,infoX,infoY,.74f,{.56f,.72f,.76f,.88f});infoY+=25;
                std::ostringstream hz;hz<<"HAZARD "<<static_cast<int>(std::clamp(selected->hazard,0.0f,1.0f)*100.0f)<<"%";
                DrawText5x7(hz.str(),infoX,infoY,.78f,selected->hazard>.55f?Rgba{.96f,.42f,.22f,.96f}:Rgba{.62f,.82f,.72f,.90f});infoY+=23;
                DrawText5x7(selected->warpable?"VECTOR CAPABLE":"LOCAL ONLY",infoX,infoY,.78f,selected->warpable?Rgba{.34f,.86f,.68f,.92f}:Rgba{.72f,.58f,.42f,.88f});infoY+=28;
                DrawText5x7("RMB FOR DESTINATION ACTIONS",infoX,infoY,.68f,{.52f,.70f,.74f,.80f});
            }else DrawText5x7("CLICK A MAP OBJECT",infoX,infoY,.84f,{.50f,.68f,.72f,.80f});

            infoY=top+360.0f;DrawText5x7("MAP LEGEND",infoX,infoY,.92f,{.62f,.82f,.86f,.92f});infoY+=28;
            const SystemMapNodeKind legendKinds[]={SystemMapNodeKind::Star,SystemMapNodeKind::Planet,SystemMapNodeKind::Moon,SystemMapNodeKind::Station,SystemMapNodeKind::Belt,SystemMapNodeKind::Signature,SystemMapNodeKind::Salvage};
            for(auto kind:legendKinds){const auto c=SystemMapNodeColor(kind);DrawSystemMapMarker(kind,infoX+8,infoY+5,c,false,false);DrawText5x7(SystemMapSystem::KindName(kind),infoX+28,infoY,.68f,{.62f,.76f,.78f,.84f});infoY+=25;}

            if(frame.systemMapHoveredNode>=0&&static_cast<std::size_t>(frame.systemMapHoveredNode)<frame.systemMap->nodes.size()){
                const auto& hover=frame.systemMap->nodes[static_cast<std::size_t>(frame.systemMapHoveredNode)];
                const float tw=238.0f,th=56.0f;const float tx=std::clamp(frame.pointerX+16.0f,left+8.0f,layout.plotRight-tw-8.0f);const float ty=std::clamp(frame.pointerY+18.0f,top+72.0f,bottom-th-38.0f);
                FilledRect(tx,ty,0,tw,th,{.004f,.020f,.030f,.96f});Line(tx,ty,0,tx+tw,ty,0,{.24f,.72f,.78f,.72f},1.0f);
                DrawText5x7(hover.label,tx+10,ty+10,.78f,{.86f,.94f,.94f,.96f});
                DrawText5x7(SystemMapSystem::KindName(hover.kind),tx+10,ty+31,.66f,{.54f,.76f,.80f,.88f});
            }
        }else{
            const float panelW=680.0f,panelH=360.0f,left=62.0f,top=88.0f;
            FilledRect(left,top,0,panelW,panelH,{0.004f,0.018f,0.030f,0.93f});
            Line(left,top,0,left+panelW,top,0,{0.16f,0.62f,0.70f,0.72f},1.6f);
            DrawText5x7(workspace.Title(),left+26,top+24,1.55f,{0.72f,0.88f,0.92f,0.96f});
            float y=top+70;for(const auto&line:frame.workspaceLines){DrawText5x7(line,left+26,y,0.98f,{0.58f,0.74f,0.78f,0.88f});y+=28;}
            DrawText5x7("RMB ORBIT  WHEEL ZOOM  ESC CLOSE",left+26,top+panelH-34,0.90f,{0.42f,0.62f,0.68f,0.72f});
        }
    }
    glDisable(GL_BLEND);glEnable(GL_DEPTH_TEST);
}
#endif // _WIN32
} // namespace

NativeBattlefieldRenderer::NativeBattlefieldRenderer()
    : _assets(std::make_unique<VisualAssets>())
{
}

NativeBattlefieldRenderer::~NativeBattlefieldRenderer() = default;

bool NativeBattlefieldRenderer::Initialize() {
    if(!_assets)_assets=std::make_unique<VisualAssets>();
    LoadVisualAssets(*_assets);
#ifdef _WIN32
    InitializeSpaceShader();
    InitializeNativeUiFonts();
    LoadImportedPlanetTextures(*_assets);
#endif
    _initialized=true;
    return true;
}

const std::vector<ShipyardModuleRecord>& NativeBattlefieldRenderer::ShipyardCatalog() const {
    static const std::vector<ShipyardModuleRecord> empty;
    return _assets?_assets->shipyardCatalog:empty;
}

void NativeBattlefieldRenderer::ApplyShipyardCatalogAuthoringOverrides(const std::vector<ShipyardModuleRecord>& catalog){
    if(!_assets)return;
    _assets->shipyardCatalog=catalog;
    _assets->canonicalAssetRegistry.Clear();
    ShipyardCanonicalAssetBridge::PopulateRegistry(_assets->canonicalAssetRegistry,_assets->shipyardCatalog);
    auto& recipes=_assets->generatedVisualCatalog.shipRecipes;
    recipes.erase(std::remove_if(recipes.begin(),recipes.end(),[](const auto& recipe){return recipe.sourceFamily=="SHIPYARD_V07_CC0";}),recipes.end());
    const auto refreshed=ShipyardModuleSystem::BuildShowcaseRecipes(_assets->shipyardCatalog,0x5A17C0DEu);
    recipes.insert(recipes.end(),refreshed.begin(),refreshed.end());
    Logger::Instance().Info("Shipyard","Applied persistent/manual socket authoring overrides to runtime catalog: "+std::to_string(catalog.size())+" modules; regenerated "+std::to_string(refreshed.size())+" Shipyard recipes.");
}

ProceduralShipVisualRecipe NativeBattlefieldRenderer::DefaultShipyardRecipe(const std::string& role,
                                                                             std::uint32_t visualSeed) const {
    if(!_assets)return {};
    const auto* recipe=_assets->shipyardReady
        ? ProceduralVisualVariantSystem::SelectSourceFamily(_assets->generatedVisualCatalog,role,"SHIPYARD_V07_CC0",visualSeed)
        : nullptr;
    if(!recipe)recipe=ProceduralVisualVariantSystem::Select(_assets->generatedVisualCatalog,role,visualSeed);
    return recipe?*recipe:ProceduralShipVisualRecipe{};
}

void NativeBattlefieldRenderer::Shutdown() {
#ifdef _WIN32
    if(_assets){
        ReleaseImportedPlanetTextures(*_assets);
        for(auto& kv:_assets->shipTextures){GLuint id=static_cast<GLuint>(kv.second);if(id)glDeleteTextures(1,&id);}
        _assets->shipTextures.clear();
    }
    ShutdownNativeUiFonts();
    ShutdownSpaceShader();
#endif
    if(_assets){_assets->shipModules.clear();_assets->generatedVisualCatalog={};_assets->shipyardCatalog.clear();_assets->canonicalAssetRegistry.Clear();_assets->moduleLibraryReady=false;}
    _initialized=false;
}

Vector3 NativeBattlefieldRenderer::SectorToWorld(const SectorPosition& position) {
    return {position.x*kSectorPositionScale,position.y*kSectorPositionScale,0.0f};
}

SectorPosition NativeBattlefieldRenderer::WorldToSector(const Vector3& position) {
    return {position.x/kSectorPositionScale,position.y/kSectorPositionScale,0.0f};
}

float NativeBattlefieldRenderer::PlanetRadiusToWorld(float radius) {
    // Generic picking radius mirrors the Pass226 2.75x terrestrial visual
    // scale. DrawPlanet3D applies type-specific giant/ice refinements.
    // Keep picking consistent with the reconstructed large flight-space
    // celestial presentation. System-map icons remain independently scaled.
    return std::clamp(radius*0.30f,68.0f,720.0f);
}

Vector3 NativeBattlefieldRenderer::ScreenToWorld(float screenX,float screenY,
                                                  int viewportWidth,int viewportHeight,
                                                  const StrategicCamera& camera) {
    return StrategicViewProjection::ScreenToGameplayPlane(
        screenX,screenY,static_cast<float>(viewportWidth),static_cast<float>(viewportHeight),camera,0.0f);
}

StrategicScreenPoint NativeBattlefieldRenderer::WorldToScreen(const Vector3& worldPoint,
                                                               int viewportWidth,int viewportHeight,
                                                               const StrategicCamera& camera) {
    return StrategicViewProjection::WorldToScreen(
        worldPoint,static_cast<float>(viewportWidth),static_cast<float>(viewportHeight),camera);
}

int NativeBattlefieldRenderer::PickShipyardModule(const std::vector<ShipyardModuleRecord>& catalog,
                                                  const ProceduralShipVisualRecipe& recipe,
                                                  const StrategicCamera& camera,
                                                  int viewportWidth,int viewportHeight,
                                                  float screenX,float screenY,
                                                  float shipX,float shipY,float shipYaw,
                                                  float shipScale,float screenFraction,bool player) {
    if(recipe.modules.empty()||viewportWidth<=0||viewportHeight<=0)return -1;
    const std::string role=recipe.role.empty()?"INDUSTRIAL":recipe.role;
    const auto presentation=ForwardSpacePresentationSystem{}.ForShip(role,player,screenFraction);
    const float axisX=shipScale*presentation.widthScale*recipe.widthScale;
    const float axisY=shipScale*presentation.lengthScale*recipe.lengthScale;
    const float axisZ=shipScale;
    constexpr float kPickPi=3.14159265358979323846f;
    const float globalYaw=shipYaw+recipe.forwardVisualYawDegrees*kPickPi/180.0f;
    auto findRecord=[&](const std::string& id)->const ShipyardModuleRecord*{
        for(const auto& r:catalog)if(r.source.moduleId==id)return &r;
        return nullptr;
    };
    float bestScore=1.0e9f;int best=-1;
    for(std::size_t i=0;i<recipe.modules.size();++i){
        const auto& m=recipe.modules[i];const auto* record=findRecord(m.moduleId);if(!record)continue;
        const float gsy=std::sin(globalYaw),gcy=std::cos(globalYaw);
        const float lx=m.x*axisX,ly=m.y*axisY;
        Vector3 center{shipX+lx*gcy-ly*gsy,shipY+lx*gsy+ly*gcy,0.30f+m.z*axisZ};
        const auto sp=WorldToScreen(center,viewportWidth,viewportHeight,camera);if(!sp.visible)continue;

        const float moduleYaw=globalYaw+m.yawDegrees*kPickPi/180.0f;
        const float cr=std::cos(moduleYaw),sr=std::sin(moduleYaw);
        const float ex=std::max(.02f,record->source.halfWidth*std::fabs(m.scaleX)*axisX);
        const float ey=std::max(.02f,record->source.halfLength*std::fabs(m.scaleY)*axisY);
        const float ez=std::max(.02f,record->source.halfHeight*std::fabs(m.scaleZ)*axisZ);
        const Vector3 rx{cr*ex,sr*ex,0.0f};
        const Vector3 fy{-sr*ey,cr*ey,0.0f};
        const auto sxp=WorldToScreen(center+rx,viewportWidth,viewportHeight,camera);
        const auto syp=WorldToScreen(center+fy,viewportWidth,viewportHeight,camera);
        const auto szp=WorldToScreen(center+Vector3{0,0,ez},viewportWidth,viewportHeight,camera);
        auto screenDistance=[&](const StrategicScreenPoint& q){const float dx=q.x-sp.x,dy=q.y-sp.y;return std::sqrt(dx*dx+dy*dy);};
        float radius=14.0f;
        if(sxp.visible)radius=std::max(radius,screenDistance(sxp));
        if(syp.visible)radius=std::max(radius,screenDistance(syp));
        if(szp.visible)radius=std::max(radius,screenDistance(szp));
        radius=std::clamp(radius*1.18f,14.0f,180.0f);
        const float dx=screenX-sp.x,dy=screenY-sp.y,dist=std::sqrt(dx*dx+dy*dy);
        if(dist>radius)continue;
        const float score=dist/radius+std::max(0.0f,sp.depth)*.0001f;
        if(score<bestScore){bestScore=score;best=static_cast<int>(i);}
    }
    return best;
}

NativeContactSelection NativeBattlefieldRenderer::PickContact(const GalaxySector& sector,
                                                               const Vector3& worldPoint,
                                                               float maxWorldDistance,
                                                               const std::vector<RuntimeStationContact>* stationContacts) {
    NativeContactSelection best;
    float bestDistance=maxWorldDistance;
    for(std::size_t i=0;i<sector.ships.size();++i){
        const float d=Distance2D(SectorToWorld(sector.ships[i].position),worldPoint);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Ship,i,"ship_"+std::to_string(i)};}
    }
    for(std::size_t i=0;i<sector.planets.size();++i){
        const auto p=SectorToWorld(sector.planets[i].position);
        const float d=std::max(0.0f,Distance2D(p,worldPoint)-PlanetRadiusToWorld(sector.planets[i].radius));
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Planet,i,sector.planets[i].planetId};}
    }
    if(sector.hasStation){
        const float d=Distance2D(SectorToWorld(sector.station.position),worldPoint);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Station,0,sector.station.name};}
    }
    if(stationContacts){
        for(std::size_t i=0;i<stationContacts->size();++i){
            const auto& station=(*stationContacts)[i];
            const float d=Distance2D(station.position,worldPoint);
            if(d<bestDistance){bestDistance=d;best={NativeContactKind::Station,i+1,station.name};}
        }
    }
    for(std::size_t i=0;i<sector.derelicts.size();++i){
        const float d=Distance2D(SectorToWorld(sector.derelicts[i].position),worldPoint);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Derelict,i,sector.derelicts[i].derelictId};}
    }
    for(std::size_t i=0;i<sector.orbitalHubs.size();++i){
        const float d=Distance2D(SectorToWorld(sector.orbitalHubs[i].position),worldPoint);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::OrbitalHub,i,sector.orbitalHubs[i].hubId};}
    }
    for(std::size_t i=0;i<sector.asteroids.size();++i){
        const float radius=std::clamp(sector.asteroids[i].size*0.019f,0.20f,0.82f);
        const float d=std::max(0.0f,Distance2D(SectorToWorld(sector.asteroids[i].position),worldPoint)-radius);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Asteroid,i,"asteroid_"+std::to_string(i)};}
    }
    for(std::size_t i=0;i<sector.pointsOfInterest.size();++i){
        const float d=Distance2D(SectorToWorld(sector.pointsOfInterest[i].position),worldPoint);
        if(d<bestDistance){bestDistance=d;best={NativeContactKind::Site,i,sector.pointsOfInterest[i].siteId};}
    }
    return best;
}

void NativeBattlefieldRenderer::Render(const NativeBattlefieldFrame& frame) {
#ifdef _WIN32
    if(!_initialized||!_assets)return;
    if(frame.frontendScreen!=FrontendScreen::InGame){DrawFrontend(frame);return;}
    if(!frame.sector||!frame.camera)return;
    gRenderTime=frame.elapsedSeconds;

    if(frame.standaloneShipyard)DrawStandaloneShipyardBackdrop(frame);
    DrawStarfield(frame);
    DrawVectorTravelBackdrop(frame);
    SetupPerspectiveProjection(frame);
    ConfigureSolarLighting(frame);
    SetupSceneLighting();
    glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    if(frame.embodimentMode==ShipEmbodimentMode::DockedHangar||frame.dockingStage==DockingExperienceStage::Docked){
        DrawHangarBay(frame);
        if(frame.playerPhysics){
            const auto&p=*frame.playerPhysics;
            const auto* dockRecipe=frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilderRecipe?frame.shipBuilderRecipe:frame.playerShipRecipe;
            const int selected=frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilder&&!frame.shipBuilder->recipe.modules.empty()?static_cast<int>(frame.shipBuilder->selectedPlacedModule):-1;
            DrawModularShip(*_assets,p.position.x,p.position.y,0.30f,p.rotation.z,0.24f,true,"INDUSTRIAL",{0.34f,0.39f,0.43f,1.0f},1.0f,0.30f,1u,frame.shipInspection,dockRecipe,selected,frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder?frame.shipBuilderAppearance:frame.playerShipAppearance);
        }
        glDisable(GL_BLEND);
        DrawHud(frame,_assets.get());
        DrawWorkspaceOverlay(frame,_assets.get());
        if(frame.workspaceMode!=SandboxWorkspaceMode::Flight)DrawContextMenuOverlay(frame);
        return;
    }

    const bool compressedCruise=frame.vectorTravelStage==VectorTravelStage::Cruise;
    const auto runtimeVisible=[&](const Vector3& world,float pixelMargin=160.0f){
        if(!frame.camera)return true;
        const auto sp=StrategicViewProjection::WorldToScreen(world,static_cast<float>(frame.viewportWidth),static_cast<float>(frame.viewportHeight),*frame.camera);
        return sp.depth>0.0f&&sp.x>=-pixelMargin&&sp.x<=static_cast<float>(frame.viewportWidth)+pixelMargin&&sp.y>=-pixelMargin&&sp.y<=static_cast<float>(frame.viewportHeight)+pixelMargin;
    };
    // During sustained slipstream the ordinary local scene is suppressed so
    // the animated tunnel is a real travel environment rather than a texture
    // pasted over planets and asteroids. Deceleration renders the resolved
    // destination scene beneath the fading tunnel.
    if(!compressedCruise&&!frame.standaloneShipyard){
        if(frame.sector->hasStar)DrawSun3D(frame.sector->star);
        gCelestialFrameTelemetry={};const auto celestialStart=std::chrono::steady_clock::now();
        for(std::size_t i=0;i<frame.sector->planets.size();++i)DrawPlanet3D(frame.sector->planets[i],i,*_assets);
        const auto celestialEnd=std::chrono::steady_clock::now();
        if(frame.elapsedSeconds-_assets->lastCelestialTelemetryLog>=8.0f){
            const double cpuMs=std::chrono::duration<double,std::milli>(celestialEnd-celestialStart).count();
            Logger::Instance().Info("Renderer","Pass475 celestial submit: planets="+std::to_string(frame.sector->planets.size())+
                " tiers(N/M/F)="+std::to_string(gCelestialFrameTelemetry.nearBodies)+"/"+std::to_string(gCelestialFrameTelemetry.midBodies)+"/"+std::to_string(gCelestialFrameTelemetry.farBodies)+
                " importedCloudLayers="+std::to_string(gCelestialFrameTelemetry.importedCloudLayers)+" proceduralCloudLayers="+std::to_string(gCelestialFrameTelemetry.proceduralCloudLayers)+
                " cpuSubmitMs="+std::to_string(cpuMs));
            _assets->lastCelestialTelemetryLog=frame.elapsedSeconds;
        }
        DrawLocalRingEnvironment(frame);
        DrawAnomalies(frame);
        DrawTacticalPlane(frame);
        DrawSectorSites(frame);
        DrawDebrisFields(frame);
        for(std::size_t i=0;i<frame.sector->asteroids.size();++i){
            if(frame.fractureSystem&&frame.fractureSystem->IsFractured(i))continue;
            if(!runtimeVisible(SectorToWorld(frame.sector->asteroids[i].position),96.0f))continue;
            DrawAsteroid3D(frame.sector->asteroids[i],static_cast<int>(i));
        }
        if(frame.sector->hasStation)DrawStation3D(*_assets,frame.sector->station);
        if(frame.stationContacts){
            for(std::size_t i=0;i<frame.stationContacts->size();++i){
                const auto& station=(*frame.stationContacts)[i];if(!runtimeVisible(station.position,260.0f))continue;DrawRuntimeStation3D(*_assets,station);
                if(frame.selection.kind==NativeContactKind::Station&&frame.selection.index==i+1){DisableSceneLighting();Ring(station.position.x,station.position.y,0.08f,4.2f,{1.0f,0.64f,0.16f,0.76f},1.6f,40);SetupSceneLighting();}
            }
        }
        for(const auto& hub:frame.sector->orbitalHubs)if(runtimeVisible(SectorToWorld(hub.position),140.0f))DrawOrbitalHub3D(hub);
        for(const auto& d:frame.sector->derelicts)if(runtimeVisible(SectorToWorld(d.position),180.0f))DrawDerelict3D(*_assets,d);
        DrawMissilesAndMiningEffects(frame);
        DrawDockingGuidance(frame);
        DrawObservableWarpEvidence(frame);
    }

    // Native traffic now uses the same module library as the player instead of
    // flat icon silhouettes. Role strings only choose an original composition.
    if(!compressedCruise&&!frame.standaloneShipyard) for(std::size_t i=0;i<frame.sector->ships.size();++i){
        const auto& ship=frame.sector->ships[i];
        if(ship.claimed) continue;
        const Vector3 p=SectorToWorld(ship.position);
        if(!runtimeVisible(p,180.0f))continue;
        const Rgba base=ship.disabled?Rgba{0.27f,0.29f,0.30f,1.0f}:(ship.hostile?Rgba{0.54f,0.18f,0.14f,1.0f}:Rgba{0.28f,0.39f,0.47f,1.0f});
        ProceduralShipVisualRecipe authoredRecipe;
        const ProceduralShipVisualRecipe* trafficRecipe=nullptr;
        if(!ship.authoredShipId.empty()) if(const auto* def=ShipyardAuthoredShipSystem::Find(ship.authoredShipId)){authoredRecipe=ShipyardAuthoredShipSystem::BuildRecipe(*def);trafficRecipe=&authoredRecipe;}
        if(!trafficRecipe)trafficRecipe=ResolveShipRecipe(*_assets,false,ship.shipType,ship.visualSeed);
        const auto trafficAxis=ResolveShipAxisScale(0.14f,ship.shipType,false,0.032f,trafficRecipe);
        DrawModularShip(*_assets,p.x,p.y,0.22f,ship.heading,0.14f,false,ship.shipType,base,1.0f,0.032f,ship.visualSeed,false,trafficRecipe);
        if(frame.selection.kind==NativeContactKind::Ship&&frame.selection.index==i){
            DisableSceneLighting();Ring(p.x,p.y,0.08f,1.35f,{1.0f,0.64f,0.16f,0.72f},1.5f,36);SetupSceneLighting();
        }
    }

    // Pass409 runtime closure: the immediate four-ship wing is visible in
    // flight-space using the same modular ship renderer. Offsets are expressed
    // in player-local right/forward axes so formations follow heading instead
    // of remaining painted onto world coordinates.
    if(!compressedCruise&&frame.playerPhysics&&frame.fleetCaptainRuntime&&!frame.fleetCaptainRuntime->ships.empty()){
        for(const auto& wing:frame.fleetCaptainRuntime->ships){
            if(!wing.operational)continue;const Vector3 wp=wing.position;if(!runtimeVisible(wp,180.0f))continue;const float yaw=wing.headingRadians;
            const char* role="ESCORT";switch(wing.role){case FleetShipRole::Mining:role="MINING";break;case FleetShipRole::Support:role="MULTIPURPOSE";break;case FleetShipRole::Salvage:role="SALVAGE";break;default:role="COMBAT";break;}
            const auto wingSeed=static_cast<std::uint32_t>(wing.shipId*97u+17u);
            const auto* wingRecipe=ResolveShipRecipe(*_assets,false,role,wingSeed);
            const auto wingAxis=ResolveShipAxisScale(0.18f,role,false,0.048f,wingRecipe);
            DrawModularShip(*_assets,wp.x,wp.y,0.24f,yaw,0.18f,false,role,{0.27f,0.39f,0.45f,1.0f},1.0f,0.048f,wingSeed,false,wingRecipe);
        }
    }

    if(frame.standaloneShipyard&&!frame.playerPhysics&&frame.shipBuilderRecipe){
        const std::string previewRole=frame.shipBuilderRecipe->role.empty()?"INDUSTRIAL":frame.shipBuilderRecipe->role;
        const int selected=frame.shipBuilder&&!frame.shipBuilder->recipe.modules.empty()?static_cast<int>(frame.shipBuilder->selectedPlacedModule):-1;
        const ShipyardModuleRecord* selectedRecord=nullptr;int selectedSocket=-1;bool socketEdit=false;
        if(frame.shipBuilder&&!frame.shipBuilder->recipe.modules.empty()&&selected>=0){
            const auto& moduleId=frame.shipBuilder->recipe.modules[static_cast<std::size_t>(selected)].moduleId;
            for(const auto& record:frame.shipBuilder->catalog)if(record.source.moduleId==moduleId){selectedRecord=&record;break;}
            if(selectedRecord&&!selectedRecord->sockets.empty())selectedSocket=static_cast<int>(std::min(frame.shipBuilder->selectedSocket,selectedRecord->sockets.size()-1));
            socketEdit=frame.shipBuilder->inspectorTab==ShipyardInspectorTab::Sockets;
        }
        DrawModularShip(*_assets,0.0f,0.0f,0.30f,0.0f,0.24f,true,previewRole,{0.34f,0.39f,0.43f,1.0f},1.0f,0.22f,frame.shipBuilderRecipe->seed,false,frame.shipBuilderRecipe,selected,frame.shipBuilderAppearance,selectedRecord,selectedSocket,socketEdit,frame.shipBuilder&&frame.shipBuilder->dragPreview.active?&frame.shipBuilder->dragPreview.ghost:nullptr,frame.shipBuilder&&frame.shipBuilder->dragPreview.mirroredPreviewActive?&frame.shipBuilder->dragPreview.mirroredGhost:nullptr);
    }

    if(frame.playerPhysics){
        const auto& p=*frame.playerPhysics;
        const auto* playerRecipe=frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilderRecipe
            ? frame.shipBuilderRecipe
            : (frame.playerShipRecipe?frame.playerShipRecipe:ResolveShipRecipe(*_assets,true,"INDUSTRIAL",1u));
        const std::string playerRole=(playerRecipe&&!playerRecipe->role.empty())?playerRecipe->role:"INDUSTRIAL";
        const auto playerAxis=ResolveShipAxisScale(0.24f,playerRole,true,0.22f,playerRecipe);
        const float alpha=frame.cutaway.visible?std::max(0.30f,frame.cutaway.exteriorShellAlpha):1.0f;
        if(frame.cutaway.visible)glDepthMask(GL_FALSE);
        const int selected=frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilder&&!frame.shipBuilder->recipe.modules.empty()?static_cast<int>(frame.shipBuilder->selectedPlacedModule):-1;
        if(frame.embodimentMode==ShipEmbodimentMode::InteriorOnFoot){
            if(frame.cutaway.visible)glDepthMask(GL_TRUE);
            DrawPlayableInterior(frame);
        }else{
            DrawModularShip(*_assets,p.position.x,p.position.y,0.30f,p.rotation.z,0.24f,true,playerRole,{0.34f,0.39f,0.43f,1.0f},alpha,0.22f,1u,frame.shipInspection,playerRecipe,selected,frame.playerShipAppearance,nullptr,-1,false,frame.workspaceMode==SandboxWorkspaceMode::ShipBuilder&&frame.shipBuilder&&frame.shipBuilder->dragPreview.active?&frame.shipBuilder->dragPreview.ghost:nullptr,frame.shipBuilder&&frame.shipBuilder->dragPreview.mirroredPreviewActive?&frame.shipBuilder->dragPreview.mirroredGhost:nullptr);
            if(frame.commandHud.shieldOnline)DrawShipProfileShield(*_assets,p,playerRecipe,playerAxis,frame.commandHud.shield,frame.elapsedSeconds,frame);
            // Shipyard propulsion geometry is authoritative; only exhaust effects
            // are emitted from authored propulsion sockets.
            DrawAuthoredPropulsionEffects(*_assets,p,*frame.input,playerRecipe,playerAxis,0.30f,
                                          frame.vectorVisual.engineOverdrive,frame.elapsedSeconds);
            if(frame.cutaway.visible){glDepthMask(GL_TRUE);DrawInteriorCutaway(p,p.rotation.z);}
        }
    }

    glDisable(GL_BLEND);
    // Vector presentation overlays the ordinary scene during compression and
    // fades during exit, revealing the actual destination local scene below.
    DrawVectorTravelForeground(frame);
    if(!compressedCruise&&!frame.standaloneShipyard)DrawWorldLabels(frame);
    if(!frame.standaloneShipyard){DrawQueuedUI(frame);DrawHud(frame,_assets.get());}
    DrawWorkspaceOverlay(frame,_assets.get());
    if(frame.workspaceMode!=SandboxWorkspaceMode::Flight)DrawContextMenuOverlay(frame);
#else
    (void)frame;
#endif
}

} // namespace subspace
