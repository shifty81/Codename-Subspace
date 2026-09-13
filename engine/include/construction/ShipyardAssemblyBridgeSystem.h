#pragma once

#include "construction/AssemblyConstructionSystem.h"
#include "content/ShipyardModuleSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct ShipyardAssemblyImportOptions {
    double densityKgPerCubicMeter = 42.0;
    bool generateFallbackSockets = true;
    bool preserveMirrorsAsSignedScale = true;
};

struct ShipyardAssemblyImportReport {
    AssemblyDefinition assembly{};
    bool valid = false;
    std::size_t importedElements = 0;
    std::size_t importedAttachments = 0;
    std::size_t generatedSockets = 0;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

/// Converts the certified Shipyard catalog + procedural/manual visual recipe
/// into the canonical editable AssemblyDefinition introduced in Pass987-997.
/// The adapter is deliberately one-way for now: legacy recipe data remains
/// intact while the canonical assembly becomes the editing/runtime authority.
class ShipyardAssemblyBridgeSystem {
public:
    static ShipyardAssemblyImportReport Import(const std::vector<ShipyardModuleRecord>& catalog,
                                               const ProceduralShipVisualRecipe& recipe,
                                               PersistentEntityId persistentShipId,
                                               ShipyardAssemblyImportOptions options = {});

    static std::vector<std::string> CapabilityTags(const ShipyardModuleRecord& record);
    static AttachmentFace FaceFromDirection(double x, double y, double z);
};

} // namespace subspace
