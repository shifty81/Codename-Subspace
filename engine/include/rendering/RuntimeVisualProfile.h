#pragma once

#include "ships/ModularShipFactory.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

// RuntimeVisualProfile is the first bridge between converted gameplay/content
// data and the lightweight playable client renderer.  It intentionally stores
// renderer-neutral primitive data so the current Win32/GDI client, a future
// OpenGL/D3D backend, and developer-editor overlays can all consume the same
// visual authority without hardcoding every silhouette in the client file.
enum class RuntimeVisualEntityType {
    Unknown,
    PlayerShip,
    NpcShip,
    Station,
    Asteroid,
    CargoPod,
    CelestialBody,
    Projectile,
    Effect
};

enum class RuntimeVisualPrimitiveKind {
    Polygon,
    Box,
    Diamond,
    Circle,
    Ring,
    Line,
    TextAnchor
};

enum class RuntimeVisualLayer {
    Shadow,
    Body,
    Detail,
    Accent,
    Engine,
    Hardpoint,
    Shield,
    Label
};

struct RuntimeVisualPoint2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct RuntimeVisualPrimitive {
    RuntimeVisualPrimitiveKind kind = RuntimeVisualPrimitiveKind::Polygon;
    RuntimeVisualLayer layer = RuntimeVisualLayer::Body;
    std::string id;
    std::string sourceModuleId;
    std::string semanticRole;
    std::vector<RuntimeVisualPoint2> points;
    RuntimeVisualPoint2 center{};
    RuntimeVisualPoint2 size{1.0f, 1.0f};
    float radius = 1.0f;
    float rotationRadians = 0.0f;
    std::uint32_t fillColor = 0xFFFFFF;
    std::uint32_t strokeColor = 0xFFFFFF;
    bool filled = true;
};

struct RuntimeVisualProfile {
    std::string id;
    std::string displayName;
    RuntimeVisualEntityType entityType = RuntimeVisualEntityType::Unknown;
    std::string sourceAuthority;
    std::uint32_t seed = 0;
    RuntimeVisualPoint2 boundsMin{};
    RuntimeVisualPoint2 boundsMax{};
    std::vector<RuntimeVisualPrimitive> primitives;
    std::vector<std::string> tags;

    bool Empty() const;
    std::size_t PrimitiveCount() const;
};

struct RuntimeShipVisualOptions {
    float moduleScale = 11.0f;
    std::uint32_t bodyColor = 0x2EB8E0;
    std::uint32_t accentColor = 0xFFDC60;
    std::uint32_t darkColor = 0x114B68;
    bool includeShieldRing = true;
};

RuntimeVisualProfile BuildShipVisualProfileFromModules(const ModularGeneratedShip& ship,
                                                       const RuntimeShipVisualOptions& options = {});
RuntimeVisualProfile BuildUlyssesStarterVisualProfile(const RuntimeShipVisualOptions& options = {});
RuntimeVisualProfile BuildDefaultStationVisualProfile(std::uint32_t seed = 0);
RuntimeVisualProfile BuildAsteroidVisualProfile(float radius, std::uint32_t seed);
RuntimeVisualProfile BuildCargoPodVisualProfile(std::uint32_t seed = 0);
std::string RuntimeVisualEntityTypeName(RuntimeVisualEntityType type);
std::string RuntimeVisualSummary(const RuntimeVisualProfile& profile);

} // namespace subspace
