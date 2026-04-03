#include "input/Keyboard.h"

namespace subspace {
namespace Input {

bool Keyboard::IsKeyDown(KeyCode key) const noexcept {
    auto idx = static_cast<std::size_t>(key);
    return idx < _current.size() && _current[idx];
}

bool Keyboard::IsKeyPressed(KeyCode key) const noexcept {
    auto idx = static_cast<std::size_t>(key);
    if (idx >= _current.size()) return false;
    return _current[idx] && !_previous[idx];
}

bool Keyboard::IsKeyReleased(KeyCode key) const noexcept {
    auto idx = static_cast<std::size_t>(key);
    if (idx >= _current.size()) return false;
    return !_current[idx] && _previous[idx];
}

void Keyboard::Update(
    const std::array<bool, static_cast<std::size_t>(KeyCode::Count)>& newState) noexcept {
    _previous = _current;
    _current  = newState;
}

void Keyboard::SetKey(KeyCode key, bool down) noexcept {
    auto idx = static_cast<std::size_t>(key);
    if (idx < _current.size()) _current[idx] = down;
}

} // namespace Input
} // namespace subspace
