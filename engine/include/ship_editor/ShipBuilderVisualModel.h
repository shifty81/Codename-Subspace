#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ShipBuilderTool {
    Select,
    PlaceBlock,
    RemoveBlock,
    PaintBlock,
    MirrorPlane,
    Validate
};

struct ShipBuilderPaletteEntry {
    std::string id;
    std::string label;
    std::string category;
};

struct ShipBuilderVisualState {
    ShipBuilderTool activeTool = ShipBuilderTool::Select;
    int gridSize = 1;
    bool mirrorX = false;
    bool mirrorY = false;
    bool mirrorZ = false;
    std::vector<ShipBuilderPaletteEntry> palette;
};

ShipBuilderVisualState BuildDefaultShipBuilderVisualState();
std::string ShipBuilderToolName(ShipBuilderTool tool);
std::string ShipBuilderVisualStateSummary(const ShipBuilderVisualState& state);

} // namespace subspace
