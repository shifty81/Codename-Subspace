#pragma once
#include "runtime/StableIdentitySystem.h"
#include "world/SpatialFrameSystem.h"
#include <cstdint>
#include <string>

namespace subspace {

constexpr std::uint32_t SubspacePersistenceSchemaVersion = 2;

struct PersistentRecordHeader {
    std::uint32_t schemaVersion = SubspacePersistenceSchemaVersion;
    PersistentEntityId id{};
    PersistentEntityKind kind = PersistentEntityKind::Unknown;
    std::uint64_t revision = 0;
    std::string generationRecipeVersion;
};

struct PersistentShipStateContract {
    PersistentRecordHeader header{};
    SpatialFrameId frameId = InvalidSpatialFrameId;
    std::string assemblyDefinitionId;
    std::string appearanceDefinitionId;
    std::string interiorDefinitionId;
    std::string damageStateId;
};

struct PersistentCellStateContract {
    PersistentRecordHeader header{};
    PersistentEntityId parentWorldId{};
    std::string cellDefinitionId;
    std::string deltaJournalId;
};

struct PersistentSettlementStateContract {
    PersistentRecordHeader header{};
    PersistentEntityId parentWorldId{};
    std::string settlementDefinitionId;
    std::string simulationStateId;
};

struct PersistentSurveyRecordContract {
    PersistentRecordHeader header{};
    PersistentEntityId subjectId{};
    std::string discovererId;
    double quality = 0.0;
    double confidence = 0.0;
    std::string provenance;
};

} // namespace subspace
