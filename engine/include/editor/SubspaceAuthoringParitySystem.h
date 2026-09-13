#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class SubspaceAuthoringDomain {
    Ship = 0,
    Character,
    World,
    Interior,
    Material,
    Animation,
    Pcg,
    Vfx,
    Audio,
    Logic
};

struct SubspaceAuthoringContract {
    SubspaceAuthoringDomain domain = SubspaceAuthoringDomain::Ship;
    std::string contractId;
    std::string canonicalType;
    std::string editorWorkspaceId;
    bool editorCanAuthorDefinitions = true;
    bool gameClientConsumesCanonicalType = true;
    bool gameClientPlayerEditing = false;
    std::string playerFacingSurface;
    std::string restrictionSummary;
};

struct SubspaceParityReport {
    bool valid = false;
    std::vector<std::string> errors;
};

/// Defines editor/client parity. Shipyard Editor is the authoring superset; the
/// game client consumes the same canonical contracts and may expose restricted
/// player-facing editors without inventing duplicate data formats.
class SubspaceAuthoringParitySystem {
public:
    static std::vector<SubspaceAuthoringContract> BuildDefaultContracts();
    static const SubspaceAuthoringContract* Find(const std::vector<SubspaceAuthoringContract>& contracts,
                                                 SubspaceAuthoringDomain domain);
    static SubspaceParityReport Validate(const std::vector<SubspaceAuthoringContract>& contracts);
};

} // namespace subspace
