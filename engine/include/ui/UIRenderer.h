#pragma once

#include "ui/UITypes.h"

#include <string>
#include <vector>

namespace subspace {

/// Collects draw commands from the UI tree and provides them to a backend.
///
/// This renderer is **data-driven** — it doesn't issue GPU calls.  A
/// platform-specific backend (OpenGL, Vulkan, software) reads the command
/// buffer produced each frame and translates it into actual draw calls.
class UIRenderer {
public:
    /// Clear the command buffer for a new frame.
    void BeginFrame(float screenWidth, float screenHeight);

    /// Finalize the frame (no-op currently, reserved for sorting).
    void EndFrame();

    /// Retrieve the command buffer for the current frame.
    const std::vector<DrawCommand>& GetCommands() const { return _commands; }

    /// Append one or more commands from an external source (e.g. a UIPanel).
    void Submit(const std::vector<DrawCommand>& commands);

    // --- Convenience immediate-mode helpers ---

    void DrawFilledRect(const Rect& rect, const Color& color);
    void DrawOutlineRect(const Rect& rect, const Color& color, float lineWidth = 1.0f);
    void DrawText(const std::string& text, const Vec2& pos,
                  const Color& color = Color::White(), int fontSize = 14);
    void DrawLine(const Vec2& from, const Vec2& to,
                  const Color& color = Color::White(), float lineWidth = 1.0f);
    void DrawCircle(const Vec2& center, float radius,
                    const Color& color = Color::White(), float lineWidth = 1.0f);
    void DrawFilledCircle(const Vec2& center, float radius,
                          const Color& color = Color::White());

    // --- Screen metrics ---

    float GetScreenWidth()  const { return _screenWidth; }
    float GetScreenHeight() const { return _screenHeight; }

    /// Number of draw commands queued this frame.
    size_t GetCommandCount() const { return _commands.size(); }

private:
    std::vector<DrawCommand> _commands;
    float _screenWidth  = 1920.0f;
    float _screenHeight = 1080.0f;
};

} // namespace subspace
