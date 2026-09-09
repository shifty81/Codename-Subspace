#include "rendering/RuntimeVisualProfile.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {

constexpr float kPi = 3.14159265358979323846f;

RuntimeVisualPoint2 ToClientPlane(const Vector3& value, float scale) {
    // The ported module factory uses x/y/z in ship-space.  The playable client
    // is top-down 2D, so x becomes lateral and z becomes forward/back.
    return {value.z * scale, value.x * scale};
}

RuntimeVisualPrimitive MakeBox(const std::string& id,
                               const ShipModulePart& part,
                               const ShipModuleDefinition* definition,
                               RuntimeVisualLayer layer,
                               std::uint32_t fill,
                               std::uint32_t stroke,
                               float scale) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Box;
    primitive.layer = layer;
    primitive.id = id;
    primitive.sourceModuleId = part.moduleDefinitionId;
    primitive.semanticRole = definition ? ModuleCategoryName(definition->category) : "Module";
    primitive.center = ToClientPlane(part.position, scale);
    const float width = definition ? std::max(8.0f, definition->size.z * scale) : scale;
    const float height = definition ? std::max(8.0f, definition->size.x * scale) : scale;
    primitive.size = {width, height};
    primitive.fillColor = fill;
    primitive.strokeColor = stroke;
    primitive.filled = true;
    return primitive;
}

RuntimeVisualPrimitive MakePolygon(const std::string& id,
                                   const ShipModulePart& part,
                                   const ShipModuleDefinition* definition,
                                   RuntimeVisualLayer layer,
                                   std::uint32_t fill,
                                   std::uint32_t stroke,
                                   float scale,
                                   const std::vector<RuntimeVisualPoint2>& localPoints) {
    RuntimeVisualPrimitive primitive;
    primitive.kind = RuntimeVisualPrimitiveKind::Polygon;
    primitive.layer = layer;
    primitive.id = id;
    primitive.sourceModuleId = part.moduleDefinitionId;
    primitive.semanticRole = definition ? ModuleCategoryName(definition->category) : "Module";
    primitive.center = ToClientPlane(part.position, scale);
    primitive.fillColor = fill;
    primitive.strokeColor = stroke;
    primitive.filled = true;
    primitive.points.reserve(localPoints.size());
    for (const auto& point : localPoints) {
        primitive.points.push_back({primitive.center.x + point.x, primitive.center.y + point.y});
    }
    return primitive;
}

RuntimeVisualLayer LayerForCategory(ModuleCategory category) {
    switch (category) {
        case ModuleCategory::Engine:
        case ModuleCategory::Thruster:
            return RuntimeVisualLayer::Engine;
        case ModuleCategory::Weapon:
        case ModuleCategory::WeaponMount:
        case ModuleCategory::Mining:
            return RuntimeVisualLayer::Hardpoint;
        case ModuleCategory::Sensor:
        case ModuleCategory::Antenna:
        case ModuleCategory::Decorative:
            return RuntimeVisualLayer::Detail;
        case ModuleCategory::Shield:
            return RuntimeVisualLayer::Shield;
        case ModuleCategory::Wing:
        case ModuleCategory::Tail:
            return RuntimeVisualLayer::Accent;
        default:
            return RuntimeVisualLayer::Body;
    }
}

std::uint32_t ColorForCategory(ModuleCategory category, const RuntimeShipVisualOptions& options) {
    switch (category) {
        case ModuleCategory::Sensor:
            return options.darkColor;
        case ModuleCategory::Engine:
        case ModuleCategory::Thruster:
            return 0x184D70;
        case ModuleCategory::Wing:
        case ModuleCategory::Weapon:
        case ModuleCategory::WeaponMount:
        case ModuleCategory::Mining:
            return options.accentColor;
        case ModuleCategory::Cargo:
            return 0x62C48B;
        case ModuleCategory::Shield:
            return 0x5EDCFF;
        case ModuleCategory::PowerCore:
            return 0x8DDCFF;
        default:
            return options.bodyColor;
    }
}

