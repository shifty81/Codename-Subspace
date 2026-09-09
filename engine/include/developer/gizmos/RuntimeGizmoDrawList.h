#pragma once

#include <array>
#include <string>
#include <vector>

namespace subspace {

struct GizmoVec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct GizmoColor {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

enum class RuntimeGizmoKind {
    Line,
    Box,
    Sphere,
    Axis,
    Label,
    SelectionBounds,
    PlacementPreview,
};

struct RuntimeGizmoCommand {
    RuntimeGizmoKind kind = RuntimeGizmoKind::Line;
    GizmoVec3 a;
    GizmoVec3 b;
    GizmoVec3 size;
    GizmoColor color;
    std::string label;
};

class RuntimeGizmoDrawList {
public:
    void Clear();
    void AddLine(GizmoVec3 start, GizmoVec3 end, GizmoColor color = {});
    void AddBox(GizmoVec3 center, GizmoVec3 size, GizmoColor color = {});
    void AddSelectionBounds(GizmoVec3 center, GizmoVec3 size, GizmoColor color = {});
    void AddLabel(GizmoVec3 position, std::string text, GizmoColor color = {});

    bool Empty() const { return _commands.empty(); }
    const std::vector<RuntimeGizmoCommand>& GetCommands() const { return _commands; }

private:
    std::vector<RuntimeGizmoCommand> _commands;
};

} // namespace subspace
