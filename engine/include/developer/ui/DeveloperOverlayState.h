#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class DeveloperOverlayPrimitiveType {
    Panel,
    Text,
    InputBox,
    Divider,
    StatusPill
};

struct DeveloperOverlayPrimitive {
    DeveloperOverlayPrimitiveType type = DeveloperOverlayPrimitiveType::Text;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::string text;
};

struct DeveloperOverlayDrawList {
    std::vector<DeveloperOverlayPrimitive> primitives;

    void Clear() { primitives.clear(); }
    bool Empty() const { return primitives.empty(); }
};

struct DeveloperOverlayStatus {
    bool visible = false;
    bool developerModeEnabled = false;
    bool canUndo = false;
    bool canRedo = false;
    std::size_t dirtyEditCount = 0;
    std::string lastMessage;
};

} // namespace subspace
