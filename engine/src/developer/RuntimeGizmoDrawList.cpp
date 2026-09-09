#include "developer/gizmos/RuntimeGizmoDrawList.h"

namespace subspace {

void RuntimeGizmoDrawList::Clear()
{
    _commands.clear();
}

void RuntimeGizmoDrawList::AddLine(GizmoVec3 start, GizmoVec3 end, GizmoColor color)
{
    _commands.push_back({RuntimeGizmoKind::Line, start, end, {}, color, {}});
}

void RuntimeGizmoDrawList::AddBox(GizmoVec3 center, GizmoVec3 size, GizmoColor color)
{
    _commands.push_back({RuntimeGizmoKind::Box, center, {}, size, color, {}});
}

void RuntimeGizmoDrawList::AddSelectionBounds(GizmoVec3 center, GizmoVec3 size, GizmoColor color)
{
    _commands.push_back({RuntimeGizmoKind::SelectionBounds, center, {}, size, color, {}});
}

void RuntimeGizmoDrawList::AddLabel(GizmoVec3 position, std::string text, GizmoColor color)
{
    _commands.push_back({RuntimeGizmoKind::Label, position, {}, {}, color, std::move(text)});
}

} // namespace subspace