void ExpandBounds(RuntimeVisualProfile& profile, const RuntimeVisualPrimitive& primitive) {
    auto visit = [&profile](RuntimeVisualPoint2 point) {
        profile.boundsMin.x = std::min(profile.boundsMin.x, point.x);
        profile.boundsMin.y = std::min(profile.boundsMin.y, point.y);
        profile.boundsMax.x = std::max(profile.boundsMax.x, point.x);
        profile.boundsMax.y = std::max(profile.boundsMax.y, point.y);
    };

    if (!primitive.points.empty()) {
        for (const auto& point : primitive.points) {
            visit(point);
        }
        return;
    }

    visit({primitive.center.x - primitive.size.x * 0.5f - primitive.radius,
           primitive.center.y - primitive.size.y * 0.5f - primitive.radius});
    visit({primitive.center.x + primitive.size.x * 0.5f + primitive.radius,
           primitive.center.y + primitive.size.y * 0.5f + primitive.radius});
}

float SeededUnit(std::uint32_t seed, int index) {
    std::uint32_t value = seed + static_cast<std::uint32_t>(index) * 747796405u + 2891336453u;
    value = ((value >> ((value >> 28u) + 4u)) ^ value) * 277803737u;
    value = (value >> 22u) ^ value;
    return static_cast<float>(value & 0xffffu) / 65535.0f;
}

} // namespace

bool RuntimeVisualProfile::Empty() const {
    return primitives.empty();
}

std::size_t RuntimeVisualProfile::PrimitiveCount() const {
    return primitives.size();
}

RuntimeVisualProfile BuildShipVisualProfileFromModules(const ModularGeneratedShip& ship,
                                                       const RuntimeShipVisualOptions& options) {
    RuntimeVisualProfile profile;
    profile.id = ship.entityId.empty() ? "player-ship-visual" : ship.entityId;
    profile.displayName = ship.name.empty() ? "Runtime Ship" : ship.name;
    profile.entityType = RuntimeVisualEntityType::PlayerShip;
    profile.sourceAuthority = "ModularGeneratedShip";
    profile.seed = ship.config.seed;
    profile.tags.push_back("module-driven");
    profile.tags.push_back(ModularShipRoleName(ship.config.role));
    profile.tags.push_back(ModularShipSizeName(ship.config.size));

    ShipModuleLibrary library;
    library.InitializeBuiltInModules();

    bool firstPrimitive = true;
    auto addPrimitive = [&](RuntimeVisualPrimitive primitive) {
        if (firstPrimitive) {
            profile.boundsMin = primitive.center;
            profile.boundsMax = primitive.center;
            firstPrimitive = false;
        }
        ExpandBounds(profile, primitive);
        profile.primitives.push_back(std::move(primitive));
    };

    int ordinal = 0;
    for (const auto& part : ship.modules) {
        const ShipModuleDefinition* definition = library.GetDefinition(part.moduleDefinitionId);
        const ModuleCategory category = definition ? definition->category : ModuleCategory::Hull;
        const RuntimeVisualLayer layer = LayerForCategory(category);
        const std::uint32_t fill = ColorForCategory(category, options);
        const std::uint32_t stroke = category == ModuleCategory::Wing ? options.accentColor : 0x7DEFFF;
        const std::string id = part.id.empty() ? ("part-" + std::to_string(ordinal)) : part.id;

        const bool looksLikeCockpit = part.moduleDefinitionId.find("cockpit") != std::string::npos;
        if (looksLikeCockpit) {
            addPrimitive(MakePolygon(id, part, definition, RuntimeVisualLayer::Detail, options.darkColor, stroke, options.moduleScale,
                                     {{18.0f, 0.0f}, {2.0f, -10.0f}, {-10.0f, -7.0f}, {-10.0f, 7.0f}, {2.0f, 10.0f}}));
        }
        else if (category == ModuleCategory::Wing) {
            const float sign = part.position.x < 0.0f ? -1.0f : 1.0f;
            addPrimitive(MakePolygon(id, part, definition, layer, fill, stroke, options.moduleScale,
                                     {{0.0f, sign * 3.0f}, {-18.0f, sign * 35.0f}, {22.0f, sign * 18.0f}}));
        }
        else if (category == ModuleCategory::Engine || category == ModuleCategory::Thruster) {
            addPrimitive(MakeBox(id, part, definition, layer, fill, 0x7DEFFF, options.moduleScale));
            RuntimeVisualPrimitive nozzle = MakeBox(id + "-nozzle", part, definition, RuntimeVisualLayer::Engine, 0x0F2436, 0xFFDC60, options.moduleScale);
            nozzle.center.x -= 12.0f;
            nozzle.size = {8.0f, 12.0f};
            addPrimitive(nozzle);
        }
        else if (category == ModuleCategory::Weapon || category == ModuleCategory::WeaponMount || category == ModuleCategory::Mining) {
            RuntimeVisualPrimitive hardpoint = MakeBox(id, part, definition, layer, fill, 0xFFDC60, options.moduleScale);
            hardpoint.size = {10.0f, 8.0f};
            addPrimitive(hardpoint);
        }
        else if (category == ModuleCategory::Sensor || category == ModuleCategory::Antenna) {
            RuntimeVisualPrimitive sensor = MakeBox(id, part, definition, RuntimeVisualLayer::Detail, fill, 0x7DEFFF, options.moduleScale);
            sensor.size = {8.0f, 8.0f};
            addPrimitive(sensor);
        }
        else {
            addPrimitive(MakeBox(id, part, definition, layer, fill, stroke, options.moduleScale));
        }
        ++ordinal;
    }

    if (options.includeShieldRing) {
        RuntimeVisualPrimitive shield;
        shield.kind = RuntimeVisualPrimitiveKind::Ring;
        shield.layer = RuntimeVisualLayer::Shield;
        shield.id = "ship-shield-radius";
        shield.semanticRole = "ShieldRadius";
        shield.center = {0.0f, 0.0f};
        shield.radius = std::max(82.0f, std::max(std::abs(profile.boundsMin.x), std::abs(profile.boundsMax.x)) + 42.0f);
        shield.fillColor = 0x000000;
        shield.strokeColor = 0x4A97B8;
        shield.filled = false;
        addPrimitive(shield);
    }

    return profile;
}


