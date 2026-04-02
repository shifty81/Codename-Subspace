#include "input/Gamepad.h"

namespace subspace {
namespace Input {

bool Gamepad::IsButtonDown(GamepadButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    return idx < kButtonCount && _current[idx];
}

bool Gamepad::IsButtonPressed(GamepadButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    if (idx >= kButtonCount) return false;
    return _current[idx] && !_previous[idx];
}

bool Gamepad::IsButtonReleased(GamepadButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    if (idx >= kButtonCount) return false;
    return !_current[idx] && _previous[idx];
}

void Gamepad::Update(bool connected,
                     float lx, float ly, float rx, float ry,
                     float lt, float rt,
                     uint32_t buttons) noexcept {
    _connected = connected;
    _leftX  = lx; _leftY  = ly;
    _rightX = rx; _rightY = ry;
    _lt = lt; _rt = rt;
    for (int i = 0; i < kButtonCount; ++i) {
        _previous[i] = _current[i];
        _current[i]  = (buttons >> i) & 1u;
    }
}

void Gamepad::SetButton(GamepadButton btn, bool down) noexcept {
    auto idx = static_cast<int>(btn);
    if (idx < kButtonCount) _current[idx] = down;
}

} // namespace Input
} // namespace subspace
