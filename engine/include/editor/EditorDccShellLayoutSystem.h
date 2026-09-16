#pragma once

namespace subspace {

struct EditorDccRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct EditorDccShellLayout {
    bool valid = false;
    bool compact = false;
    float uiScale = 1.0f;
    EditorDccRect applicationMenu{};
    EditorDccRect workspaceStrip{};
    EditorDccRect viewportHeader{};
    EditorDccRect viewport{};
    EditorDccRect toolRail{};
    EditorDccRect assetShelf{};
    EditorDccRect outliner{};
    EditorDccRect properties{};
    EditorDccRect statusBar{};
};

/// Dimension-agnostic DCC shell geometry shared by Subspace authoring surfaces.
/// The shell follows a Blender-like arrangement without copying Blender UI:
/// compact application/workspace headers, dominant center viewport, left tool
/// rail, Outliner over Properties on the right, and a bottom asset shelf.
class EditorDccShellLayoutSystem {
public:
    static EditorDccShellLayout Compute(int width, int height);
};

} // namespace subspace
