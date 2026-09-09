#include "ship_editor/ShipyardAuthoringBridge.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace subspace {
namespace {

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

void ApplyExistingSemantic(const ShipyardModuleRecord& record, ShipyardAuthoringDefinition& out) {
    const std::string semantic = Lower(ShipyardModuleSystem::SemanticName(record.semantic));
    if (semantic.find("hullbow") != std::string::npos || semantic.find("hull_bow") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Hull; out.subtype = ShipyardAuthoringSubtype::HullBow;
    } else if (semantic.find("hullmid") != std::string::npos || semantic.find("hull_mid") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Hull; out.subtype = ShipyardAuthoringSubtype::HullMid;
    } else if (semantic.find("hullaft") != std::string::npos || semantic.find("hull_aft") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Hull; out.subtype = ShipyardAuthoringSubtype::HullStern;
    } else if (semantic.find("structural") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Hull; out.subtype = ShipyardAuthoringSubtype::StructuralSupport;
    } else if (semantic.find("cockpit") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Command; out.subtype = ShipyardAuthoringSubtype::Cockpit; out.functionalRole = "Command";
    } else if (semantic.find("bridge") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Command; out.subtype = ShipyardAuthoringSubtype::Bridge; out.functionalRole = "Command";
    } else if (semantic.find("adapter") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Adapter; out.subtype = ShipyardAuthoringSubtype::HullAdapter;
    } else if (semantic.find("enginehousing") != std::string::npos || semantic.find("engine_housing") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Propulsion; out.subtype = ShipyardAuthoringSubtype::EngineHousing;
    } else if (semantic.find("mainengine") != std::string::npos || semantic.find("main_engine") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Propulsion; out.subtype = ShipyardAuthoringSubtype::MainEngine; out.functionalRole = "MainEngine";
    } else if (semantic.find("nozzle") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Propulsion; out.subtype = ShipyardAuthoringSubtype::EngineNozzle;
    } else if (semantic.find("rcs") != std::string::npos || semantic.find("thruster") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Propulsion; out.subtype = ShipyardAuthoringSubtype::ManeuverThruster; out.functionalRole = "ManeuverThrust";
    } else if (semantic.find("turret") != std::string::npos || semantic.find("weaponmount") != std::string::npos || semantic.find("weapon_mount") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Weapon; out.subtype = ShipyardAuthoringSubtype::WeaponMount; out.functionalRole = "WeaponMount";
    } else if (semantic.find("wing") != std::string::npos || semantic.find("fin") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Surface; out.subtype = ShipyardAuthoringSubtype::LateralWing; out.functionalRole = "StructuralSurface"; out.frame.lateralSurface = true;
    } else if (semantic.find("sensor") != std::string::npos || semantic.find("radar") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Electronics; out.subtype = ShipyardAuthoringSubtype::SensorArray; out.functionalRole = "Sensor";
    } else if (semantic.find("detail") != std::string::npos || semantic.find("decoration") != std::string::npos) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Detail; out.subtype = ShipyardAuthoringSubtype::Panel; out.functionalRole = "ExteriorDetail";
    }
}

std::string SlotPrefix(const ShipyardAuthoringDefinition& def) {
    return Lower(ShipyardAuthoringAuthority::PrimaryClassName(def.primaryClass));
}

std::string SideOf(float x) {
    if (x < -0.05f) return "port";
    if (x > 0.05f) return "starboard";
    return "center";
}

float DistanceSquared(const VisualModulePlacement& a, const VisualModulePlacement& b) {
    const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return dx * dx + dy * dy + dz * dz;
}

} // namespace

void ShipyardAuthoringBridge::RegisterCurrentCatalog(ShipyardAuthoringAuthority& authority,
                                                      const std::vector<ShipyardModuleRecord>& catalog) {
    authority.RegisterBuiltInCertifiedOverrides();
    for (const auto& record : catalog) {
        if (record.source.moduleId.empty()) continue;
        // Never downgrade an explicit certified override such as wing_149.
        if (const auto* existing = authority.FindDefinition(record.source.moduleId);
            existing && existing->certification == ShipyardCertificationState::Certified) continue;

        auto def = authority.InferDefinition(record.source.moduleId);
        ApplyExistingSemantic(record, def);
        def.sizeClass = ShipyardModuleSystem::SizeName(record.size);
        def.certification = ShipyardCertificationState::Reviewed;
        def.compatibleSocketTypes.clear();
        for (const auto& socket : record.sockets) {
            if (std::find(def.compatibleSocketTypes.begin(), def.compatibleSocketTypes.end(), socket.type) == def.compatibleSocketTypes.end())
                def.compatibleSocketTypes.push_back(socket.type);
        }
        if (!record.preferredRoles.empty()) def.functionalRole = record.preferredRoles.front();
        def.family = "greyoxide." + Lower(ShipyardAuthoringAuthority::PrimaryClassName(def.primaryClass)) + "." +
                     Lower(ShipyardAuthoringAuthority::SubtypeName(def.subtype));
        def.rerollGroup = def.family + "." + Lower(def.sizeClass);
        authority.RegisterDefinition(std::move(def));
    }
}

