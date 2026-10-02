#pragma once

#include "interior/ModularInteriorKitSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct InteriorKitRecord {
    InteriorModuleKit kit;
    int priority = 0;
    std::vector<std::string> contexts;
    std::vector<std::string> capabilities;
    std::string provenanceId;
};

struct InteriorKitSelectionRequest {
    std::string preferredFamily = "industrial_modular";
    std::string context = "ship_interior";
    std::vector<InteriorModuleKind> requiredKinds;
    bool requireHydrated = true;
};

struct InteriorKitSelectionResult {
    const InteriorModuleKit* kit = nullptr;
    std::string status;
    std::vector<std::string> warnings;
    bool valid = false;
};

/// One registry for ship, station, hangar, derelict and planetary interiors.
/// Gameplay asks for capabilities/context; source-pack names never leak into
/// ship module gameplay code.
class InteriorKitRegistrySystem {
public:
    bool Register(InteriorKitRecord record);
    bool Remove(const std::string& kitId);
    const InteriorKitRecord* Find(const std::string& kitId) const;
    InteriorKitSelectionResult Resolve(const InteriorKitSelectionRequest& request) const;
    const std::vector<InteriorKitRecord>& Records() const { return _records; }

private:
    static bool Contains(const std::vector<std::string>& values,const std::string& value);
    static bool SatisfiesKinds(const InteriorModuleKit& kit,const std::vector<InteriorModuleKind>& kinds);
    std::vector<InteriorKitRecord> _records;
};

} // namespace subspace
