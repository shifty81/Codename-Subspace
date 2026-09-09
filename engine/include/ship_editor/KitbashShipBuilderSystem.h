#pragma once

#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

struct KitbashShipBuilderPaletteItem {
    std::string moduleId;
    std::string label;
    ShipyardPartCategory category = ShipyardPartCategory::Decoration;
    ShipyardPartRole role = ShipyardPartRole::Decoration;
    ShipyardModuleSize size = ShipyardModuleSize::M;
    bool primaryHull = false;
    bool generatorEligible = false;
    bool surfaceOnly = false;
    bool functional = false;
    bool mirrorPreferred = false;
};

struct KitbashShipBuilderPreview {
    bool valid = false;
    std::size_t parentModuleIndex = 0;
    std::string parentSocket;
    std::string childSocket;
    VisualModulePlacement placement{};
    ShipVisualAttachment attachment{};
    std::string reason;
};


struct KitbashShipBuilderSnapshot {
    ProceduralShipVisualRecipe blueprint{};
    std::vector<ShipEquipmentSlot> equipmentSlots;
    ShipAppearanceState appearance{};
    std::size_t selectedModuleIndex = 0;
};

struct KitbashShipBuilderState {
    std::vector<KitbashShipBuilderPaletteItem> palette;
    ProceduralShipVisualRecipe blueprint{};
    ShipyardPartCategory category = ShipyardPartCategory::Hull;
    int page = 0;
    int pageSize = 9;
    bool dragging = false;
    std::string draggingModuleId;
    KitbashShipBuilderPreview preview{};
    std::string status = "SELECT A PART";
    bool dirty = false;
    std::size_t selectedModuleIndex = 0;
    std::vector<ShipEquipmentSlot> equipmentSlots;
    ShipAppearanceState appearance{};
    std::vector<KitbashShipBuilderSnapshot> undoStack;
    std::vector<KitbashShipBuilderSnapshot> redoStack;
    std::string blueprintName = "Untitled Ship";
};

/// Pass426-430 in-game kitbash Shipyard authority.
///
/// The manual builder and procedural generator consume the same certified
/// ShipyardModuleRecord catalog. Drag/drop never invents world offsets: every
/// placement is calculated by typed mating sockets and validated before it is
/// committed to the rooted assembly graph.
class KitbashShipBuilderSystem {
public:
    static std::vector<KitbashShipBuilderPaletteItem> BuildPalette(
        const std::vector<ShipyardModuleRecord>& catalog);

    static KitbashShipBuilderState CreateStarter(
        const std::vector<ShipyardModuleRecord>& catalog,
        const std::string& role = "INDUSTRIAL");

    static const ShipyardModuleRecord* FindRecord(
        const std::vector<ShipyardModuleRecord>& catalog,
        const std::string& moduleId);

    static bool BeginDrag(KitbashShipBuilderState& state,
                          const std::vector<ShipyardModuleRecord>& catalog,
                          const std::string& moduleId,
                          std::string* error = nullptr);

    static KitbashShipBuilderPreview PreviewDrop(
        const KitbashShipBuilderState& state,
        const std::vector<ShipyardModuleRecord>& catalog,
        std::size_t parentModuleIndex,
        const std::string& parentSocketName);

    static bool CommitDrop(KitbashShipBuilderState& state,
                           const std::vector<ShipyardModuleRecord>& catalog,
                           const KitbashShipBuilderPreview& preview,
                           std::string* error = nullptr);

    static void CancelDrag(KitbashShipBuilderState& state);

    static void SelectModule(KitbashShipBuilderState& state, std::size_t moduleIndex);
    static bool Undo(KitbashShipBuilderState& state);
    static bool Redo(KitbashShipBuilderState& state);
    static bool CanUndo(const KitbashShipBuilderState& state) { return !state.undoStack.empty(); }
    static bool CanRedo(const KitbashShipBuilderState& state) { return !state.redoStack.empty(); }

    static bool ValidateBlueprint(const KitbashShipBuilderState& state,
                                  const std::vector<ShipyardModuleRecord>& catalog,
                                  std::string* error = nullptr);

    static std::vector<std::size_t> FilterPalette(const KitbashShipBuilderState& state);
};

} // namespace subspace
