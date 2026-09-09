#pragma once

#include "core/Math.h"

#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace::assets {

using AssetIndex = std::uint32_t;
constexpr AssetIndex kInvalidAssetIndex = std::numeric_limits<AssetIndex>::max();
constexpr std::uint32_t kCanonicalAssetSchemaVersion = 2;

struct Float2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Float4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
};

struct Matrix4 {
    // Column-major, matching glTF/OpenGL convention.
    std::array<float, 16> value{{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    }};

    static Matrix4 Identity() { return {}; }
};

enum class CoordinateBasis : std::uint8_t {
    XRightYForwardZUp,
};

enum class TextureSemantic : std::uint8_t {
    Unknown,
    BaseColor,
    MetallicRoughness,
    Normal,
    Occlusion,
    Emissive,
};

enum class TextureColorSpace : std::uint8_t {
    Unknown,
    Linear,
    SRGB,
};

enum class AlphaMode : std::uint8_t {
    Opaque,
    Mask,
    Blend,
};

enum class MaterialDomain : std::uint8_t {
    Surface,
    Canopy,
    EngineHousing,
    ThrusterCore,
    EmissiveDetail,
};

enum class SurfaceSemantic : std::uint8_t {
    Unknown,
    HullPrimary,
    HullSecondary,
    Trim,
    StructuralMetal,
    Armor,
    CanopyGlass,
    EngineHousing,
    Nozzle,
    ThrusterEmissive,
    Radiator,
    WeaponMetal,
    Sensor,
    InteriorVisible,
    DecalSurface,
    DoNotPaint,
};

struct SurfaceAuthoringPolicy {
    AssetIndex materialIndex = kInvalidAssetIndex;
    SurfaceSemantic semantic = SurfaceSemantic::Unknown;
    bool primaryPaint = false;
    bool secondaryPaint = false;
    bool trimPaint = false;
    bool pattern = false;
    bool decal = false;
    bool factionLivery = false;
    bool emissiveOverride = false;
    bool needsReview = false;
};

enum class SocketType : std::uint8_t {
    Unknown,
    HullForward,
    HullAft,
    HullLateral,
    Adapter,
    EngineCavity,
    EngineMount,
    RcsMount,
    TurretMount,
    UtilityMount,
    SensorMount,
    Docking,
    DetailSurface,
};

enum class SocketSize : std::uint8_t {
    XS,
    S,
    M,
    L,
    XL,
};

enum class SocketAuthority : std::uint8_t {
    Authored,
    Reviewed,
    ManualOverride,
    Inferred,
};

enum class ProxyType : std::uint8_t {
    Mount,
    Clearance,
    Collision,
    Keepout,
    Exhaust,
    TurretSweep,
    DockingClearance,
};

enum class ModuleRole : std::uint8_t {
    Unknown,
    Hull,
    HullAdapter,
    Cockpit,
    Bridge,
    EngineHousing,
    MainEngine,
    RcsThruster,
    Wing,
    StructuralFrame,
    TurretHardpoint,
    WeaponMount,
    UtilityMount,
    Sensor,
    Detail,
    Greeble,
};

struct SourceProvenance {
    std::string sourcePath;
    std::string sourceFormat;
    std::string sourceSha256;
    std::string importer;
    std::string importerVersion;
    std::string importPolicy = "PRESERVE_AUTHORED_OBJECTS";
};

struct TextureReference {
    std::string id;
    std::string path;
    TextureSemantic semantic = TextureSemantic::Unknown;
    TextureColorSpace colorSpace = TextureColorSpace::Unknown;
    bool embedded = false;
};

struct PbrMaterial {
    std::string name;
    MaterialDomain domain = MaterialDomain::Surface;

    Float4 baseColorFactor{1.0f, 1.0f, 1.0f, 1.0f};
    float metallicFactor = 0.0f;
    float roughnessFactor = 1.0f;
    Float4 emissiveFactor{0.0f, 0.0f, 0.0f, 1.0f};
    float emissiveStrength = 1.0f;