RuntimeVisualProfile BuildUlyssesStarterVisualProfile(const RuntimeShipVisualOptions& options) {
    RuntimeVisualProfile profile;
    profile.id = "ulysses-starter-visual";
    profile.displayName = "Ulysses Starter Ship";
    profile.entityType = RuntimeVisualEntityType::PlayerShip;
    profile.sourceAuthority = "Assets/Models/ships/Ulysses/source/ulysses.blend top-down proxy";
    profile.seed = 116u;
    profile.tags = {"ulysses", "starter", "blend-reference", "top-down-proxy", "socketed-thrusters"};

    bool firstPrimitive = true;
    auto expandPoint = [&](RuntimeVisualPoint2 point) {
        if (firstPrimitive) {
            profile.boundsMin = point;
            profile.boundsMax = point;
            firstPrimitive = false;
            return;
        }
        profile.boundsMin.x = std::min(profile.boundsMin.x, point.x);
        profile.boundsMin.y = std::min(profile.boundsMin.y, point.y);
        profile.boundsMax.x = std::max(profile.boundsMax.x, point.x);
        profile.boundsMax.y = std::max(profile.boundsMax.y, point.y);
    };
    auto add = [&](RuntimeVisualPrimitive primitive) {
        if (!primitive.points.empty()) {
            for (const auto& point : primitive.points) {
                expandPoint(point);
            }
        }
        else {
            const float halfW = primitive.size.x * 0.5f;
            const float halfH = primitive.size.y * 0.5f;
            expandPoint({primitive.center.x - halfW - primitive.radius, primitive.center.y - halfH - primitive.radius});
            expandPoint({primitive.center.x + halfW + primitive.radius, primitive.center.y + halfH + primitive.radius});
        }
        profile.primitives.push_back(std::move(primitive));
    };
    auto polygon = [&](std::string id, RuntimeVisualLayer layer, std::uint32_t fill, std::uint32_t stroke,
                       std::vector<RuntimeVisualPoint2> points, bool filled = true) {
        RuntimeVisualPrimitive primitive;
        primitive.kind = RuntimeVisualPrimitiveKind::Polygon;
        primitive.layer = layer;
        primitive.id = std::move(id);
        primitive.sourceModuleId = "ulysses-blend-proxy";
        primitive.semanticRole = "UlyssesSilhouette";
        primitive.points = std::move(points);
        primitive.fillColor = fill;
        primitive.strokeColor = stroke;
        primitive.filled = filled;
        add(std::move(primitive));
    };
    auto box = [&](std::string id, RuntimeVisualLayer layer, RuntimeVisualPoint2 center, RuntimeVisualPoint2 size,
                   std::uint32_t fill, std::uint32_t stroke, std::string role = "UlyssesDetail") {
        RuntimeVisualPrimitive primitive;
        primitive.kind = RuntimeVisualPrimitiveKind::Box;
        primitive.layer = layer;
        primitive.id = std::move(id);
        primitive.sourceModuleId = "ulysses-blend-proxy";
        primitive.semanticRole = std::move(role);
        primitive.center = center;
        primitive.size = size;
        primitive.fillColor = fill;
        primitive.strokeColor = stroke;
        primitive.filled = true;
        add(std::move(primitive));
    };
    auto ring = [&](std::string id, RuntimeVisualPoint2 center, float radius, std::uint32_t stroke) {
        RuntimeVisualPrimitive primitive;
        primitive.kind = RuntimeVisualPrimitiveKind::Ring;
        primitive.layer = RuntimeVisualLayer::Shield;
        primitive.id = std::move(id);
        primitive.semanticRole = "ShieldRadius";
        primitive.center = center;
        primitive.radius = radius;
        primitive.fillColor = 0x000000;
        primitive.strokeColor = stroke;
        primitive.filled = false;
        add(std::move(primitive));
    };

    // Local +X is forward/nose in the playable client.  This is not yet a true
    // GLB renderer; it is a readable top-down proxy derived from the Ulysses
    // source-model design notes: angular corvette hull, forward canopy, rear
    // engine cluster, side stabilizers, and forward utility hardpoints.
    polygon("shadow", RuntimeVisualLayer::Shadow, 0x06101A, 0x06101A,
            {{76.0f, 0.0f}, {48.0f, -20.0f}, {8.0f, -27.0f}, {-38.0f, -31.0f}, {-80.0f, -22.0f}, {-86.0f, 22.0f}, {-38.0f, 31.0f}, {8.0f, 27.0f}, {48.0f, 20.0f}});
    polygon("main-hull", RuntimeVisualLayer::Body, options.bodyColor, 0x9EEBFF,
            {{74.0f, 0.0f}, {44.0f, -17.0f}, {8.0f, -22.0f}, {-34.0f, -24.0f}, {-72.0f, -16.0f}, {-82.0f, 0.0f}, {-72.0f, 16.0f}, {-34.0f, 24.0f}, {8.0f, 22.0f}, {44.0f, 17.0f}});
    polygon("central-armored-spine", RuntimeVisualLayer::Detail, options.darkColor, 0x6CD6FF,
            {{55.0f, 0.0f}, {30.0f, -7.0f}, {-44.0f, -8.5f}, {-66.0f, 0.0f}, {-44.0f, 8.5f}, {30.0f, 7.0f}});
    polygon("cockpit-canopy", RuntimeVisualLayer::Detail, 0x102B42, 0xBDEEFF,
            {{60.0f, 0.0f}, {42.0f, -8.0f}, {21.0f, -7.0f}, {15.0f, 0.0f}, {21.0f, 7.0f}, {42.0f, 8.0f}});
    polygon("port-stabilizer", RuntimeVisualLayer::Accent, options.accentColor, 0xFFF0A0,
            {{-10.0f, -20.0f}, {-48.0f, -50.0f}, {-66.0f, -26.0f}, {-32.0f, -20.0f}});
    polygon("starboard-stabilizer", RuntimeVisualLayer::Accent, options.accentColor, 0xFFF0A0,
            {{-10.0f, 20.0f}, {-48.0f, 50.0f}, {-66.0f, 26.0f}, {-32.0f, 20.0f}});
    box("port-main-engine", RuntimeVisualLayer::Engine, {-78.0f, -12.0f}, {16.0f, 11.0f}, 0x10283B, 0x72E4FF, "MainThrusterSocket");
    box("starboard-main-engine", RuntimeVisualLayer::Engine, {-78.0f, 12.0f}, {16.0f, 11.0f}, 0x10283B, 0x72E4FF, "MainThrusterSocket");
    box("center-engine-core", RuntimeVisualLayer::Engine, {-86.0f, 0.0f}, {12.0f, 10.0f}, 0x07131E, 0xFFD66A, "MainThrusterSocket");
    box("port-forward-hardpoint", RuntimeVisualLayer::Hardpoint, {34.0f, -23.5f}, {10.0f, 6.0f}, 0xFFE06A, 0xFFF0A0, "WeaponHardpoint");
    box("starboard-forward-hardpoint", RuntimeVisualLayer::Hardpoint, {34.0f, 23.5f}, {10.0f, 6.0f}, 0xFFE06A, 0xFFF0A0, "WeaponHardpoint");
    box("port-rcs-node", RuntimeVisualLayer::Engine, {30.0f, -18.0f}, {6.0f, 5.0f}, 0x1D6A68, 0x8FFFE0, "RcsThrusterSocket");
    box("starboard-rcs-node", RuntimeVisualLayer::Engine, {30.0f, 18.0f}, {6.0f, 5.0f}, 0x1D6A68, 0x8FFFE0, "RcsThrusterSocket");
    box("port-aft-rcs-node", RuntimeVisualLayer::Engine, {-58.0f, -21.0f}, {6.0f, 5.0f}, 0x1D6A68, 0x8FFFE0, "RcsThrusterSocket");
    box("starboard-aft-rcs-node", RuntimeVisualLayer::Engine, {-58.0f, 21.0f}, {6.0f, 5.0f}, 0x1D6A68, 0x8FFFE0, "RcsThrusterSocket");
    box("port-panel-line", RuntimeVisualLayer::Detail, {-20.0f, -12.0f}, {42.0f, 2.0f}, 0x0B3149, 0x0B3149, "PanelLine");
    box("starboard-panel-line", RuntimeVisualLayer::Detail, {-20.0f, 12.0f}, {42.0f, 2.0f}, 0x0B3149, 0x0B3149, "PanelLine");

    if (options.includeShieldRing) {
        ring("ship-shield-radius", {0.0f, 0.0f}, 84.0f, 0x4A97B8);
    }

    return profile;
}

