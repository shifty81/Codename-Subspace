#pragma once

#include "core/math/Vector2.h"

#include <cstdint>

namespace subspace {
namespace Input {

enum class MouseButton : uint8_t {
    Left   = 0,
    Right  = 1,
    Middle = 2,
    Count
};

/// Tracks per-frame mouse state.
///
/// Call Update() once per frame with raw position and button state.
class Mouse {
public:
    /// Current cursor position in window-space pixels.
    float GetX() const noexcept { return _x; }
    float GetY() const noexcept { return _y; }

    /// Cursor delta since last frame.
    float GetDeltaX() const noexcept { return _dx; }
    float GetDeltaY() const noexcept { return _dy; }

    /// Mouse wheel scroll amount this frame (positive = up).
    float GetScrollDelta() const noexcept { return _scrollDelta; }

    /// Returns true while a button is held down.
    bool IsButtonDown(MouseButton btn) const noexcept;

    /// Returns true only on the first frame the button is pressed.
    bool IsButtonPressed(MouseButton btn) const noexcept;

    /// Returns true only on the frame the button is released.
    bool IsButtonReleased(MouseButton btn) const noexcept;

    /// Advance to the next frame.
    /// @param x, y        New absolute cursor position.
    /// @param buttons     Bitmask — bit N is set when MouseButton(N) is held.
    /// @param scrollDelta Wheel movement this frame.
    void Update(float x, float y, uint8_t buttons,
                float scrollDelta = 0.0f) noexcept;

    /// Set a single button state directly (convenience for unit tests).
    void SetButton(MouseButton btn, bool down) noexcept;

private:
    static constexpr int kButtonCount =
        static_cast<int>(MouseButton::Count);

    float _x = 0.0f, _y = 0.0f;
    float _prevX = 0.0f, _prevY = 0.0f;
    float _dx = 0.0f, _dy = 0.0f;
    float _scrollDelta = 0.0f;

    bool _current [kButtonCount]{};
    bool _previous[kButtonCount]{};
};

} // namespace Input
} // namespace subspace
