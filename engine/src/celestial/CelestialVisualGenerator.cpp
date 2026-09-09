#include "celestial/CelestialVisualGenerator.h"
#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {

constexpr float kPi = 3.14159265358979323846f;

void AddPrimitive(RuntimeVisualProfile& profile, RuntimeVisualPrimitive primitive) {
    profile.primitives.push_back(std::move(primitive));
}

RuntimeVisualPrimitive MakeCircle(const std::string& id,
                                  RuntimeVisualLayer layer,
                                  float radius,
                                  std::uint32_t fill,
                                  std::uint32_t stroke,
                                  bool filled = true) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Circle;
    primitive.layer = layer;
    primitive.id = id;
    primitive.radius = radius;
    primitive.fillColor = fill;
    primitive.strokeColor = stroke;
    primitive.filled = filled;
    return primitive;
}

RuntimeVisualPrimitive MakeRing(const std::string& id, float radius, std::uint32_t stroke) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Ring;
    primitive.layer = RuntimeVisualLayer::Shield;
    primitive.id = id;
    primitive.radius = radius;
    primitive.strokeColor = stroke;
    primitive.fillColor = 0;
    primitive.filled = false;
    return primitive;
}

RuntimeVisualPrimitive MakeDiscBand(const std::string& id,
                                    float radius,
                                    float y,
                                    float height,
                                    std::uint32_t fill,
                                    std::uint32_t stroke,
                                    RuntimeVisualLayer layer = RuntimeVisualLayer::Detail) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Polygon;
    primitive.layer = layer;
    primitive.id = id;
    primitive.fillColor = fill;
    primitive.strokeColor = stroke;
    primitive.filled = true;

    const float halfHeight = height * 0.5f;
    const float yTop = std::max(-radius * 0.92f, y - halfHeight);
    const float yBottom = std::min(radius * 0.92f, y + halfHeight);
    const float topChord = std::sqrt(std::max(0.0f, radius * radius - yTop * yTop));
    const float bottomChord = std::sqrt(std::max(0.0f, radius * radius - yBottom * yBottom));
    const float topInset = radius * 0.04f;
    const float bottomInset = radius * 0.04f;
    primitive.points.push_back({-topChord + topInset, yTop});
    primitive.points.push_back({ topChord - topInset, yTop});
    primitive.points.push_back({ bottomChord - bottomInset, yBottom});
    primitive.points.push_back({-bottomChord + bottomInset, yBottom});
    return primitive;
}

RuntimeVisualPrimitive MakeEllipticalRing(const std::string& id,
                                          float width,
                                          float height,
                                          std::uint32_t stroke,
                                          RuntimeVisualLayer layer = RuntimeVisualLayer::Shield) {
    RuntimeVisualPrimitive primitive = MakeRing(id, std::max(width, height) * 0.5f, stroke);
    primitive.layer = layer;
    primitive.size = {width, height};
    return primitive;
}

RuntimeVisualPrimitive MakeIrregularLandPatch(const std::string& id,
                                              float radius,
                                              std::uint32_t seed,
                                              int index,
                                              std::uint32_t fill) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Polygon;
    primitive.layer = RuntimeVisualLayer::Detail;
    primitive.id = id;
    primitive.fillColor = fill;
    primitive.strokeColor = fill;
    const float centerAngle = CelestialSeededUnit(seed, 100 + index) * kPi * 2.0f;
    const float centerOffset = radius * (0.12f + CelestialSeededUnit(seed, 120 + index) * 0.42f);
    const float cx = std::cos(centerAngle) * centerOffset;
    const float cy = std::sin(centerAngle) * centerOffset;
    const int points = 6;
    const float patchRadius = radius * (0.12f + CelestialSeededUnit(seed, 140 + index) * 0.16f);
    for (int i = 0; i < points; ++i) {
        const float angle = (static_cast<float>(i) / static_cast<float>(points)) * kPi * 2.0f;
        const float wobble = 0.72f + CelestialSeededUnit(seed, 180 + index * 11 + i) * 0.48f;
        primitive.points.push_back({cx + std::cos(angle) * patchRadius * wobble,
                                    cy + std::sin(angle) * patchRadius * wobble});
    }
    return primitive;
}

} // namespace

