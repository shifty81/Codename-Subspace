#include "station/StationKitbashVisualSystem.h"

#include "station/StationKitbashCatalogSystem.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kRadToDeg = 57.29577951308232f;

std::uint32_t Mix(std::uint64_t value) {
    value ^= value >> 33;
    value *= 0xff51afd7ed558ccdULL;
    value ^= value >> 33;
    value *= 0xc4ceb9fe1a85ec53ULL;
    value ^= value >> 33;
    return static_cast<std::uint32_t>(value ^ (value >> 32));
}

const ShipyardModuleRecord* FindModule(const std::vector<ShipyardModuleRecord>& catalog,
                                       const std::string& id) {
    const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const auto& r) { return r.source.moduleId == id; });
    return it == catalog.end() ? nullptr : &*it;
}

float UniformScaleFor(const ShipyardModuleRecord& r, float targetHalfSpan) {
    const float span = std::max({0.05f, r.source.halfWidth, r.source.halfLength, r.source.halfHeight});
    return std::clamp(targetHalfSpan / span, 0.10f, 5.0f);
}

float ProjectedRadiusXY(const ShipyardModuleRecord& r, const VisualModulePlacement& p, float dx, float dy) {
    const float yaw = p.yawDegrees / kRadToDeg;
    const float cx = std::cos(yaw), sx = std::sin(yaw);
    const float localXx = cx, localXy = sx;
    const float localYx = -sx, localYy = cx;
    const float hx = std::max(0.04f, r.source.halfWidth * std::abs(p.scaleX));
    const float hy = std::max(0.04f, r.source.halfLength * std::abs(p.scaleY));
    return std::abs(dx*localXx + dy*localXy)*hx + std::abs(dx*localYx + dy*localYy)*hy;
}

float HalfHeight(const ShipyardModuleRecord& r, const VisualModulePlacement& p) {
    return std::max(0.04f, r.source.halfHeight * std::abs(p.scaleZ));
}

SpaceMaterialKind MaterialFor(StationKitbashPieceRole role) {
    switch (role) {
        case StationKitbashPieceRole::CommandSpire: return SpaceMaterialKind::Canopy;
        case StationKitbashPieceRole::RefineryPod:
        case StationKitbashPieceRole::ManufacturingPod:
        case StationKitbashPieceRole::TankFarm:
        case StationKitbashPieceRole::IndustrialBrace:
        case StationKitbashPieceRole::RepairBay:
        case StationKitbashPieceRole::ShipyardGantry:
            return SpaceMaterialKind::IndustrialHull;
        default: return SpaceMaterialKind::StationHull;
    }
}

float TargetSpan(StationKitbashPieceRole role, bool asteroidEmbedded) {
    const float compact = asteroidEmbedded ? 0.78f : 1.0f;
    switch (role) {
        case StationKitbashPieceRole::CoreHub: return 1.75f * compact;
        case StationKitbashPieceRole::StructuralSpine: return 0.78f * compact;
        case StationKitbashPieceRole::CrossJunction:
        case StationKitbashPieceRole::TJunction:
        case StationKitbashPieceRole::LogisticsNode: return 0.88f * compact;
        case StationKitbashPieceRole::DockNeck: return 0.62f * compact;
        case StationKitbashPieceRole::DockCollar: return 0.78f * compact;
        case StationKitbashPieceRole::ShipyardGantry: return 1.15f * compact;
        case StationKitbashPieceRole::DefenseRing: return 1.0f * compact;
        case StationKitbashPieceRole::CommandSpire:
        case StationKitbashPieceRole::SensorCrown: return 0.60f * compact;
        default: return 0.72f * compact;
    }
}

struct RecipeBuilder {
    const std::vector<ShipyardModuleRecord>& catalog;
    const std::vector<StationKitbashPiece>& pieces;
    StationKitbashVisualRecipe& out;

    std::size_t AddRoot(StationKitbashPieceRole role, float z = 0.95f) {
        const auto* piece = StationKitbashCatalogSystem::Find(pieces, role);
        if (!piece || !piece->generatorEligible) return static_cast<std::size_t>(-1);
        const auto* record = FindModule(catalog, piece->sourceModuleId);
        if (!record) return static_cast<std::size_t>(-1);
        VisualModulePlacement p;
        p.moduleId = record->source.moduleId;
        p.z = z;
        const float s = UniformScaleFor(*record, TargetSpan(role, out.archetype == StationArchetype::AsteroidStation));
        p.scaleX = p.scaleY = p.scaleZ = s;
        p.material = MaterialFor(role);
        out.modules.push_back(p);
        out.logicalPieceIds.push_back(piece->id);
        out.logicalRoles.push_back(role);
        return out.modules.size() - 1;
    }

