#include "input/Mouse.h"

namespace subspace {
namespace Input {

bool Mouse::IsButtonDown(MouseButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    return idx < kButtonCount && _current[idx];
}

bool Mouse::IsButtonPressed(MouseButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    if (idx >= kButtonCount) return false;
    return _current[idx] && !_previous[idx];
}

bool Mouse::IsButtonReleased(MouseButton btn) const noexcept {
    auto idx = static_cast<int>(btn);
    if (idx >= kButtonCount) return false;
    return !_current[idx] && _previous[idx];
}

void Mouse::Update(float x, float y, uint8_t buttons, float scrollDelta) noexcept {
    _prevX = _x; _prevY = _y;
    _x = x; _y = y;
    _dx = _x - _prevX;
    _dy = _y - _prevY;
    _scrollDelta = scrollDelta;
    for (int i = 0; i < kButtonCount; ++i) {
        _previous[i] = _current[i];
        _current[i]  = (buttons >> i) & 1u;
    }
}

void Mouse::SetButton(MouseButton btn, bool down) noexcept {
    auto idx = static_cast<int>(btn);
    if (idx < kButtonCount) _current[idx] = down;
}

} // namespace Input
} // namespace subspace