    AssetIndex baseColorTexture = kInvalidAssetIndex;
    AssetIndex metallicRoughnessTexture = kInvalidAssetIndex;
    AssetIndex normalTexture = kInvalidAssetIndex;
    AssetIndex occlusionTexture = kInvalidAssetIndex;
    AssetIndex emissiveTexture = kInvalidAssetIndex;

    AlphaMode alphaMode = AlphaMode::Opaque;
    float alphaCutoff = 0.5f;
    bool doubleSided = false;
};

struct StaticVertex {
    Vector3 position{};
    Vector3 normal{};
    Float4 tangent{1.0f, 0.0f, 0.0f, 1.0f};
    Float2 uv0{};
    Float2 uv1{};
    Float4 color0{1.0f, 1.0f, 1.0f, 1.0f};
};

struct MeshPrimitive {
    std::vector<StaticVertex> vertices;
    std::vector<std::uint32_t> indices;
    AssetIndex materialIndex = kInvalidAssetIndex;

    bool hasNormals = false;
    bool hasTangents = false;
    bool hasUv0 = false;
    bool hasUv1 = false;
    bool hasColor0 = false;
};

struct CanonicalMesh {
    std::string name;
    std::vector<MeshPrimitive> primitives;
};

struct CanonicalNode {
    std::string name;
    AssetIndex parentIndex = kInvalidAssetIndex;
    AssetIndex meshIndex = kInvalidAssetIndex;
    Matrix4 localTransform = Matrix4::Identity();
    std::unordered_map<std::string, std::string> extras;
};

struct BoxProxy {
    std::string id;
    ProxyType type = ProxyType::Collision;
    AssetIndex nodeIndex = kInvalidAssetIndex;
    Matrix4 localTransform = Matrix4::Identity();
    Vector3 halfExtents{0.5f, 0.5f, 0.5f};
};

struct ModuleSocket {
    std::string id;
    AssetIndex nodeIndex = kInvalidAssetIndex;
    SocketType type = SocketType::Unknown;
    SocketSize size = SocketSize::M;
    SocketAuthority authority = SocketAuthority::Inferred;

    // Socket frame: +Y outward/mating normal, +Z up, +X right.
    Matrix4 localTransform = Matrix4::Identity();
    std::vector<SocketType> accepts;

    float minInsertionMeters = 0.0f;
    float maxInsertionMeters = 0.0f;

    AssetIndex mountProxyIndex = kInvalidAssetIndex;
    AssetIndex clearanceProxyIndex = kInvalidAssetIndex;
    AssetIndex keepoutProxyIndex = kInvalidAssetIndex;
};

struct ModuleDefinition {
    std::string moduleId;
    ModuleRole role = ModuleRole::Unknown;
    SocketSize size = SocketSize::M;
    AssetIndex rootNodeIndex = kInvalidAssetIndex;
    std::vector<AssetIndex> socketIndices;
};

struct CanonicalAsset {
    std::uint32_t schemaVersion = kCanonicalAssetSchemaVersion;
    std::string assetId;
    SourceProvenance provenance;

    // Canonical Subspace gameplay/render space is right-handed, meters,
    // +X right, +Y forward, +Z up. Authoring formats are converted once by
    // the cooker; runtime assembly never special-cases OBJ/FBX axes.
    bool rightHanded = true;
    float metersPerUnit = 1.0f;
    CoordinateBasis coordinateBasis = CoordinateBasis::XRightYForwardZUp;

    std::vector<TextureReference> textures;
    std::vector<PbrMaterial> materials;
    std::vector<SurfaceAuthoringPolicy> surfacePolicies;
    std::vector<CanonicalMesh> meshes;
    std::vector<CanonicalNode> nodes;
    std::vector<BoxProxy> proxies;
    std::vector<ModuleSocket> sockets;
    std::vector<ModuleDefinition> modules;
};

} // namespace subspace::assets