RuntimeVisualProfile BuildDefaultStationVisualProfile(std::uint32_t seed) {
    RuntimeVisualProfile profile;
    profile.id = "starter-station-visual";
    profile.displayName = "Starter Dock";
    profile.entityType = RuntimeVisualEntityType::Station;
    profile.sourceAuthority = "RuntimeVisualProfile.default.station";
    profile.seed = seed;
    profile.tags = {"station", "dock", "starter-sector"};

    auto addBox = [&profile](const std::string& id, RuntimeVisualPoint2 center, RuntimeVisualPoint2 size, std::uint32_t fill) {
        RuntimeVisualPrimitive primitive;
        primitive.kind = RuntimeVisualPrimitiveKind::Box;
        primitive.layer = RuntimeVisualLayer::Body;
        primitive.id = id;
        primitive.center = center;
        primitive.size = size;
        primitive.fillColor = fill;
        primitive.strokeColor = 0xA8C9FF;
        profile.primitives.push_back(primitive);
    };

    addBox("core", {0.0f, 0.0f}, {108.0f, 76.0f}, 0x41568E);
    addBox("hangar", {0.0f, 0.0f}, {56.0f, 36.0f}, 0x192232);
    addBox("left-dock", {-90.0f, 0.0f}, {52.0f, 36.0f}, 0x181F37);
    addBox("right-dock", {90.0f, 0.0f}, {52.0f, 36.0f}, 0x181F37);
    addBox("top-spine", {0.0f, -52.0f}, {36.0f, 28.0f}, 0x181F37);
    addBox("bottom-spine", {0.0f, 52.0f}, {36.0f, 28.0f}, 0x181F37);

    RuntimeVisualPrimitive dockRing;
    dockRing.kind = RuntimeVisualPrimitiveKind::Ring;
    dockRing.layer = RuntimeVisualLayer::Shield;
    dockRing.id = "dock-radius";
    dockRing.center = {0.0f, 0.0f};
    dockRing.radius = 150.0f;
    dockRing.strokeColor = 0xFFD858;
    dockRing.filled = false;
    profile.primitives.push_back(dockRing);

    profile.boundsMin = {-160.0f, -90.0f};
    profile.boundsMax = {160.0f, 90.0f};
    return profile;
}