    std::size_t AddRadial(std::size_t parentIndex, StationKitbashPieceRole role,
                          float dx, float dy, float targetSpanScale = 1.0f) {
        if (parentIndex >= out.modules.size()) return static_cast<std::size_t>(-1);
        const auto* piece = StationKitbashCatalogSystem::Find(pieces, role);
        if (!piece || !piece->generatorEligible) return static_cast<std::size_t>(-1);
        const auto* childRecord = FindModule(catalog, piece->sourceModuleId);
        const auto* parentRecord = FindModule(catalog, out.modules[parentIndex].moduleId);
        if (!childRecord || !parentRecord) return static_cast<std::size_t>(-1);
        const float len = std::sqrt(dx*dx + dy*dy);
        if (len < 0.0001f) return static_cast<std::size_t>(-1);
        dx /= len; dy /= len;
        VisualModulePlacement p;
        p.moduleId = childRecord->source.moduleId;
        const float s = UniformScaleFor(*childRecord, TargetSpan(role, out.archetype == StationArchetype::AsteroidStation) * targetSpanScale);
        p.scaleX = p.scaleY = p.scaleZ = s;
        p.yawDegrees = std::atan2(dy, dx) * kRadToDeg - 90.0f;
        p.material = MaterialFor(role);
        p.mirrorX = (dx < -0.001f);
        const auto& parent = out.modules[parentIndex];
        const float parentR = ProjectedRadiusXY(*parentRecord, parent, dx, dy);
        const float childR = ProjectedRadiusXY(*childRecord, p, dx, dy);
        const float insertion = std::min(parentR, childR) * 0.06f;
        const float separation = std::max(0.02f, parentR + childR - insertion);
        p.x = parent.x + dx * separation;
        p.y = parent.y + dy * separation;
        p.z = parent.z;
        const std::size_t childIndex = out.modules.size();
        out.modules.push_back(p);
        out.logicalPieceIds.push_back(piece->id);
        out.logicalRoles.push_back(role);
        out.attachments.push_back({parentIndex, childIndex, "AUTO_OUT", "AUTO_IN", 0.0f, true});
        return childIndex;
    }

    std::size_t AddVertical(std::size_t parentIndex, StationKitbashPieceRole role, float dz = 1.0f) {
        if (parentIndex >= out.modules.size()) return static_cast<std::size_t>(-1);
        const auto* piece = StationKitbashCatalogSystem::Find(pieces, role);
        if (!piece || !piece->generatorEligible) return static_cast<std::size_t>(-1);
        const auto* childRecord = FindModule(catalog, piece->sourceModuleId);
        const auto* parentRecord = FindModule(catalog, out.modules[parentIndex].moduleId);
        if (!childRecord || !parentRecord) return static_cast<std::size_t>(-1);
        VisualModulePlacement p;
        p.moduleId = childRecord->source.moduleId;
        const float s = UniformScaleFor(*childRecord, TargetSpan(role, out.archetype == StationArchetype::AsteroidStation));
        p.scaleX = p.scaleY = p.scaleZ = s;
        p.material = MaterialFor(role);
        const auto& parent = out.modules[parentIndex];
        const float sign = dz >= 0.0f ? 1.0f : -1.0f;
        const float parentH = HalfHeight(*parentRecord, parent);
        const float childH = HalfHeight(*childRecord, p);
        const float insertion = std::min(parentH, childH) * 0.06f;
        p.x = parent.x;
        p.y = parent.y;
        p.z = parent.z + sign * std::max(0.02f, parentH + childH - insertion);
        const std::size_t childIndex = out.modules.size();
        out.modules.push_back(p);
        out.logicalPieceIds.push_back(piece->id);
        out.logicalRoles.push_back(role);
        out.attachments.push_back({parentIndex, childIndex, sign > 0 ? "AUTO_DORSAL" : "AUTO_VENTRAL", sign > 0 ? "AUTO_VENTRAL" : "AUTO_DORSAL", 0.0f, true});
        return childIndex;
    }
};

bool HasRole(const StationKitbashVisualRecipe& out, StationKitbashPieceRole role) {
    return std::find(out.logicalRoles.begin(), out.logicalRoles.end(), role) != out.logicalRoles.end();
}

} // namespace

