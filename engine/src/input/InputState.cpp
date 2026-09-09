#include "input/InputState.h"

#include <algorithm>

namespace subspace {

void InputState::SetAction(InputAction action, bool down)
{
    auto& state = _actions[Index(action)];
    if (state.down == down) {
        state.value = down ? 1.0f : 0.0f;
        return;
    }

    state.down = down;
    state.value = down ? 1.0f : 0.0f;
    state.pressed = down;
    state.released = !down;
}

void InputState::SetActionValue(InputAction action, float value)
{
    value = std::clamp(value, 0.0f, 1.0f);
    auto& state = _actions[Index(action)];
    const bool down = value > 0.0001f;

    if (state.down != down) {
        state.pressed = down;
        state.released = !down;
    }

    state.down = down;
    state.value = value;
}

bool InputState::IsDown(InputAction action) const
{
    return _actions[Index(action)].down;
}

bool InputState::WasPressed(InputAction action) const
{
    return _actions[Index(action)].pressed;
}

bool InputState::WasReleased(InputAction action) const
{
    return _actions[Index(action)].released;
}

float InputState::GetValue(InputAction action) const
{
    return _actions[Index(action)].value;
}

void InputState::EndFrame()
{
    for (auto& state : _actions) {
        state.pressed = false;
        state.released = false;
    }
}

void InputState::Clear()
{
    for (auto& state : _actions) {
        state = InputActionState{};
    }
}

} // namespace subspace