RuntimeVisualProfile BuildAsteroidVisualProfile(float radius, std::uint32_t seed) {
    RuntimeVisualProfile profile;
    profile.id = "asteroid-" + std::to_string(seed);
    profile.displayName = "Procedural Asteroid";
    profile.entityType = RuntimeVisualEntityType::Asteroid;
    profile.sourceAuthority = "RuntimeVisualProfile.procedural.asteroid";
    profile.seed = seed;
    profile.tags = {"asteroid", "procedural", "resource-node"};

    RuntimeVisualPrimitive body;
    body.kind = RuntimeVisualPrimitiveKind::Polygon;
    body.layer = RuntimeVisualLayer::Body;
    body.id = "asteroid-body";
    body.fillColor = 0x625B52;
    body.strokeColor = 0x27354B;
    for (int i = 0; i < 14; ++i) {
        const float angle = (static_cast<float>(i) / 14.0f) * kPi * 2.0f;
        const float wobble = 0.76f + SeededUnit(seed, i) * 0.42f;
        body.points.push_back({std::cos(angle) * radius * wobble, std::sin(angle) * radius * wobble});
    }
    profile.primitives.push_back(body);

    for (int crater = 0; crater < 4; ++crater) {
        RuntimeVisualPrimitive c;
        c.kind = RuntimeVisualPrimitiveKind::Circle;
        c.layer = RuntimeVisualLayer::Detail;
        c.id = "crater-" + std::to_string(crater);
        const float angle = SeededUnit(seed, crater + 20) * kPi * 2.0f;
        const float offset = radius * (0.18f + SeededUnit(seed, crater + 30) * 0.38f);
        c.center = {std::cos(angle) * offset, std::sin(angle) * offset};
        c.radius = std::max(3.0f, radius * (0.09f + SeededUnit(seed, crater + 40) * 0.10f));
        c.fillColor = 0x3E3A35;
        c.strokeColor = 0x3E3A35;
        profile.primitives.push_back(c);
    }

    profile.boundsMin = {-radius * 1.2f, -radius * 1.2f};
    profile.boundsMax = {radius * 1.2f, radius * 1.2f};
    return profile;
}