ShipyardResolvedBlueprint ShipyardAuthoringBridge::BuildPreservedBlueprint(ShipyardAuthoringAuthority& authority,
                                                                           const ProceduralShipVisualRecipe& recipe,
                                                                           const std::vector<ShipyardModuleRecord>& catalog,
                                                                           std::uint32_t generatorVersion) {
    RegisterCurrentCatalog(authority, catalog);

    ShipyardResolvedBlueprint out;
    out.shipSeed = recipe.seed;
    out.generatorVersion = generatorVersion;
    out.blueprintId = recipe.recipeId.empty() ? ("generated." + std::to_string(recipe.seed)) : recipe.recipeId;

    std::unordered_map<std::string, unsigned> counters;
    std::vector<size_t> hullIndices;
    out.slots.reserve(recipe.modules.size());

    for (size_t i = 0; i < recipe.modules.size(); ++i) {
        const auto& placement = recipe.modules[i];
        auto def = authority.FindDefinition(placement.moduleId);
        if (!def) {
            authority.RegisterDefinition(authority.InferDefinition(placement.moduleId));
            def = authority.FindDefinition(placement.moduleId);
        }

        const std::string prefix = def ? SlotPrefix(*def) : "unknown";
        const std::string side = SideOf(placement.x);
        unsigned ordinal = ++counters[prefix + "." + side];
        std::ostringstream id;
        id << prefix << "." << side << "." << std::setfill('0') << std::setw(2) << ordinal;

        ShipyardSlot slot;
        slot.slotId = id.str();
        slot.sourcePlacementIndex = static_cast<uint32_t>(i);
        slot.role = def ? def->functionalRole : "Unknown";
        slot.moduleDefinitionId = placement.moduleId;
        slot.transform = {placement.x, placement.y, placement.z, placement.yawDegrees, placement.pitchDegrees, placement.rollDegrees};
        out.slots.push_back(std::move(slot));
        if (def && def->primaryClass == ShipyardAuthoringPrimaryClass::Hull) hullIndices.push_back(i);
    }

    // Metadata-only parent projection: choose the nearest preserved hull placement.
    // This does not move or rebuild any module.
    for (size_t i = 0; i < out.slots.size(); ++i) {
        auto* def = authority.FindDefinition(out.slots[i].moduleDefinitionId);
        if (!def || def->primaryClass == ShipyardAuthoringPrimaryClass::Hull || hullIndices.empty()) continue;
        size_t best = hullIndices.front();
        float bestDistance = DistanceSquared(recipe.modules[i], recipe.modules[best]);
        for (size_t candidate : hullIndices) {
            const float distance = DistanceSquared(recipe.modules[i], recipe.modules[candidate]);
            if (distance < bestDistance) { bestDistance = distance; best = candidate; }
        }
        out.slots[i].parentSlotId = out.slots[best].slotId;
        out.slots[i].parentSocketId = "legacy_preserved";
    }

    // Recover obvious existing mirror pairs without changing transforms.
    for (size_t i = 0; i < out.slots.size(); ++i) {
        if (!out.slots[i].mirrorPartnerSlotId.empty()) continue;
        const auto* aDef = authority.FindDefinition(out.slots[i].moduleDefinitionId);
        if (!aDef || (aDef->primaryClass != ShipyardAuthoringPrimaryClass::Surface &&
                      aDef->primaryClass != ShipyardAuthoringPrimaryClass::Propulsion &&
                      aDef->primaryClass != ShipyardAuthoringPrimaryClass::Weapon &&
                      aDef->primaryClass != ShipyardAuthoringPrimaryClass::Utility)) continue;
        const auto& a = recipe.modules[i];
        for (size_t j = i + 1; j < out.slots.size(); ++j) {
            if (!out.slots[j].mirrorPartnerSlotId.empty()) continue;
            const auto& b = recipe.modules[j];
            if (a.moduleId != b.moduleId) continue;
            const bool mirrorX = std::fabs(a.x + b.x) < 0.15f;
            const bool sameY = std::fabs(a.y - b.y) < 0.15f;
            const bool sameZ = std::fabs(a.z - b.z) < 0.15f;
            if (mirrorX && sameY && sameZ && std::fabs(a.x) > 0.05f) {
                out.slots[i].mirrorPartnerSlotId = out.slots[j].slotId;
                out.slots[j].mirrorPartnerSlotId = out.slots[i].slotId;
                out.slots[i].pairMode = ShipyardPairMode::MirroredIdentical;
                out.slots[j].pairMode = ShipyardPairMode::MirroredIdentical;
                break;
            }
        }
    }

    return out;
}

bool ShipyardAuthoringBridge::ApplyModuleChoicesPreservingPlacement(const ShipyardResolvedBlueprint& blueprint,
                                                                    ProceduralShipVisualRecipe& recipe) {
    for (const auto& slot : blueprint.slots) {
        if (slot.sourcePlacementIndex >= recipe.modules.size()) return false;
    }
    for (const auto& slot : blueprint.slots) {
        recipe.modules[slot.sourcePlacementIndex].moduleId = slot.moduleDefinitionId;
    }
    return true;
}

} // namespace subspace
