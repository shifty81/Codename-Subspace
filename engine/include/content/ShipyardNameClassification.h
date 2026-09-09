#pragma once

#include <algorithm>
#include <cctype>
#include <initializer_list>
#include <string>
#include "content/ShipyardSbsClassification.generated.h"

namespace subspace {

/// Filename-first classification for Greyoxide Shipyard v0.7 and compatible
/// Subspace-authored module names.
///
/// IMPORTANT: certified module IDs contain historical class prefixes such as
/// `shipyard_a_propulsion_...`. Those prefixes are provenance, not semantic
/// truth. Classification must operate on the original leaf name so a piece
/// called engineStrutLadder can never become a complete engine simply because
/// an earlier intake pass put it under a `propulsion` prefix.
struct ShipyardNameClassification {
    std::string canonicalName;
    std::string moduleClass;
    std::string semantic;
    int confidence = 0;
    std::string rule;
};

class ShipyardNameClassifier {
public:
    static std::string Lower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    static bool ContainsAny(const std::string& value, std::initializer_list<const char*> tokens) {
        for (const char* token : tokens) if (value.find(token) != std::string::npos) return true;
        return false;
    }

    /// Strip paths/extensions and both historical certification prefixes:
    /// shipyard_a_<oldclass>_<serial>_shipyard_<oldclass>_<serial>_<leaf>
    /// shipyard_a_<oldclass>_<serial>_<leaf>
    /// shipyard_<oldclass>_<serial>_<leaf>
    static std::string CanonicalLeafName(std::string value) {
        const std::size_t slash = value.find_last_of("/\\");
        if (slash != std::string::npos) value = value.substr(slash + 1);
        const std::size_t dot = value.find_last_of('.');
        if (dot != std::string::npos) value = value.substr(0, dot);
        value = Lower(value);

        const std::size_t nested = value.find("_shipyard_");
        if (nested != std::string::npos) value = value.substr(nested + 1); // keep shipyard_ for common parser below

        auto stripShipyardPrefix = [](const std::string& v, const std::string& prefix) -> std::string {
            if (v.rfind(prefix, 0) != 0) return v;
            std::string rest = v.substr(prefix.size());
            const std::size_t classEnd = rest.find('_');
            if (classEnd == std::string::npos) return rest;
            const std::size_t serialEnd = rest.find('_', classEnd + 1);
            if (serialEnd == std::string::npos) return rest.substr(classEnd + 1);
            return rest.substr(serialEnd + 1);
        };

        if (value.rfind("shipyard_a_", 0) == 0) value = stripShipyardPrefix(value, "shipyard_a_");
        if (value.rfind("shipyard_", 0) == 0) value = stripShipyardPrefix(value, "shipyard_");
        return value;
    }

