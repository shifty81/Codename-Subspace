#pragma once

#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardDocumentSystem.h"
#include "ship_editor/ShipyardHistorySystem.h"
#include "ship_editor/ShipyardSessionSystem.h"

#include <cstdint>
#include <string>

namespace subspace {

struct ShipyardBuilderMigrationState {
    ShipyardDocument document{};
    ShipyardSession session{};
    ShipyardHistorySystem history{128};
    std::uint64_t authoredFingerprint = 0;
    bool bound = false;
};

/// Compatibility bridge while ShipyardBuilderSystem is being decomposed.
/// Authored state is projected into ShipyardDocument while transient editor
/// state is projected into ShipyardSession.
class ShipyardBuilderMigrationSystem {
public:
    static ShipyardBuilderMigrationState Bind(const ShipyardBuilderRuntimeModel& legacy,
                                               std::string persistentShipId = {});
    static void PullFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                               ShipyardBuilderMigrationState& state);
    static void SyncSessionFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                                      ShipyardBuilderMigrationState& state);
    static void SyncSelectionFromLegacy(const ShipyardBuilderRuntimeModel& legacy,
                                        ShipyardBuilderMigrationState& state);
    static bool Validate(const ShipyardBuilderRuntimeModel& legacy,
                         const ShipyardBuilderMigrationState& state,
                         std::string* error = nullptr);
};

} // namespace subspace