RuntimeVisualProfile BuildCelestialVisualProfile(const CelestialBodyDefinition& body,
                                                 const CelestialVisualOptions& options) {
    RuntimeVisualProfile profile;
    profile.id = body.id.empty() ? "celestial-body" : body.id;
    profile.displayName = body.displayName.empty() ? CelestialBodyTypeName(body.type) : body.displayName;
    profile.entityType = RuntimeVisualEntityType::CelestialBody;
    profile.sourceAuthority = "CelestialBodyDefinition/PixelPlanetsPortPlan";
    profile.seed = body.seed;
    profile.tags = body.gameplayTags;

    const float r = body.visualRadius;

    if (options.includeOrbitRing && body.orbitRadius > 0.0f) {
        RuntimeVisualPrimitive orbit = MakeRing("orbit-ring", body.orbitRadius * options.orbitRingScale, 0x223348);
        orbit.layer = RuntimeVisualLayer::Shadow;
        AddPrimitive(profile, orbit);
    }

    switch (body.type) {
        case CelestialBodyType::Star: {
            AddPrimitive(profile, MakeCircle("star-corona", RuntimeVisualLayer::Shadow, r * 1.42f, body.palette.atmosphere, body.palette.atmosphere, false));
            AddPrimitive(profile, MakeCircle("star-body", RuntimeVisualLayer::Body, r, body.palette.primary, body.palette.accent, true));
            AddPrimitive(profile, MakeCircle("star-core", RuntimeVisualLayer::Accent, r * 0.52f, body.palette.secondary, body.palette.secondary, true));
            break;
        }
        case CelestialBodyType::BlackHole: {
            AddPrimitive(profile, MakeRing("accretion-outer", r * 1.35f, body.palette.accent));
            AddPrimitive(profile, MakeRing("accretion-inner", r * 0.92f, body.palette.secondary));
            AddPrimitive(profile, MakeCircle("event-horizon", RuntimeVisualLayer::Body, r * 0.58f, body.palette.primary, body.palette.shadow, true));
            break;
        }
        case CelestialBodyType::GasGiant:
        case CelestialBodyType::RingedGasGiant: {
            AddPrimitive(profile, MakeCircle("gas-body", RuntimeVisualLayer::Body, r, body.palette.primary, body.palette.shadow, true));
            for (int band = 0; band < 5; ++band) {
                const float y = -r * 0.55f + static_cast<float>(band) * r * 0.28f;
                const std::uint32_t color = (band % 2 == 0) ? body.palette.secondary : body.palette.accent;
                AddPrimitive(profile, MakeDiscBand("gas-band-" + std::to_string(band), r, y, r * 0.13f, color, color));
            }
            if (body.hasRings) {
                AddPrimitive(profile, MakeEllipticalRing("planet-rings", r * 3.25f, r * 0.92f, body.palette.accent));
            }
            break;
        }
        case CelestialBodyType::AsteroidBelt: {
            AddPrimitive(profile, MakeRing("belt-inner", r * 1.8f, body.palette.secondary));
            AddPrimitive(profile, MakeRing("belt-outer", r * 2.8f, body.palette.accent));
            for (int rock = 0; rock < 10; ++rock) {
                const float angle = CelestialSeededUnit(body.seed, rock + 40) * kPi * 2.0f;
                const float dist = r * (1.9f + CelestialSeededUnit(body.seed, rock + 50) * 0.8f);
                RuntimeVisualPrimitive pebble = MakeCircle("belt-rock-" + std::to_string(rock), RuntimeVisualLayer::Detail,
                                                           2.0f + CelestialSeededUnit(body.seed, rock + 60) * 4.0f,
                                                           body.palette.primary, body.palette.primary, true);
                pebble.center = {std::cos(angle) * dist, std::sin(angle) * dist};
                AddPrimitive(profile, pebble);
            }
            break;
        }
        default: {
            AddPrimitive(profile, MakeCircle("planet-body", RuntimeVisualLayer::Body, r, body.palette.primary, body.palette.shadow, true));
            if (body.hasAtmosphere) {
                AddPrimitive(profile, MakeCircle("atmosphere", RuntimeVisualLayer::Shield, r * 1.1f, body.palette.atmosphere, body.palette.atmosphere, false));
            }
            const int patchCount = body.type == CelestialBodyType::LavaWorld ? 5 : 4;
            for (int i = 0; i < patchCount; ++i) {
                const std::uint32_t patchColor = body.type == CelestialBodyType::LavaWorld
                    ? ((i % 2 == 0) ? body.palette.accent : body.palette.secondary)
                    : ((i % 2 == 0) ? body.palette.secondary : body.palette.accent);
                AddPrimitive(profile, MakeIrregularLandPatch("surface-patch-" + std::to_string(i), r, body.seed, i, patchColor));
            }
            if (body.hasClouds) {
                for (int cloud = 0; cloud < 3; ++cloud) {
                    RuntimeVisualPrimitive c = MakeDiscBand("cloud-band-" + std::to_string(cloud),
                                                            r,
                                                            -r * 0.35f + static_cast<float>(cloud) * r * 0.32f,
                                                            r * 0.07f,
                                                            0xEAFBFF,
                                                            0xEAFBFF,
                                                            RuntimeVisualLayer::Accent);
                    AddPrimitive(profile, c);
                }
            }
            break;
        }
    }

    profile.boundsMin = {-r * 3.0f, -r * 3.0f};
    profile.boundsMax = {r * 3.0f, r * 3.0f};
    return profile;
}

RuntimeVisualProfile BuildStarSystemBackdropVisualProfile(const StarSystemDefinition& system,
                                                         const CelestialVisualOptions& options) {
    RuntimeVisualProfile profile = BuildCelestialVisualProfile(system.primary, options);
    profile.id = system.id + ":backdrop";
    profile.displayName = system.displayName + " Backdrop";
    profile.sourceAuthority = "StarSystemDefinition.backdrop";
    profile.tags.push_back("star-system-backdrop");

    int index = 0;
    for (const auto& body : system.bodies) {
        RuntimeVisualProfile bodyVisual = BuildCelestialVisualProfile(body, {});
        const float angle = body.orbitAngleRadians;
        const float distance = body.orbitRadius * 0.12f;
        const float offsetX = std::cos(angle) * distance;
        const float offsetY = std::sin(angle) * distance;
        for (auto primitive : bodyVisual.primitives) {
            primitive.id = "body-" + std::to_string(index) + ":" + primitive.id;
            primitive.center.x += offsetX;
            primitive.center.y += offsetY;
            for (auto& point : primitive.points) {
                point.x += offsetX;
                point.y += offsetY;
            }
            AddPrimitive(profile, primitive);
        }
        ++index;
    }

    return profile;
}

} // namespace subspace
