#pragma once

#include "core/math/Vector2.h"

#include <array>
#include <cstdint>

namespace subspace {
namespace Input {

enum class GamepadButton : uint32_t {
    A, B, X, Y,
    LeftBumper, RightBumper,
    LeftTrigger, RightTrigger,
    LeftStick, RightStick,
    DpadUp, DpadDown, DpadLeft, DpadRight,
    Start, Back,
    Count
};

/// Tracks per-frame gamepad/controller state for one connected device.
///
/// Axes are normalised to [-1, +1].  Triggers are in [0, +1].
class Gamepad {
public:
    /// Whether this gamepad slot is currently connected.
    bool IsConnected() const noexcept { return _connected; }

    /// Analogue axis values.
    float GetLeftStickX()  const noexcept { return _leftX;  }
    float GetLeftStickY()  const noexcept { return _leftY;  }
    float GetRightStickX() const noexcept { return _rightX; }
    float GetRightStickY() const noexcept { return _rightY; }
    float GetLeftTrigger() const noexcept { return _lt;     }
    float GetRightTrigger()const noexcept { return _rt;     }

    bool IsButtonDown    (GamepadButton btn) const noexcept;
    bool IsButtonPressed (GamepadButton btn) const noexcept;
    bool IsButtonReleased(GamepadButton btn) const noexcept;

    /// Advance to the next frame.
    /// @param connected    Whether the device is connected.
    /// @param lx, ly       Left stick (-1..+1).
    /// @param rx, ry       Right stick (-1..+1).
    /// @param lt, rt       Triggers (0..+1).
    /// @param buttons      Bitmask of held buttons.
    void Update(bool connected,
                float lx, float ly, float rx, float ry,
                float lt, float rt,
                uint32_t buttons) noexcept;

    /// Set a single button state directly (convenience for unit tests).
    void SetButton(GamepadButton btn, bool down) noexcept;

private:
    static constexpr int kButtonCount =
        static_cast<int>(GamepadButton::Count);

    bool  _connected = false;
    float _leftX = 0, _leftY = 0, _rightX = 0, _rightY = 0;
    float _lt = 0, _rt = 0;
    bool  _current [kButtonCount]{};
    bool  _previous[kButtonCount]{};
};

} // namespace Input
} // namespace subspace
