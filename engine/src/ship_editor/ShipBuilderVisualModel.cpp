#include "ship_editor/ShipBuilderVisualModel.h"

#include <sstream>

namespace subspace {

ShipBuilderVisualState BuildDefaultShipBuilderVisualState()
{
    ShipBuilderVisualState state;
    state.activeTool = ShipBuilderTool::PlaceBlock;
    state.gridSize = 1;
    state.mirrorX = true;
    state.palette = {
        {"hull-light", "Light Hull", "structure"},
        {"hull-heavy", "Heavy Hull", "structure"},
        {"cockpit", "Cockpit", "command"},
        {"engine", "Engine", "propulsion"},
        {"weapon-mount", "Weapon Mount", "combat"},
        {"cargo", "Cargo Bay", "utility"},
        {"shield", "Shield Generator", "defense"},
        {"scanner", "Scanner", "utility"}
    };
    return state;
}

std::string ShipBuilderToolName(ShipBuilderTool tool)
{
    switch (tool) {
        case ShipBuilderTool::Select: return "Select";
        case ShipBuilderTool::PlaceBlock: return "PlaceBlock";
        case ShipBuilderTool::RemoveBlock: return "RemoveBlock";
        case ShipBuilderTool::PaintBlock: return "PaintBlock";
        case ShipBuilderTool::MirrorPlane: return "MirrorPlane";
        case ShipBuilderTool::Validate: return "Validate";
        default: return "Unknown";
    }
}

std::string ShipBuilderVisualStateSummary(const ShipBuilderVisualState& state)
{
    std::ostringstream stream;
    stream << "tool=" << ShipBuilderToolName(state.activeTool) << " grid=" << state.gridSize << " palette=" << state.palette.size();
    return stream.str();
}

} // namespace subspace