const char* StationKitbashVisualSystem::Identity(StationArchetype archetype) {
    switch (archetype) {
        case StationArchetype::TradeHub: return "TRADE HUB";
        case StationArchetype::IndustrialRefinery: return "INDUSTRIAL REFINERY";
        case StationArchetype::MiningDepot: return "MINING DEPOT";
        case StationArchetype::Shipyard: return "SHIPYARD";
        case StationArchetype::Military: return "MILITARY CITADEL";
        case StationArchetype::Research: return "RESEARCH ARRAY";
        case StationArchetype::TetherTerminal: return "TETHER TERMINAL";
        case StationArchetype::FrontierOutpost: return "FRONTIER OUTPOST";
        case StationArchetype::AsteroidStation: return "ASTEROID STATION";
        case StationArchetype::CorporateHQ: return "CORPORATE HQ";
    }
    return "STATION";
}

StationKitbashVisualRecipe StationKitbashVisualSystem::Build(
    const std::vector<ShipyardModuleRecord>& catalog,
    StationArchetype archetype,
    std::uint64_t seed,
    bool asteroidEmbedded) {
    StationKitbashVisualRecipe out;
    out.identity = Identity(archetype);
    out.seed = seed;
    out.archetype = archetype;
    if (catalog.empty()) return out;

    const auto logicalCatalog = StationKitbashCatalogSystem::Build(catalog);
    const auto grammar = StationDesignGrammarSystem::ForArchetype(archetype, asteroidEmbedded);
    out.grammarId = grammar.id;
    RecipeBuilder b{catalog, logicalCatalog, out};
    const auto root = b.AddRoot(StationKitbashPieceRole::CoreHub, asteroidEmbedded ? 0.62f : 0.95f);
    if (root == static_cast<std::size_t>(-1)) return out;

    const std::uint32_t mix = Mix(seed ^ (static_cast<std::uint64_t>(archetype) << 40));
    std::vector<std::size_t> branchEnds;
    branchEnds.reserve(std::max(1, grammar.branchCount));
    for (int i = 0; i < std::max(1, grammar.branchCount); ++i) {
        float angle = -kPi * 0.5f + (2.0f*kPi*float(i)/float(std::max(1, grammar.branchCount)));
        if (!grammar.radial) {
            static const float angles[] = {-kPi*.5f, 0.0f, kPi*.5f, kPi, -kPi*.25f, kPi*.25f};
            angle = angles[i % 6];
        }
        const float dx = std::cos(angle), dy = std::sin(angle);
        std::size_t parent = root;
        const int segments = 1 + ((i + int(mix & 3u)) % std::max(1, std::min(3, grammar.spineSegments)));
        for (int s = 0; s < segments; ++s) {
            const auto next = b.AddRadial(parent,
                                          s == 0 && grammar.branchCount >= 5 ? StationKitbashPieceRole::CrossJunction : StationKitbashPieceRole::StructuralSpine,
                                          dx, dy, grammar.branchLengthScale);
            if (next == static_cast<std::size_t>(-1)) break;
            parent = next;
            out.usedStructuralConnectors = true;
        }
        branchEnds.push_back(parent);
    }

    // Dedicated docking chain on the first branch. This anchors docking
    // readability to actual generated geometry rather than a fixed Y offset.
    const float dockDx = 0.0f, dockDy = -1.0f;
    std::size_t dockParent = branchEnds.empty() ? root : branchEnds.front();
    const auto neck = b.AddRadial(dockParent, StationKitbashPieceRole::DockNeck, dockDx, dockDy, 1.0f);
    if (neck != static_cast<std::size_t>(-1)) dockParent = neck;
    const auto dock = b.AddRadial(dockParent, StationKitbashPieceRole::DockCollar, dockDx, dockDy, 1.0f);
    if (dock != static_cast<std::size_t>(-1)) {
        out.usedDedicatedDock = true;
        const auto* dockRecord = FindModule(catalog, out.modules[dock].moduleId);
        float radius = 0.8f;
        if (dockRecord) radius = ProjectedRadiusXY(*dockRecord, out.modules[dock], dockDx, dockDy);
        out.primaryDockLocal = {out.modules[dock].x + dockDx*(radius + 0.35f), out.modules[dock].y + dockDy*(radius + 0.35f), out.modules[dock].z};
        out.primaryDockDirection = {dockDx, dockDy, 0.0f};
    }

    // Command and sensor elements are connected vertically so they read as a
    // crown/spire instead of detached ornaments.
    std::size_t crownParent = root;
    if (archetype != StationArchetype::AsteroidStation) {
        const auto command = b.AddVertical(root, StationKitbashPieceRole::CommandSpire, 1.0f);
        if (command != static_cast<std::size_t>(-1)) { crownParent = command; out.usedCommandModule = true; }
    }
    if (archetype == StationArchetype::Research || archetype == StationArchetype::CorporateHQ ||
        archetype == StationArchetype::Military || archetype == StationArchetype::TetherTerminal) {
        const auto sensor = b.AddVertical(crownParent, StationKitbashPieceRole::SensorCrown, 1.0f);
        if (sensor != static_cast<std::size_t>(-1)) out.usedSensorModule = true;
    }

    // Distribute every archetype-required functional piece onto a connected
    // branch. This makes grammar requirements deterministic rather than RNG-only.
    // Reserve branch zero for the primary docking approach whenever another
    // branch exists. Functional massing should not be spawned through the dock
    // corridor or co-located with the dock collar.
    const std::size_t functionalBranchStart = branchEnds.size() > 1 ? 1u : 0u;
    const std::size_t functionalBranchCount = branchEnds.size() > functionalBranchStart ? branchEnds.size() - functionalBranchStart : branchEnds.size();
    std::size_t branchCursor = 0;
    for (const auto role : grammar.requiredPieces) {
        if (role == StationKitbashPieceRole::CoreHub || role == StationKitbashPieceRole::StructuralSpine ||
            role == StationKitbashPieceRole::DockCollar || HasRole(out, role)) continue;
        const std::size_t branchIndex = branchEnds.empty() ? 0u : functionalBranchStart + (branchCursor++ % std::max<std::size_t>(1, functionalBranchCount));
        const std::size_t parent = branchEnds.empty() ? root : branchEnds[branchIndex];
        const auto& parentPlacement = out.modules[parent];
        const float radialLength = std::sqrt(parentPlacement.x*parentPlacement.x + parentPlacement.y*parentPlacement.y);
        const float fallbackAngle = 2.0f*kPi*float(branchCursor)/float(std::max<std::size_t>(1, branchEnds.size()));
        const float dx = radialLength > 0.05f ? parentPlacement.x/radialLength : std::cos(fallbackAngle);
        const float dy = radialLength > 0.05f ? parentPlacement.y/radialLength : std::sin(fallbackAngle);
        const auto child = b.AddRadial(parent, role, dx, dy, 1.0f);
        if (child != static_cast<std::size_t>(-1)) {
            if(!branchEnds.empty()) branchEnds[branchIndex] = child;
            const auto* piece = StationKitbashCatalogSystem::Find(logicalCatalog, role);
            if (piece && piece->function == StationModuleRole::Defense) out.usedHardpointModule = true;
            if (piece && (piece->function == StationModuleRole::Storage || piece->function == StationModuleRole::Refinery ||
                          piece->function == StationModuleRole::Manufacturing || piece->function == StationModuleRole::Market ||
                          piece->function == StationModuleRole::Shipyard || piece->function == StationModuleRole::Repair ||
                          piece->function == StationModuleRole::Habitation || piece->function == StationModuleRole::Logistics)) out.usedServiceModule = true;
        }
    }

    // Add a small amount of deterministic optional massing, always attached.
    for (int i = 0; i < std::min<int>(3, static_cast<int>(grammar.optionalPieces.size())); ++i) {
        if (branchEnds.empty()) break;
        const auto role = grammar.optionalPieces[(i + int(mix % grammar.optionalPieces.size())) % grammar.optionalPieces.size()];
        const std::size_t idx = functionalBranchStart + static_cast<std::size_t>((i + (mix >> 5)) % std::max<std::size_t>(1, functionalBranchCount));
        const auto& parentPlacement = out.modules[branchEnds[idx]];
        const float radialLength = std::sqrt(parentPlacement.x*parentPlacement.x + parentPlacement.y*parentPlacement.y);
        const float fallbackAngle = 2.0f*kPi*float(idx)/float(branchEnds.size());
        const float dx = radialLength > 0.05f ? parentPlacement.x/radialLength : std::cos(fallbackAngle);
        const float dy = radialLength > 0.05f ? parentPlacement.y/radialLength : std::sin(fallbackAngle);
        const auto child = b.AddRadial(branchEnds[idx], role, dx, dy, .92f);
        if (child != static_cast<std::size_t>(-1)) branchEnds[idx] = child;
    }

    out.resolved = !out.modules.empty();
    out.connectedGraph = out.resolved && out.attachments.size() + 1 == out.modules.size();
    out.maxMeasuredGap = 0.0f;
    for (const auto& a : out.attachments) out.maxMeasuredGap = std::max(out.maxMeasuredGap, std::abs(a.measuredGap));
    return out;
}

} // namespace subspace
