#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/ShipyardStableIdSystem.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

struct ShipyardDocument {
    static constexpr std::uint32_t SchemaVersion = 1;

    std::uint32_t schemaVersion = SchemaVersion;
    ShipyardObjectId documentId{};
    std::string persistentShipId;
    ProceduralShipVisualRecipe recipe{};
    ShipAppearanceState appearance{};

    // Stable identity is intentionally sidecar metadata rather than vector
    // position. The recipe remains the runtime/save authority while the editor
    // can survive insertion, deletion, undo/redo and future serialization.
    std::vector<ShipyardObjectId> moduleIds;
    std::vector<ShipyardObjectId> attachmentIds;

    std::uint64_t revision = 1;
    std::uint64_t savedRevision = 1;
    std::uint64_t nextLocalOrdinal = 1;
};

struct ShipyardDocumentSnapshot {
    ShipyardDocument document{};
};

struct ShipyardDocumentValidation {
    bool valid = false;
    std::vector<std::string> errors;
};

class ShipyardDocumentSystem {
public:
    static ShipyardDocument Create(const ProceduralShipVisualRecipe& recipe,
                                   const ShipAppearanceState& appearance = {},
                                   std::string persistentShipId = {});

    static ShipyardDocumentSnapshot Capture(const ShipyardDocument& document);
    static void Restore(ShipyardDocument& target, const ShipyardDocumentSnapshot& snapshot);

    static ShipyardObjectId ModuleIdAt(const ShipyardDocument& document, std::size_t index);
    static ShipyardObjectId AttachmentIdAt(const ShipyardDocument& document, std::size_t index);
    static std::optional<std::size_t> FindModuleIndex(const ShipyardDocument& document,
                                                      const ShipyardObjectId& moduleId);
    static std::optional<std::size_t> FindAttachmentIndex(const ShipyardDocument& document,
                                                          const ShipyardObjectId& attachmentId);

    static ShipyardObjectId AppendModule(ShipyardDocument& document,
                                         const VisualModulePlacement& placement);
    static ShipyardObjectId AppendAttachment(ShipyardDocument& document,
                                             const VisualAssemblyAttachment& attachment);
    static bool EraseModule(ShipyardDocument& document, const ShipyardObjectId& moduleId);
    static bool EraseAttachment(ShipyardDocument& document, const ShipyardObjectId& attachmentId);

    static void MarkModified(ShipyardDocument& document);
    static void MarkSaved(ShipyardDocument& document);
    static bool IsDirty(const ShipyardDocument& document);

    static ShipyardDocumentValidation ValidateIdentity(const ShipyardDocument& document);
};

} // namespace subspace