    static ShipyardNameClassification Classify(const std::string& sourceNameOrId) {
        const std::string v = CanonicalLeafName(sourceNameOrId);
        if (const auto* exact = FindShipyardSbsClassification(v))
            return ShipyardNameClassification{v, exact->moduleClass, exact->semantic, exact->confidence, exact->rule};
        auto make = [&](const char* cls, const char* semantic, int confidence, const char* rule) {
            return ShipyardNameClassification{v, cls, semantic, confidence, rule};
        };

        // Explicit hull naming wins over generic connector/join tokens. This
        // corrects hullJoined, which was historically mislabeled as Adapter.
        if (v.rfind("hull", 0) == 0 || ContainsAny(v, {"_hull", "fuselage"})) {
            if (ContainsAny(v, {"bow", "nose", "front"})) return make("hull", "HULL_BOW", 100, "hull-name/bow");
            if (ContainsAny(v, {"aft", "rear", "stern"})) return make("hull", "HULL_AFT", 100, "hull-name/aft");
            return make("hull", "HULL_MID", 100, "hull-name");
        }

        // The dedicated hardpoint layer contains both bases/platforms and
        // actual gun assemblies. Names with gun/weapon language are equipment;
        // the remaining hardpoint objects are mounting structures.
        if (v.rfind("hardpoint", 0) == 0) {
            if (ContainsAny(v, {"gun", "weapon", "cannon", "missile"}))
                return make("hardpoint", "WEAPON_MOUNT", 98, "hardpoint-gun-name");
            return make("hardpoint", "TURRET_HARDPOINT", 96, "hardpoint-base-name");
        }

        if (ContainsAny(v, {"bridge", "cockpit", "canopy", "command"})) {
            return make("command", ContainsAny(v, {"cockpit", "canopy"}) ? "COMMAND_COCKPIT" : "COMMAND_BRIDGE",
                        100, "command-name");
        }

        // Greyoxide explicitly describes layer four as an engine *builder*.
        // Therefore its engine-prefixed names need a second semantic pass.
        if (v.rfind("engine", 0) == 0 || v.rfind("enigne", 0) == 0) {
            if (ContainsAny(v, {"trussworkwing", "wing"}))
                return make("wing", "WING", 100, "engine-builder-wing");
            if (ContainsAny(v, {"strut"}))
                return make("component", "STRUCTURAL_FRAME", 100, "engine-builder-strut");
            if (ContainsAny(v, {"body", "bracket", "housing", "shroud", "nacelle", "enginepod", "engine_pod"}))
                return make("propulsion", "ENGINE_HOUSING", 98, "engine-builder-body/housing");
            if (ContainsAny(v, {"trumpet", "vanes", "flap", "nozzle", "bell", "exhaust"}))
                return make("propulsion", "ENGINE_NOZZLE", 94, "engine-builder-nozzle/control");
            if (ContainsAny(v, {"rcs", "retro", "maneuver", "thruster"}))
                return make("propulsion", "RCS_THRUSTER", 100, "engine-builder-thruster");
            return make("propulsion", "MAIN_ENGINE", 92, "engine-builder-drive");
        }

        if (ContainsAny(v, {"bipolarengine"}))
            return make("propulsion", "MAIN_ENGINE", 100, "explicit-engine-name");

        // Generic Subspace-authored propulsion names remain supported.
        if (ContainsAny(v, {"rcs", "retro", "maneuver", "thruster"}))
            return make("propulsion", "RCS_THRUSTER", 100, "thruster-name");
        if (ContainsAny(v, {"nozzle", "bell", "exhaust"}))
            return make("propulsion", "ENGINE_NOZZLE", 100, "nozzle-name");
        if (ContainsAny(v, {"housing", "shroud", "nacelle", "enginepod", "engine_pod"}))
            return make("propulsion", "ENGINE_HOUSING", 98, "engine-housing-name");
        if (ContainsAny(v, {"drive", "propulsion"}))
            return make("propulsion", "MAIN_ENGINE", 90, "generic-propulsion-name");

        if (ContainsAny(v, {"vertmisstube", "misstube", "missiletube", "missile_tube"}))
            return make("hardpoint", "WEAPON_MOUNT", 98, "missile-tube-name");

        if (ContainsAny(v, {"wing", "fin"}))
            return make("wing", "WING", 97, "wing/fin-name");

        // Instrument masts, telescopes and dishes are sensor hardware. Generic
        // mast is intentionally included after command/engine/wing rules.
        if (ContainsAny(v, {"sensor", "antenna", "radar", "instrument", "telescope", "dish", "mast"}))
            return make("detail", "SENSOR", 92, "sensor/instrument-name");

        if (ContainsAny(v, {"greeble", "detail", "vent", "panel", "leafpanel"}))
            return make("detail", "SURFACE_DETAIL", 96, "surface-detail-name");

        // These are useful structural kit pieces but should never enter the
        // generator's main-hull or propulsion pools automatically.
        if (ContainsAny(v, {"outrigger", "railrunner"}))
            return make("component", "STRUCTURAL_FRAME", 84, "structural-component-name");

        if (ContainsAny(v, {"adapter", "connector", "neck", "fairing"}))
            return make("adapter", "ADAPTER", 92, "adapter-name");

        if (ContainsAny(v, {"turret", "weapon", "gun", "mount"}))
            return make("hardpoint", ContainsAny(v, {"gun", "weapon"}) ? "WEAPON_MOUNT" : "TURRET_HARDPOINT",
                        88, "generic-hardpoint-name");

        return make("component", "COMPONENT", 72, "generic-component");
    }
};

} // namespace subspace