RuntimeVisualProfile BuildCargoPodVisualProfile(std::uint32_t seed) {
    RuntimeVisualProfile profile;
    profile.id = "cargo-pod-" + std::to_string(seed);
    profile.displayName = "Cargo Pod";
    profile.entityType = RuntimeVisualEntityType::CargoPod;
    profile.sourceAuthority = "RuntimeVisualProfile.default.cargo";
    profile.seed = seed;
    profile.tags = {"cargo", "pickup"};

    RuntimeVisualPrimitive pod;
    pod.kind = RuntimeVisualPrimitiveKind::Diamond;
    pod.layer = RuntimeVisualLayer::Body;
    pod.id = "cargo-diamond";
    pod.center = {0.0f, 0.0f};
    pod.size = {20.0f, 20.0f};
    pod.fillColor = 0x6DFF9A;
    pod.strokeColor = 0x27354B;
    profile.primitives.push_back(pod);
    profile.boundsMin = {-12.0f, -12.0f};
    profile.boundsMax = {12.0f, 12.0f};
    return profile;
}

std::string RuntimeVisualEntityTypeName(RuntimeVisualEntityType type) {
    switch (type) {
        case RuntimeVisualEntityType::PlayerShip: return "PlayerShip";
        case RuntimeVisualEntityType::NpcShip: return "NpcShip";
        case RuntimeVisualEntityType::Station: return "Station";
        case RuntimeVisualEntityType::Asteroid: return "Asteroid";
        case RuntimeVisualEntityType::CargoPod: return "CargoPod";
        case RuntimeVisualEntityType::CelestialBody: return "CelestialBody";
        case RuntimeVisualEntityType::Projectile: return "Projectile";
        case RuntimeVisualEntityType::Effect: return "Effect";
        default: return "Unknown";
    }
}

std::string RuntimeVisualSummary(const RuntimeVisualProfile& profile) {
    std::ostringstream stream;
    stream << profile.displayName << " [" << RuntimeVisualEntityTypeName(profile.entityType) << "] "
           << profile.primitives.size() << " primitive(s) from " << profile.sourceAuthority;
    return stream.str();
}

} // namespace subspace
